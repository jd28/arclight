#include "model.hpp"

#include "../../services/renderer/renderservice.h"

#include <nw/formats/Tileset.hpp>
#include <nw/kernel/ModelCache.hpp>
#include <nw/kernel/Strings.hpp>
#include <nw/objects/Area.hpp>

#include <DiligentCore/Graphics/GraphicsTools/interface/MapHelper.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/string_cast.hpp>

void Node::draw(RenderContext& ctx, const glm::mat4x4& mtx)
{
    glm::mat4x4 trans;
    if (has_transform_) {
        trans = glm::translate(mtx, position_);
        trans = trans * glm::toMat4(rotation_);
        trans = glm::scale(trans, scale_);
    } else {
        trans = mtx;
    }

    for (auto child : children_) {
        child->draw(ctx, trans);
    }
}

// == Mesh ====================================================================s
// ============================================================================

glm::mat4 Node::get_transform() const
{
    auto parent = glm::mat4{1.0f};
    if (!has_transform_) { return parent; }
    if (parent_) {
        parent = parent_->get_transform();
    }

    auto trans = glm::translate(parent, position_);
    trans = trans * glm::toMat4(rotation_);
    trans = glm::scale(trans, scale_);

    return trans;
}

void Mesh::draw(RenderContext& ctx, const glm::mat4x4& mtx)
{
    auto trans = glm::translate(mtx, position_);
    trans = trans * glm::toMat4(rotation_);
    trans = glm::scale(trans, scale_);

    if (!no_render_) {
        // LOG_F(INFO, "view matrix: {}", glm::to_string(ctx.view));
        // LOG_F(INFO, "projection matrix: {}", glm::to_string(ctx.projection));
        // LOG_F(INFO, "model transform matrix: {}", glm::to_string(trans));
        if (!vertices || !indices) {
            LOG_F(ERROR, "Invalid vertex or index buffers for mesh");
            return;
        }

        auto [pso, srb] = renderer().get_pso(rps_);
        if (!pso || !srb) {
            LOG_F(ERROR, "Invalid PSO for mesh");
            return;
        }

        {
            Diligent::MapHelper<MeshConstants> constants(renderer().immediate_context(), constant_buffer, Diligent::MAP_WRITE, Diligent::MAP_FLAG_DISCARD);
            if (!constants) {
                LOG_F(ERROR, "Failed to map constant buffer for mesh");
                return;
            }
            constants->model = trans;
            constants->view = ctx.view;
            constants->projection = ctx.projection;
        }

        if (constant_buffer) {
            srb->GetVariableByName(Diligent::SHADER_TYPE_VERTEX, "Constants")->Set(constant_buffer);
        } else {
            LOG_F(ERROR, "Constant buffer is null");
        }

        if (texture0) {
            srb->GetVariableByName(Diligent::SHADER_TYPE_PIXEL, "g_Texture")->Set(texture0_view);
        } else {
            LOG_F(WARNING, "No texture for mesh");
        }

        renderer().immediate_context()->SetPipelineState(pso);
        renderer().immediate_context()->CommitShaderResources(srb, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

        Diligent::Uint64 offsets[] = {0};
        Diligent::IBuffer* vertex_buffers[] = {vertices};
        renderer().immediate_context()->SetVertexBuffers(0, 1, vertex_buffers, offsets, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION, Diligent::SET_VERTEX_BUFFERS_FLAG_RESET);
        renderer().immediate_context()->SetIndexBuffer(indices, 0, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

        auto orig = static_cast<nw::model::TrimeshNode*>(orig_);
        Diligent::DrawIndexedAttribs draw_attrs;
        draw_attrs.IndexType = Diligent::VT_UINT16;
        draw_attrs.NumIndices = static_cast<uint32_t>(orig->indices.size());
        draw_attrs.Flags = Diligent::DRAW_FLAG_VERIFY_ALL;
        renderer().immediate_context()->DrawIndexed(draw_attrs);
    }

    for (auto child : children_) {
        child->draw(ctx, trans);
    }
}

// == Skin ====================================================================
// ============================================================================

inline void build_inverse_bind_array(Skin* parent, Node* node, glm::mat4 parent_transform, std::vector<glm::mat4>& binds)
{
    if (!node) { return; }
    auto trans = glm::translate(parent_transform, node->position_);
    trans = trans * glm::toMat4(node->rotation_);
    binds.push_back(glm::inverse(trans));

    for (auto n : node->children_) {
        build_inverse_bind_array(parent, n, trans, binds);
    }
}

void Skin::build_inverse_binds()
{
    glm::mat4 ptrans{1.0f};
    Node* parent = this;
    while (parent->parent_) {
        parent = parent->parent_;
        if (parent->has_transform_) {
            ptrans = glm::translate(ptrans, parent->position_) * glm::toMat4(parent->rotation_);
        }
    }

    auto trans = glm::translate(ptrans, position_) * glm::toMat4(rotation_);
    build_inverse_bind_array(this, parent, glm::inverse(trans), inverse_bind_pose_);
}

void Skin::draw(RenderContext& ctx, const glm::mat4x4& mtx)
{
    auto orig = static_cast<nw::model::SkinNode*>(orig_);

    for (size_t i = 0; i < 64; ++i) {
        if (orig->bone_nodes[i] <= 0 || size_t(orig->bone_nodes[i]) >= owner_->nodes_.size()) {
            break;
        }

        auto bone_node = owner_->nodes_[orig->bone_nodes[i]].get();

        joints.data[i] = bone_node->get_transform() * inverse_bind_pose_[orig->bone_nodes[i]];
    }

    uniforms.projection = ctx.projection;
    uniforms.view = ctx.view;
    uniforms.model = mtx; // [NOTE] Model transform is already included!
    {
        Diligent::MapHelper<SkinConstants> constants(renderer().immediate_context(), constant_buffer, Diligent::MAP_WRITE, Diligent::MAP_FLAG_DISCARD);
        if (!constants) {
            LOG_F(ERROR, "Failed to map constant buffer for mesh");
            return;
        }
        *constants = uniforms;
    }

    {
        Diligent::MapHelper<JointConstants> constants(renderer().immediate_context(), joint_constant_buffer, Diligent::MAP_WRITE, Diligent::MAP_FLAG_DISCARD);
        if (!constants) {
            LOG_F(ERROR, "Failed to map constant buffer for mesh");
            return;
        }
        *constants = joints;
    }

    auto [pso, srb] = renderer().get_pso(rps_);

    if (constant_buffer) {
        srb->GetVariableByName(Diligent::SHADER_TYPE_VERTEX, "Constants")->Set(constant_buffer);
    } else {
        LOG_F(ERROR, "Constant buffer is null");
    }

    if (joint_constant_buffer) {
        srb->GetVariableByName(Diligent::SHADER_TYPE_VERTEX, "Joints")->Set(joint_constant_buffer);
    } else {
        LOG_F(ERROR, "Joint constant buffer is null");
    }

    if (texture0) {
        srb->GetVariableByName(Diligent::SHADER_TYPE_PIXEL, "g_Texture")->Set(texture0_view);
    } else {
        LOG_F(WARNING, "No texture for mesh");
    }

    renderer().immediate_context()->SetPipelineState(pso);
    renderer().immediate_context()->CommitShaderResources(srb, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

    Diligent::Uint64 offsets[] = {0};
    Diligent::IBuffer* vertex_buffers[] = {vertices};
    renderer().immediate_context()->SetVertexBuffers(0, 1, vertex_buffers, offsets, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION, Diligent::SET_VERTEX_BUFFERS_FLAG_RESET);
    renderer().immediate_context()->SetIndexBuffer(indices, 0, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

    Diligent::DrawIndexedAttribs draw_attrs;
    draw_attrs.IndexType = Diligent::VT_UINT16;
    draw_attrs.NumIndices = static_cast<uint32_t>(orig->indices.size());
    draw_attrs.Flags = Diligent::DRAW_FLAG_VERIFY_ALL;
    renderer().immediate_context()->DrawIndexed(draw_attrs);

    // Draw children
    for (auto child : children_) {
        child->draw(ctx, mtx);
    }
}

// == Model ===================================================================
// ============================================================================

void Model::draw(RenderContext& ctx, const glm::mat4x4& mtx)
{
    nodes_[0]->draw(ctx, mtx);
}

Node* Model::find(std::string_view name)
{
    for (const auto& node : nodes_) {
        if (nw::string::icmp(node->orig_->name, name)) {
            return node.get();
        }
    }

    return nullptr;
}

void Model::initialize_skins()
{
    for (auto& node : nodes_) {
        if (node->orig_->type == nw::model::NodeType::skin) {
            auto n = static_cast<Skin*>(node.get());
            n->build_inverse_binds();
        }
    }
}

bool Model::load(nw::model::Model* mdl)
{
    auto root = mdl->find(std::regex(mdl->name));
    if (!root) {
        LOG_F(INFO, "No root dummy");
        return false;
    }
    mdl_ = mdl;
    if (load_node(root)) {
        for (auto& node : nodes_) {
            node->owner_ = this;
        }
        initialize_skins();

        return true;
    }
    return false;
}

bool Model::load_animation(std::string_view anim)
{
    anim_ = nullptr;
    nw::model::Model* m = mdl_;
    while (m) {
        for (const auto& it : m->animations) {
            if (it->name == anim) {
                anim_ = it.get();
                break;
            }
        }
        if (!m->supermodel || anim_) { break; }
        m = &m->supermodel->model;
    }
    if (anim_) {
        LOG_F(INFO, "Loaded animation: {} from model: {}", anim, m->name);
    }
    return !!anim_;
}

Node* Model::load_node(nw::model::Node* node, Node* parent)
{
    Node* result = nullptr;
    if (node->type & nw::model::NodeFlags::skin) {
        auto n = static_cast<nw::model::SkinNode*>(node);
        if (!n->indices.empty()) {
            Skin* skin = new Skin;
            skin->rps_.has_skin = true;
            skin->rps_.has_diffuse = true;
            auto [pso, srb] = renderer().get_pso(rps_);

            // Create vertex buffer
            Diligent::BufferDesc VBDesc;
            VBDesc.Name = "Skin Vertex Buffer";
            VBDesc.Usage = Diligent::USAGE_IMMUTABLE;
            VBDesc.BindFlags = Diligent::BIND_VERTEX_BUFFER;
            VBDesc.Size = n->vertices.size() * sizeof(nw::model::SkinVertex);
            Diligent::BufferData VBData;
            VBData.pData = n->vertices.data();
            VBData.DataSize = VBDesc.Size;
            renderer().device()->CreateBuffer(VBDesc, &VBData, &skin->vertices);

            // Create index buffer
            Diligent::BufferDesc IBDesc;
            IBDesc.Name = "Skin Index Buffer";
            IBDesc.Usage = Diligent::USAGE_IMMUTABLE;
            IBDesc.BindFlags = Diligent::BIND_INDEX_BUFFER;
            IBDesc.Size = n->indices.size() * sizeof(uint16_t);
            Diligent::BufferData IBData;
            IBData.pData = n->indices.data();
            IBData.DataSize = IBDesc.Size;
            renderer().device()->CreateBuffer(IBDesc, &IBData, &skin->indices);

            auto tex = renderer().textures().load(n->bitmap);
            if (tex) {
                skin->texture0 = tex->first;
                skin->texture0_is_plt = tex->second;
                skin->texture0_view = skin->texture0->GetDefaultView(Diligent::TEXTURE_VIEW_SHADER_RESOURCE);

                if (skin->texture0_is_plt) {
                }
            } else {
                LOG_F(FATAL, "Failed to bind texture");
            }

            // Constant Buffer Creation
            if (skin->constant_buffer == nullptr) {
                Diligent::BufferDesc constantBufferDesc;
                constantBufferDesc.Name = "Skin Constant Buffer";
                constantBufferDesc.Usage = Diligent::USAGE_DYNAMIC;
                constantBufferDesc.BindFlags = Diligent::BIND_UNIFORM_BUFFER;
                constantBufferDesc.CPUAccessFlags = Diligent::CPU_ACCESS_WRITE;
                constantBufferDesc.Size = sizeof(SkinConstants);

                renderer().device()->CreateBuffer(constantBufferDesc, nullptr, &skin->constant_buffer);
            }

            if (skin->joint_constant_buffer == nullptr) {
                Diligent::BufferDesc constantBufferDesc;
                constantBufferDesc.Name = "Skin Joint Constant Buffer";
                constantBufferDesc.Usage = Diligent::USAGE_DYNAMIC;
                constantBufferDesc.BindFlags = Diligent::BIND_UNIFORM_BUFFER;
                constantBufferDesc.CPUAccessFlags = Diligent::CPU_ACCESS_WRITE;
                constantBufferDesc.Size = sizeof(JointConstants);

                renderer().device()->CreateBuffer(constantBufferDesc, nullptr, &skin->joint_constant_buffer);
            }

            result = skin;
        } else {
            LOG_F(ERROR, "No vertex indicies");
        }
    } else if (node->type & nw::model::NodeFlags::mesh && !(node->type & nw::model::NodeFlags::aabb)) {
        auto n = static_cast<nw::model::TrimeshNode*>(node);
        if (!n->vertices.empty() && !n->indices.empty()) {
            auto mesh = new Mesh();
            mesh->orig_ = node;
            mesh->no_render_ = !n->render;
            mesh->rps_.has_diffuse = true;

            // Vertex Buffer Creation
            Diligent::BufferDesc vertexBufferDesc;
            vertexBufferDesc.Name = "Vertex Buffer";
            vertexBufferDesc.Usage = Diligent::USAGE_IMMUTABLE;
            vertexBufferDesc.BindFlags = Diligent::BIND_VERTEX_BUFFER;
            vertexBufferDesc.Size = n->vertices.size() * sizeof(nw::model::Vertex);

            Diligent::BufferData vertexBufferData;
            vertexBufferData.pData = n->vertices.data();
            vertexBufferData.DataSize = vertexBufferDesc.Size;

            renderer().device()->CreateBuffer(vertexBufferDesc, &vertexBufferData, &mesh->vertices);

            // Index Buffer Creation
            Diligent::BufferDesc indexBufferDesc;
            indexBufferDesc.Name = "Index Buffer";
            indexBufferDesc.Usage = Diligent::USAGE_IMMUTABLE;
            indexBufferDesc.BindFlags = Diligent::BIND_INDEX_BUFFER;
            indexBufferDesc.Size = n->indices.size() * sizeof(uint16_t);

            Diligent::BufferData indexBufferData;
            indexBufferData.pData = n->indices.data();
            indexBufferData.DataSize = indexBufferDesc.Size;

            renderer().device()->CreateBuffer(indexBufferDesc, &indexBufferData, &mesh->indices);

            // Texture Creation
            auto tex = renderer().textures().load(n->bitmap);
            if (tex) {
                mesh->texture0 = tex->first;
                mesh->texture0_is_plt = tex->second;
                mesh->texture0_view = mesh->texture0->GetDefaultView(Diligent::TEXTURE_VIEW_SHADER_RESOURCE);

                if (mesh->texture0_is_plt) {
                }
            } else {
                LOG_F(FATAL, "Failed to bind texture");
            }

            // Constant Buffer Creation
            if (mesh->constant_buffer == nullptr) {
                Diligent::BufferDesc constantBufferDesc;
                constantBufferDesc.Name = "Mesh Constant Buffer";
                constantBufferDesc.Usage = Diligent::USAGE_DYNAMIC;
                constantBufferDesc.BindFlags = Diligent::BIND_UNIFORM_BUFFER;
                constantBufferDesc.CPUAccessFlags = Diligent::CPU_ACCESS_WRITE;
                constantBufferDesc.Size = sizeof(MeshConstants);

                renderer().device()->CreateBuffer(constantBufferDesc, nullptr, &mesh->constant_buffer);
            }
            result = mesh;
        } else {
            LOG_F(ERROR, "No vertex indicies");
        }
    }

    if (!result) {
        result = new Node;
    }

    result->parent_ = parent;
    result->orig_ = node;

    auto key = node->get_controller(nw::model::ControllerType::Position);
    if (key.data.size()) {
        result->has_transform_ = true;
        if (key.data.size() != 3) {
            LOG_F(FATAL, "Wrong size position: {}", key.data.size());
        }
        result->position_ = glm::vec3{key.data[0], key.data[1], key.data[2]};

        key = node->get_controller(nw::model::ControllerType::Orientation);
        if (key.data.size() != 4) {
            LOG_F(FATAL, "Wrong size orientation: {}", key.data.size());
        }
        result->rotation_ = glm::qua{key.data[3], key.data[0], key.data[1], key.data[2]};
    }

    nodes_.emplace_back(result);
    for (auto child : node->children) {
        result->children_.push_back(load_node(child, result));
    }

    return result;
}

void Model::update(int32_t dt)
{
    if (!anim_) { return; }

    // Update animation cursor with wrapping
    if (dt + anim_cursor_ > int32_t(anim_->length * 1000)) {
        anim_cursor_ = dt + anim_cursor_ - int32_t(anim_->length * 1000);
    } else {
        anim_cursor_ += dt;
    }

    float time_ms = static_cast<float>(anim_cursor_);

    for (const auto& anim : anim_->nodes) {
        auto node = find(anim->name);
        if (!node) { continue; }

        auto poskey = anim->get_controller(nw::model::ControllerType::Position, true);
        if (poskey.time.size() > 0) {

            int idx1 = -1;
            int idx2 = 0;
            float t = 0.0f;

            for (size_t i = 0; i < poskey.time.size(); ++i) {
                if (time_ms >= poskey.time[i] * 1000) {
                    idx1 = static_cast<int>(i);
                } else {
                    idx2 = static_cast<int>(i);
                    break;
                }
            }

            if (idx1 == -1) {
                idx1 = static_cast<int>(poskey.time.size() - 1);
                idx2 = 0;
            } else if (idx2 == 0 && idx1 != -1) {
                idx2 = 0;
            }

            float time1 = poskey.time[idx1] * 1000;
            float time2 = (size_t(idx2) < poskey.time.size()) ? poskey.time[idx2] * 1000 : poskey.time[0] * 1000 + anim_->length * 1000;

            if (time2 > time1) {
                t = (time_ms - time1) / (time2 - time1);
            } else {
                t = (time_ms - time1) / ((anim_->length * 1000) - time1);
            }

            t = std::max(0.0f, std::min(1.0f, t));

            glm::vec3 pos1(
                poskey.data[idx1 * 3],
                poskey.data[idx1 * 3 + 1],
                poskey.data[idx1 * 3 + 2]);

            glm::vec3 pos2;
            if (size_t(idx2) < poskey.time.size()) {
                pos2 = glm::vec3(
                    poskey.data[idx2 * 3],
                    poskey.data[idx2 * 3 + 1],
                    poskey.data[idx2 * 3 + 2]);
            } else {
                pos2 = glm::vec3(
                    poskey.data[0],
                    poskey.data[1],
                    poskey.data[2]);
            }

            node->position_ = glm::mix(pos1, pos2, t);
        }

        auto orikey = anim->get_controller(nw::model::ControllerType::Orientation, true);
        if (orikey.time.size() > 0) {

            int idx1 = -1;
            int idx2 = 0;
            float t = 0.0f;

            for (size_t i = 0; i < orikey.time.size(); ++i) {
                if (time_ms >= orikey.time[i] * 1000) {
                    idx1 = static_cast<int>(i);
                } else {
                    idx2 = static_cast<int>(i);
                    break;
                }
            }

            if (idx1 == -1) {
                idx1 = static_cast<int>(orikey.time.size() - 1);
                idx2 = 0;
            } else if (idx2 == 0 && idx1 != -1) {
                idx2 = 0;
            }

            float time1 = orikey.time[idx1] * 1000;
            float time2 = (size_t(idx2) < orikey.time.size()) ? orikey.time[idx2] * 1000 : orikey.time[0] * 1000 + anim_->length * 1000;

            if (time2 > time1) {
                t = (time_ms - time1) / (time2 - time1);
            } else {
                t = (time_ms - time1) / ((anim_->length * 1000) - time1);
            }

            t = std::max(0.0f, std::min(1.0f, t));

            glm::quat rot1(
                orikey.data[idx1 * 4 + 3],
                orikey.data[idx1 * 4],
                orikey.data[idx1 * 4 + 1],
                orikey.data[idx1 * 4 + 2]);

            glm::quat rot2;
            if (size_t(idx2) < orikey.time.size()) {
                rot2 = glm::quat(
                    orikey.data[idx2 * 4 + 3],
                    orikey.data[idx2 * 4],
                    orikey.data[idx2 * 4 + 1],
                    orikey.data[idx2 * 4 + 2]);
            } else {
                rot2 = glm::quat(
                    orikey.data[0 + 3],
                    orikey.data[0],
                    orikey.data[1],
                    orikey.data[2]);
            }
            node->rotation_ = glm::slerp(rot1, rot2, t);
        }
    }
}

std::unique_ptr<Model> load_model(std::string_view resref)
{
    auto model = nw::kernel::models().load(resref);
    if (!model) { return {}; }

    auto mdl = std::make_unique<Model>();
    if (!mdl->load(&model->model)) {
        LOG_F(ERROR, "Failed to load model: {}", resref);
        return {};
    }
    return mdl;
}

// == BasicTileArea ===========================================================
// ============================================================================

BasicTileArea::BasicTileArea(nw::Area* area)
    : area_{area}
{
}

void BasicTileArea::draw(RenderContext& ctx, const glm::mat4x4& mtx)
{
    for (const auto& tile : tile_models_) {
        auto trans = glm::translate(mtx, tile->position_);
        trans = trans * glm::toMat4(tile->rotation_);
        trans = glm::scale(trans, tile->scale_);
        tile->draw(ctx, trans);
    }
}

void BasicTileArea::load_tile_models()
{
    for (size_t h = 0; h < static_cast<size_t>(area_->height); ++h) {
        for (size_t w = 0; w < static_cast<size_t>(area_->width); ++w) {
            auto idx = h * area_->width + w;
            const auto& at = area_->tiles[idx];
            auto mdl = load_model(area_->tileset->tiles.at(at.id).model);

            auto x = w * 10.0f + 5.0f;
            auto y = h * 10.0f + 5.0f;
            auto z = at.height * area_->tileset->tile_height;
            mdl->position_ = glm::vec3(x, y, z);
            mdl->rotation_ = glm::angleAxis(glm::radians(at.orientation * 90.0f), glm::vec3{0.0f, 0.0f, 1.0f});

            tile_models_.push_back(std::move(mdl));
        }
    }
}

void BasicTileArea::update(int32_t dt)
{
    for (const auto& tile : tile_models_) {
        tile->update(dt);
    }
}
