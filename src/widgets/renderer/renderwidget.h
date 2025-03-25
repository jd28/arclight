#pragma once

#include <DiligentCore/Common/interface/RefCntAutoPtr.hpp>
#include <DiligentCore/Graphics/GraphicsEngine/interface/DeviceContext.h>
#include <DiligentCore/Graphics/GraphicsEngine/interface/Fence.h>
#include <DiligentCore/Graphics/GraphicsEngine/interface/RenderDevice.h>
#include <DiligentCore/Graphics/GraphicsEngine/interface/SwapChain.h>
#include <DiligentCore/Graphics/GraphicsEngine/interface/Texture.h>

#include <QWidget>

class QImage;

class RenderWidget : public QWidget {
    Q_OBJECT

public:
    explicit RenderWidget(QWidget* parent = nullptr);
    ~RenderWidget() override;

    void initialize();
    void render();
    void cleanup();

protected:
    void showEvent(QShowEvent* event) override;
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

    // Override this method in subclasses to implement custom rendering
    virtual void do_render() { }

private:
    void renderToFBO();
    void transferFBOToQImage();

    // FBO-related members
    Diligent::RefCntAutoPtr<Diligent::ITexture> fboTexture_;
    Diligent::RefCntAutoPtr<Diligent::ITextureView> fboRTV_;
    Diligent::RefCntAutoPtr<Diligent::ITexture> depthTexture_;
    Diligent::RefCntAutoPtr<Diligent::ITextureView> pDSV_;
    Diligent::RefCntAutoPtr<Diligent::ITexture> stagingTexture_;

    // Qt-related members
    QImage* outputImage_;

    int frameCounter_;
    bool initialized_;
};
