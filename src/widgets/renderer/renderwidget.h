#pragma once

#include "../../services/renderer/renderservice.h"

#include <DiligentCore/Common/interface/RefCntAutoPtr.hpp>
#include <DiligentCore/Graphics/GraphicsEngine/interface/DeviceContext.h>
#include <DiligentCore/Graphics/GraphicsEngine/interface/Fence.h>
#include <DiligentCore/Graphics/GraphicsEngine/interface/RenderDevice.h>
#include <DiligentCore/Graphics/GraphicsEngine/interface/SwapChain.h>
#include <DiligentCore/Graphics/GraphicsEngine/interface/Texture.h>

#include <QWidget>

class RenderWidget : public QWidget {
    Q_OBJECT
public:
    explicit RenderWidget(QWidget* parent = nullptr);
    ~RenderWidget() override;

    QPaintEngine* paintEngine() const override;

protected:
    void showEvent(QShowEvent* event) override;
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

    // New method to handle rendering
    void render();

    virtual void do_render() = 0;
    // Initialize rendering resources
    void initialize();

protected:
    RenderContext ctx_;
    Diligent::ITextureView* pRTV_ = nullptr;
    Diligent::RefCntAutoPtr<Diligent::ITexture> depth_texture_;
    Diligent::ITextureView* pDSV_ = nullptr;

    // Synchronization objects
    Diligent::RefCntAutoPtr<Diligent::IFence> fence_;
    Diligent::Uint64 fenceValue_ = 0;
    uint64_t frameCounter_ = 0;
    float height_ = 0.0f;
    float width_ = 0.0f;
};
