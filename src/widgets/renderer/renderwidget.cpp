#include "renderwidget.h"

#include "../../services/renderer/renderservice.h"

#include <QImage>
#include <QPainter>
#include <QResizeEvent>
#include <QTimer>

RenderWidget::RenderWidget(QWidget* parent)
    : QWidget(parent)
    , fboTexture_(nullptr)
    , fboRTV_(nullptr)
    , depthTexture_(nullptr)
    , pDSV_(nullptr)
    , outputImage_(nullptr)
    , frameCounter_(0)
    , initialized_(false)
{

    setAttribute(Qt::WA_OpaquePaintEvent);
    setFocusPolicy(Qt::StrongFocus);
}

RenderWidget::~RenderWidget()
{
    cleanup();
}

void RenderWidget::initialize()
{
    LOG_F(INFO, "****************** Initialize ******************");
    cleanup(); // Make sure previous resources are properly released

    const int width = this->width() * devicePixelRatio();
    const int height = this->height() * devicePixelRatio();

    if (width <= 0 || height <= 0) {
        return;
    }

    auto* device = renderer().device();
    CHECK_F(!!device, "device is NULL - descriptor view creation failed!");

    Diligent::TextureDesc ColorDesc;
    ColorDesc.Name = "FBO Color Buffer";
    ColorDesc.Type = Diligent::RESOURCE_DIM_TEX_2D;
    ColorDesc.Width = width;
    ColorDesc.Height = height;
    ColorDesc.Format = Diligent::TEX_FORMAT_RGBA8_UNORM_SRGB; // Match your pipeline format
    ColorDesc.BindFlags = Diligent::BIND_RENDER_TARGET | Diligent::BIND_SHADER_RESOURCE;
    ColorDesc.Usage = Diligent::USAGE_DEFAULT;
    ColorDesc.CPUAccessFlags = Diligent::CPU_ACCESS_NONE;

    device->CreateTexture(ColorDesc, nullptr, &fboTexture_);
    CHECK_F(!!fboTexture_, "fboTexture_ is NULL - descriptor view creation failed!");
    fboRTV_ = fboTexture_->GetDefaultView(Diligent::TEXTURE_VIEW_RENDER_TARGET);
    CHECK_F(!!fboRTV_, "fboRTV_ is NULL - descriptor view creation failed!");

    Diligent::TextureDesc DepthDesc;
    DepthDesc.Name = "FBO Depth Buffer";
    DepthDesc.Type = Diligent::RESOURCE_DIM_TEX_2D;
    DepthDesc.Width = width;
    DepthDesc.Height = height;
    DepthDesc.Format = Diligent::TEX_FORMAT_D32_FLOAT;
    DepthDesc.BindFlags = Diligent::BIND_DEPTH_STENCIL;
    DepthDesc.Usage = Diligent::USAGE_DEFAULT;
    DepthDesc.CPUAccessFlags = Diligent::CPU_ACCESS_NONE;

    device->CreateTexture(DepthDesc, nullptr, &depthTexture_);
    CHECK_F(!!depthTexture_, "depthTexture_ is NULL - descriptor view creation failed!");
    pDSV_ = depthTexture_->GetDefaultView(Diligent::TEXTURE_VIEW_DEPTH_STENCIL);
    CHECK_F(!!pDSV_, "pDSV_ is NULL - descriptor view creation failed!");

    Diligent::TextureDesc StagingDesc;
    StagingDesc.Name = "Staging Texture";
    StagingDesc.Type = Diligent::RESOURCE_DIM_TEX_2D;
    StagingDesc.Width = width;
    StagingDesc.Height = height;
    StagingDesc.Format = Diligent::TEX_FORMAT_RGBA8_UNORM_SRGB; // Non-SRGB for CPU reading
    StagingDesc.Usage = Diligent::USAGE_STAGING;
    StagingDesc.CPUAccessFlags = Diligent::CPU_ACCESS_READ;
    StagingDesc.BindFlags = Diligent::BIND_NONE;

    device->CreateTexture(StagingDesc, nullptr, &stagingTexture_);
    CHECK_F(!!stagingTexture_, "stagingTexture_ is NULL - descriptor view creation failed!");

    // Create QImage for displaying the rendered content
    delete outputImage_;
    outputImage_ = new QImage(width, height, QImage::Format_RGBA8888);

    initialized_ = true;
}

void RenderWidget::transferFBOToQImage()
{
    if (!initialized_ || !fboTexture_ || !stagingTexture_ || !outputImage_)
        return;

    auto* ic = renderer().immediate_context();
    if (!ic) { return; }

    Diligent::CopyTextureAttribs CopyAttribs;
    CopyAttribs.pSrcTexture = fboTexture_;
    CopyAttribs.pDstTexture = stagingTexture_;
    CopyAttribs.SrcTextureTransitionMode = Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION;
    CopyAttribs.DstTextureTransitionMode = Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION;

    ic->CopyTexture(CopyAttribs);

    Diligent::MappedTextureSubresource MappedData;
    ic->MapTextureSubresource(
        stagingTexture_,
        0,
        0,
        Diligent::MAP_READ,
        Diligent::MAP_FLAG_DO_NOT_WAIT,
        nullptr,
        MappedData);

    const int width = outputImage_->width();
    const int height = outputImage_->height();

    for (int y = 0; y < height; ++y) {
        memcpy(
            outputImage_->scanLine(y),
            static_cast<const uint8_t*>(MappedData.pData) + y * MappedData.Stride,
            width * 4);
    }

    ic->UnmapTextureSubresource(stagingTexture_, 0, 0);
}

void RenderWidget::cleanup()
{
    if (renderer().immediate_context()) {
        renderer().immediate_context()->Flush();
        renderer().immediate_context()->WaitForIdle();
    }

    fboRTV_.Release();
    fboTexture_.Release();
    pDSV_.Release();
    depthTexture_.Release();
    stagingTexture_.Release();

    delete outputImage_;
    outputImage_ = nullptr;
    initialized_ = false;
}

void RenderWidget::showEvent(QShowEvent* event)
{
    if (!initialized_) {
        initialize();
    }
    QWidget::showEvent(event);
}

void RenderWidget::renderToFBO()
{
    if (!initialized_ || !fboRTV_ || !pDSV_ || !renderer().immediate_context())
        return;

    auto* immediateContext = renderer().immediate_context();

    Diligent::ITextureView* pRTVs[] = {fboRTV_};
    immediateContext->SetRenderTargets(1, pRTVs, pDSV_, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

    const float ClearColor[] = {0.2f, 0.3f, 0.3f, 1.0f};
    immediateContext->ClearRenderTarget(fboRTV_, ClearColor, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
    immediateContext->ClearDepthStencil(pDSV_, Diligent::CLEAR_DEPTH_FLAG, 1.0f, 0, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

    do_render();

    immediateContext->SetRenderTargets(0, nullptr, nullptr, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
}

void RenderWidget::render()
{
    if (!isVisible() || !initialized_) {
        return;
    }

    renderer().pre_frame();
    renderToFBO();
    transferFBOToQImage();
    renderer().immediate_context()->Flush();
    renderer().immediate_context()->WaitForIdle();
    update();
    frameCounter_++;
}

void RenderWidget::paintEvent(QPaintEvent* event)
{
    if (!initialized_ || !outputImage_) {
        QPainter painter(this);
        painter.fillRect(rect(), QColor(50, 75, 75));
        return;
    }

    QPainter painter(this);
    painter.drawImage(rect(), *outputImage_);
    QWidget::paintEvent(event);
}

void RenderWidget::resizeEvent(QResizeEvent* event)
{
    if (initialized_) {
        initialize();
    }
    QWidget::resizeEvent(event);
}
