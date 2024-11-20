#include "model.hpp"

#include "TextureCache.hpp"
#include "shader.hpp"

#include "nw/formats/Tileset.hpp"
#include "nw/kernel/ModelCache.hpp"
#include "nw/objects/Area.hpp"

#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>

#include <QOpenGLFunctions_3_3_Core>
#include <QtOpenGL/QOpenGLBuffer>
#include <QtOpenGL/QOpenGLShaderProgram>

extern TextureCache s_textures;

void Node::draw(ShaderPrograms& shader, const glm::mat4x4& mtx, QOpenGLFunctions_3_3_Core* gl)
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
        child->draw(shader, trans, gl);
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

void Mesh::draw(ShaderPrograms& shader, const glm::mat4x4& mtx, QOpenGLFunctions_3_3_Core* gl)
{
    auto trans = glm::translate(mtx, position_);
    trans = trans * glm::toMat4(rotation_);
    trans = glm::scale(trans, scale_);

    shader.basic.use(gl);
    if (!no_render_) {
        shader.basic.set_uniform("model", trans, gl);

        gl->glActiveTexture(GL_TEXTURE0);
        gl->glBindTexture(GL_TEXTURE_2D, texture0);

        // if (texture0_is_plt) {
        //     for (size_t i = 0; i < 10; ++i) {
        //         gl->glActiveTexture(GL_TEXTURE0 + 1 + i);
        //         gl->glBindTexture(GL_TEXTURE_2D, s_textures.palette_texture_[i]);
        //         CHECK_GL_ERRORS();
        //     }
        // }

        gl->glBindVertexArray(vao_);
        CHECK_GL_ERRORS();
        auto orig = static_cast<nw::model::TrimeshNode*>(orig_);
        gl->glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(orig->indices.size()), GL_UNSIGNED_SHORT, 0);
        CHECK_GL_ERRORS();
        gl->glBindVertexArray(0);
    }

    for (auto child : children_) {
        child->draw(shader, trans, gl);
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

void Skin::draw(ShaderPrograms& shader, const glm::mat4x4& mtx, QOpenGLFunctions_3_3_Core* gl)
{
    auto orig = static_cast<nw::model::SkinNode*>(orig_);
    for (size_t i = 0; i < 64; ++i) {
        if (orig->bone_nodes[i] <= 0 || size_t(orig->bone_nodes[i]) >= owner_->nodes_.size()) {
            break;
        }
        auto bone_node = owner_->nodes_[orig->bone_nodes[i]].get();
        joints_[i] = bone_node->get_transform() * inverse_bind_pose_[orig->bone_nodes[i]];
    }

    shader.skin.use(gl);
    CHECK_GL_ERRORS();
    shader.skin.set_uniform("model", mtx, gl);
    CHECK_GL_ERRORS();
    unsigned int joints_loc = gl->glGetUniformLocation(shader.skin.id_, "joints");
    CHECK_GL_ERRORS();
    gl->glUniformMatrix4fv(joints_loc, 64, GL_FALSE, glm::value_ptr(joints_[0]));
    CHECK_GL_ERRORS();

    gl->glActiveTexture(GL_TEXTURE0);
    CHECK_GL_ERRORS();
    gl->glBindTexture(GL_TEXTURE_2D, texture0);
    CHECK_GL_ERRORS();

    // if (texture0_is_plt) {
    //     for (size_t i = 0; i < 10; ++i) {
    //         gl->glActiveTexture(GL_TEXTURE0 + 1 + i);
    //         gl->glBindTexture(GL_TEXTURE_2D, s_textures.palette_texture_[i]);
    //         CHECK_GL_ERRORS();
    //     }
    // }

    gl->glBindVertexArray(vao_);
    CHECK_GL_ERRORS();
    gl->glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(orig->indices.size()), GL_UNSIGNED_SHORT, 0);
    CHECK_GL_ERRORS();

    gl->glBindVertexArray(0);

    for (auto child : children_) {
        child->draw(shader, mtx, gl);
    }
}

// == Model ===================================================================
// ============================================================================

void Model::draw(ShaderPrograms& shader, const glm::mat4x4& mtx, QOpenGLFunctions_3_3_Core* gl)
{
    nodes_[0]->draw(shader, mtx, gl);
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

bool Model::load(nw::model::Model* mdl, QOpenGLFunctions_3_3_Core* gl)
{
    auto root = mdl->find(std::regex(mdl->name));
    if (!root) {
        LOG_F(INFO, "No root dummy");
        return false;
    }
    mdl_ = mdl;
    if (load_node(root, gl)) {
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

Node* Model::load_node(nw::model::Node* node, QOpenGLFunctions_3_3_Core* gl, Node* parent)
{
    Node* result = nullptr;
    if (node->type & nw::model::NodeFlags::skin) {
        auto n = static_cast<nw::model::SkinNode*>(node);
        if (!n->indices.empty()) {
            Skin* skin = new Skin;

            gl->glGenVertexArrays(1, &skin->vao_);
            CHECK_GL_ERRORS();
            gl->glGenBuffers(1, &skin->vbo_);
            CHECK_GL_ERRORS();
            gl->glGenBuffers(1, &skin->ebo_);
            CHECK_GL_ERRORS();

            gl->glBindVertexArray(skin->vao_);
            CHECK_GL_ERRORS();
            gl->glBindBuffer(GL_ARRAY_BUFFER, skin->vbo_);
            CHECK_GL_ERRORS();

            gl->glBufferData(GL_ARRAY_BUFFER, n->vertices.size() * sizeof(nw::model::SkinVertex), &n->vertices[0],
                GL_STATIC_DRAW);
            CHECK_GL_ERRORS();

            gl->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, skin->ebo_);
            CHECK_GL_ERRORS();

            gl->glBufferData(GL_ELEMENT_ARRAY_BUFFER, n->indices.size() * sizeof(uint16_t),
                &n->indices[0], GL_STATIC_DRAW);
            CHECK_GL_ERRORS();

            // vertex positions
            gl->glEnableVertexAttribArray(0);
            gl->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(nw::model::SkinVertex), nullptr);

            // vertex texture coords
            gl->glEnableVertexAttribArray(1);
            gl->glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(nw::model::SkinVertex),
                reinterpret_cast<void*>(offsetof(nw::model::SkinVertex, tex_coords)));

            // vertex normals
            gl->glEnableVertexAttribArray(2);
            gl->glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(nw::model::SkinVertex),
                reinterpret_cast<void*>(offsetof(nw::model::SkinVertex, normal)));

            // vertex tangents
            gl->glEnableVertexAttribArray(3);
            gl->glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(nw::model::SkinVertex),
                reinterpret_cast<void*>(offsetof(nw::model::SkinVertex, tangent)));

            // vertex bones
            gl->glEnableVertexAttribArray(4);
            gl->glVertexAttribIPointer(4, 4, GL_INT, sizeof(nw::model::SkinVertex),
                reinterpret_cast<void*>(offsetof(nw::model::SkinVertex, bones)));

            // vertex weights
            gl->glEnableVertexAttribArray(5);
            gl->glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(nw::model::SkinVertex),
                reinterpret_cast<void*>(offsetof(nw::model::SkinVertex, weights)));

            gl->glBindVertexArray(0);

            auto tex = s_textures.load(n->bitmap, gl);
            if (tex) {
                skin->texture0 = tex->first;
                skin->texture0_is_plt = tex->second;
            } else {
                LOG_F(FATAL, "Failed to bind texture");
            }
            result = skin;
        } else {
            LOG_F(ERROR, "No vertex indicies");
        }
    } else if (node->type & nw::model::NodeFlags::mesh && !(node->type & nw::model::NodeFlags::aabb)) {
        auto n = static_cast<nw::model::TrimeshNode*>(node);
        if (!n->indices.empty()) {
            Mesh* mesh = new Mesh;
            mesh->orig_ = n;
            mesh->no_render_ = !n->render;

            gl->glGenVertexArrays(1, &mesh->vao_);
            CHECK_GL_ERRORS();
            gl->glGenBuffers(1, &mesh->vbo_);
            CHECK_GL_ERRORS();
            gl->glGenBuffers(1, &mesh->ebo_);
            CHECK_GL_ERRORS();

            gl->glBindVertexArray(mesh->vao_);
            CHECK_GL_ERRORS();
            gl->glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo_);
            CHECK_GL_ERRORS();

            gl->glBufferData(GL_ARRAY_BUFFER, n->vertices.size() * sizeof(nw::model::Vertex), &n->vertices[0],
                GL_STATIC_DRAW);
            CHECK_GL_ERRORS();

            gl->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ebo_);
            CHECK_GL_ERRORS();

            gl->glBufferData(GL_ELEMENT_ARRAY_BUFFER, n->indices.size() * sizeof(uint16_t),
                &n->indices[0], GL_STATIC_DRAW);
            CHECK_GL_ERRORS();

            // vertex positions
            gl->glEnableVertexAttribArray(0);
            gl->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(nw::model::Vertex), nullptr);

            // vertex texture coords
            gl->glEnableVertexAttribArray(1);
            gl->glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(nw::model::Vertex),
                reinterpret_cast<void*>(offsetof(nw::model::Vertex, tex_coords)));

            // vertex normals
            gl->glEnableVertexAttribArray(2);
            gl->glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(nw::model::Vertex),
                reinterpret_cast<void*>(offsetof(nw::model::Vertex, normal)));

            // vertex tangents
            gl->glEnableVertexAttribArray(3);
            gl->glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(nw::model::Vertex),
                reinterpret_cast<void*>(offsetof(nw::model::Vertex, tangent)));

            gl->glBindVertexArray(0);

            auto tex = s_textures.load(n->bitmap, gl);
            if (tex) {
                mesh->texture0 = tex->first;
                mesh->texture0_is_plt = tex->second;
            } else {
                LOG_F(FATAL, "Failed to bind texture");
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
        result->children_.push_back(load_node(child, gl, result));
    }

    return result;
}

void Model::update(int32_t dt)
{
    if (!anim_) { return; }

    if (dt + anim_cursor_ > int32_t(anim_->length * 1000)) {
        anim_cursor_ = dt + anim_cursor_ - int32_t(anim_->length * 1000);
    } else {
        anim_cursor_ += dt;
    }

    for (const auto& anim : anim_->nodes) {
        // LOG_F(INFO, "animation node: {}, cursor: {}", anim->name, anim_cursor_);
        auto node = find(anim->name);
        if (node) {
            auto poskey = anim->get_controller(nw::model::ControllerType::Position, true);
            // LOG_F(INFO, "time size: {}, data size: {}", poskey.time.size(), poskey.data.size());
            if (poskey.time.size()) {
                int i = 0;
                int end = 0;
                while (size_t(i) < poskey.time.size() && anim_cursor_ >= int32_t(poskey.time[i] * 1000)) {
                    ++i;
                }
                end = i;
                if (size_t(end) >= poskey.time.size()) { end = 0; }
                node->position_ = glm::vec3{poskey.data[end * 3], poskey.data[end * 3 + 1], poskey.data[end * 3 + 2]};
            }

            auto orikey = anim->get_controller(nw::model::ControllerType::Orientation, true);
            if (orikey.time.size()) {
                int i = 0;
                int end = 0;
                while (size_t(i) < orikey.time.size() && anim_cursor_ >= int32_t(orikey.time[i] * 1000)) {
                    ++i;
                }
                end = i;
                if (size_t(end) >= orikey.time.size()) { end = 0; }
                // start = i - 1;
                // if (start < 0) { start = poskey.time.size() - 1; }
                node->rotation_ = glm::qua{
                    orikey.data[end * 4 + 3],
                    orikey.data[end * 4],
                    orikey.data[end * 4 + 1],
                    orikey.data[end * 4 + 2],
                };
            }
        }
    }
}

std::unique_ptr<Model> load_model(std::string_view resref, QOpenGLFunctions_3_3_Core* gl)
{
    auto model = nw::kernel::models().load(resref);
    if (!model) { return {}; }

    auto mdl = std::make_unique<Model>();
    if (!mdl->load(&model->model, gl)) {
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

void BasicTileArea::draw(ShaderPrograms& shader, const glm::mat4x4& mtx, QOpenGLFunctions_3_3_Core* gl)
{
    for (const auto& tile : tile_models_) {
        auto trans = glm::translate(mtx, tile->position_);
        trans = trans * glm::toMat4(tile->rotation_);
        trans = glm::scale(trans, tile->scale_);

        tile->draw(shader, trans, gl);
    }
}

void BasicTileArea::load_tile_models(QOpenGLFunctions_3_3_Core* gl)
{
    for (size_t h = 0; h < static_cast<size_t>(area_->height); ++h) {
        for (size_t w = 0; w < static_cast<size_t>(area_->width); ++w) {
            auto idx = h * area_->width + w;
            const auto& at = area_->tiles[idx];
            auto mdl = load_model(area_->tileset->tiles.at(at.id).model, gl);

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
