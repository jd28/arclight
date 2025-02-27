#pragma once

#include "renderpipelinestate.h"

#include <DiligentCore/Common/interface/BasicMath.hpp>
#include <DiligentCore/Common/interface/RefCntAutoPtr.hpp>
#include <DiligentCore/Graphics/GraphicsEngine/interface/Buffer.h>
#include <DiligentCore/Graphics/GraphicsEngine/interface/PipelineState.h>
#include <DiligentCore/Graphics/GraphicsEngine/interface/Texture.h>

#include <nw/formats/Plt.hpp>
#include <nw/model/Mdl.hpp>
#include <nw/objects/Appearance.hpp>

#include <vector>

struct Model;
struct RenderContext;

namespace nw {
struct Area;
} // namespace nw

struct Node {
    virtual ~Node() = default;

    virtual void draw(RenderContext& ctx, const glm::mat4& mtx);
    glm::mat4 get_transform() const;
    virtual void reset() { }

    Model* owner_ = nullptr;
    nw::model::Node* orig_ = nullptr;
    Node* parent_ = nullptr;
    glm::mat4 inverse_{1.0f};
    glm::vec3 position_{0.0f};
    glm::quat rotation_{};
    glm::vec3 scale_ = glm::vec3(1.0);
    std::vector<Node*> children_;
    bool has_transform_ = false;
    bool no_render_ = false;

    RenderPipelineState rps_;
};

struct MeshConstants {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 projection;
};

struct Mesh : public Node {
    virtual void reset() override { }
    virtual void draw(RenderContext& ctx, const glm::mat4& mtx) override;

    Diligent::RefCntAutoPtr<Diligent::IBuffer> vertices;
    Diligent::RefCntAutoPtr<Diligent::IBuffer> indices;
    Diligent::RefCntAutoPtr<Diligent::IBuffer> constant_buffer;
    Diligent::RefCntAutoPtr<Diligent::IBuffer> instance_buffer;

    nw::PltColors plt_colors_{};
    Diligent::RefCntAutoPtr<Diligent::ITexture> texture0;
    bool texture0_is_plt = false;
};

struct SkinConstants {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 projection;
};

struct JointConstants {
    std::array<glm::mat4, 64> data;
};

struct Skin : public Node {
    virtual void reset() override { }
    // Submits mesh data to the GPU
    virtual void draw(RenderContext& ctx, const glm::mat4& mtx) override;

    Diligent::RefCntAutoPtr<Diligent::IBuffer> vertices;
    Diligent::RefCntAutoPtr<Diligent::IBuffer> indices;
    Diligent::RefCntAutoPtr<Diligent::IBuffer> constant_buffer;
    Diligent::RefCntAutoPtr<Diligent::IBuffer> instance_buffer;
    Diligent::RefCntAutoPtr<Diligent::IBuffer> joint_constant_buffer;

    void build_inverse_binds();
    nw::PltColors plt_colors_{};
    std::vector<glm::mat4> inverse_bind_pose_;
    SkinConstants uniforms;
    JointConstants joints;

    Diligent::RefCntAutoPtr<Diligent::ITexture> texture0;
    bool texture0_is_plt = false;
};

struct Model : public Node {
    nw::model::Model* mdl_ = nullptr;
    nw::model::Animation* anim_ = nullptr;
    int32_t anim_cursor_ = 0;
    std::vector<std::unique_ptr<Node>> nodes_;

    /// Finds a node by name
    Node* find(std::string_view name);

    /// Initialize skin meshes & joints
    void initialize_skins();

    /// Loads model from a NWN model
    bool load(nw::model::Model* mdl);

    /// Loads an animation
    bool load_animation(std::string_view anim);

    /// Updates the animimation by ``dt`` milliseconds.
    void update(int32_t dt);

    virtual void draw(RenderContext& ctx, const glm::mat4& mtx) override;

private:
    // Internal node loading
    Node* load_node(nw::model::Node* node, Node* parent = nullptr);
};

std::unique_ptr<Model> load_model(std::string_view resref);

// == BasicTileArea ===========================================================
// ============================================================================

class BasicTileArea : public Node {
public:
    BasicTileArea(nw::Area* area);

    virtual void draw(RenderContext& ctx, const glm::mat4& mtx) override;
    void load_tile_models();

    /// Updates the animimation by ``dt`` milliseconds.
    void update(int32_t dt);

    nw::Area* area_ = nullptr;
    std::vector<std::unique_ptr<Model>> tile_models_;
};
