#include "renderwidget.h"

#include <QResizeEvent>
#include <QTimer>

RenderWidget::RenderWidget(QWidget* parent)
    : QWidget(parent)
    , ctx_{}
    , frameCounter_(0)
{
    setAttribute(Qt::WA_PaintOnScreen);
    setAttribute(Qt::WA_NoSystemBackground);
    setFocusPolicy(Qt::StrongFocus);
}

RenderWidget::~RenderWidget()
{
    if (ctx_.swapchain) {
        // Make sure all pending commands are complete before destruction
        if (renderer().immediate_context()) {
            renderer().immediate_context()->WaitForIdle();
        }
        renderer().release(ctx_);
    }
}

void RenderWidget::initialize()
{
    if (!ctx_.swapchain) {
        LOG_F(ERROR, "Swapchain not initialized in initialize()");
        return;
    }

    pRTV_ = ctx_.swapchain->GetCurrentBackBufferRTV();
    auto& swapChainDesc = ctx_.swapchain->GetDesc();

    Diligent::TextureDesc DepthDesc;
    DepthDesc.Type = Diligent::RESOURCE_DIM_TEX_2D;
    DepthDesc.Width = swapChainDesc.Width;
    DepthDesc.Height = swapChainDesc.Height;
    DepthDesc.Format = Diligent::TEX_FORMAT_D32_FLOAT;
    DepthDesc.BindFlags = Diligent::BIND_DEPTH_STENCIL;
    DepthDesc.Usage = Diligent::USAGE_DEFAULT;
    DepthDesc.CPUAccessFlags = Diligent::CPU_ACCESS_NONE;

    depth_texture_.Release();
    renderer().device()->CreateTexture(DepthDesc, nullptr, &depth_texture_);
    pDSV_ = depth_texture_->GetDefaultView(Diligent::TEXTURE_VIEW_DEPTH_STENCIL);
}

QPaintEngine* RenderWidget::paintEngine() const
{
    return nullptr;
}

void RenderWidget::showEvent(QShowEvent* event)
{
    if (!ctx_.swapchain) {
        ctx_ = renderer().create(reinterpret_cast<void*>(winId()));
        if (!ctx_.swapchain) {
            LOG_F(ERROR, "Failed to create swapchain");
            return;
        }
        initialize();
    }
    QWidget::showEvent(event);
}

void RenderWidget::paintEvent(QPaintEvent* event)
{
}

void RenderWidget::render()
{
    if (!isVisible()) { return; }
    if (!ctx_.swapchain || !renderer().immediate_context())
        return;

    pRTV_ = ctx_.swapchain->GetCurrentBackBufferRTV();

    Diligent::ITextureView* pRTVs[] = {pRTV_};
    renderer().immediate_context()->SetRenderTargets(1, pRTVs, pDSV_, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

    const float ClearColor[] = {0.2f, 0.3f, 0.3f, 1.0f};
    renderer().immediate_context()->ClearRenderTarget(pRTV_, ClearColor, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
    renderer().immediate_context()->ClearDepthStencil(pDSV_, Diligent::CLEAR_DEPTH_FLAG, 1.0f, 0, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

    do_render();

    ctx_.swapchain->Present(1);

    update();
    frameCounter_++;
}

void RenderWidget::resizeEvent(QResizeEvent* event)
{
    if (ctx_.swapchain) {
        if (renderer().immediate_context()) {
            renderer().immediate_context()->WaitForIdle();
        }

        const QSize size = event->size();
        int width = size.width() * devicePixelRatio();
        int height = size.height() * devicePixelRatio();

        ctx_.swapchain->Resize(width, height);
        initialize(); // Reinitialize resources after resize
    }
    QWidget::resizeEvent(event);
}
