#include "basicmodelview.h"

#include "../renderer/TextureCache.hpp"

#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_transform.hpp"

#include "nw/kernel/TwoDACache.hpp"
#include "nw/objects/Creature.hpp"

#include <QMouseEvent>
#include <QOpenGLDebugLogger>
#include <QOpenGLVersionFunctionsFactory>
#include <QTimer>
#include <QWheelEvent>

TextureCache s_textures;

BasicModelView::BasicModelView(QWidget* parent)
    : QOpenGLWidget(parent)
{
    QSurfaceFormat fmt;
    fmt.setSamples(16);
    fmt.setDepthBufferSize(24);
    fmt.setVersion(3, 3);
    fmt.setProfile(QSurfaceFormat::CoreProfile);
    setFormat(fmt);

    azimuth_ = 0.0;     // Start facing the positive X axis
    declination_ = 0.0; // Start looking straight ahead
    distance_ = 8.0;    // Start 3 units away from the origin

    QTimer* timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &BasicModelView::onUpdateModelAnimation);
    timer->start(16);
}

void BasicModelView::initializeGL()
{
    context_ = context();
    funcs_ = QOpenGLVersionFunctionsFactory::get<QOpenGLFunctions_3_3_Core>(context_);

    funcs_->glEnable(GL_DEPTH_TEST);
    funcs_->glEnable(GL_CULL_FACE);

    QOpenGLDebugLogger* logger = new QOpenGLDebugLogger(this);
    logger->initialize();
    connect(logger, &QOpenGLDebugLogger::messageLogged, this, [](const QOpenGLDebugMessage& msg) {
        LOG_F(INFO, "open gl error: {}", msg.message().toStdString());
    });
    logger->startLogging();

    s_textures.load_placeholder(funcs_);
    s_textures.load_palette_texture(funcs_);
    shader_.basic.load(R"(
        #version 330 core
        layout (location = 0) in vec3 aPos;
        layout (location = 1) in vec2 aTexCoord;
        layout (location = 2) in vec3 aNormal;
        layout (location = 3) in vec4 aTangent;
        
        out vec2 TexCoord;
        
        uniform mat4 model;
        uniform mat4 view;
        uniform mat4 projection;
        
        void main()
        {
            gl_Position = projection * view * model * vec4(aPos, 1.0);
            TexCoord = aTexCoord;
        })",
        R"(
        #version 330 core
        out vec4 FragColor;
        
        in vec2 TexCoord;
        
        uniform sampler2D ourTexture;
        
        void main()
        {
            FragColor = texture(ourTexture, TexCoord);
        })",
        funcs_);

    shader_.skin.load(R"(
        #version 330 core
        layout(location = 0) in vec3 aPos;
        layout(location = 1) in vec2 aTexCoord;
        layout(location = 2) in vec3 aNormal;
        layout(location = 3) in vec4 aTangent;
        layout(location = 4) in ivec4 aIndices;
        layout(location = 5) in vec4 aWeights;
        
        out vec2 TexCoord;
        
        uniform mat4 model;
        uniform mat4 view;
        uniform mat4 projection;

        const int MAX_BONES = 64;
        const int MAX_BONE_INFLUENCE = 4;
        uniform mat4 joints[MAX_BONES];
        
        void main()
        {
            vec4 localPosition = vec4(0.0f);
            for(int i = 0 ; i < MAX_BONE_INFLUENCE ; i++)
            {
                if(aIndices[i] == -1) 
                    continue;
                if(aIndices[i] >= MAX_BONES) 
                {
                    localPosition = vec4(aPos,1.0f);
                    break;
                }
                mat4 boneTransform = joints[aIndices[i]];
                vec4 posePosition = boneTransform * vec4(aPos,1.0f);
                localPosition += posePosition * aWeights[i];
            }
                	
            gl_Position =  projection *  view * model * localPosition;
            TexCoord = aTexCoord;
        })",
        R"(
        #version 330 core
        out vec4 FragColor;
        
        in vec2 TexCoord;
        
        uniform sampler2D ourTexture;
        
        void main()
        {
            FragColor = texture(ourTexture, TexCoord);
        })",
        funcs_);

    emit initialized();
}

void BasicModelView::onUpdateModelAnimation()
{
    if (isVisible() && current_model_) {
        current_model_->update(16);
        update();
    }
}

void BasicModelView::resizeGL(int w, int h)
{
    funcs_->glViewport(0, 0, w, h);
    height_ = float(h);
    width_ = float(w);
}

void BasicModelView::setCreature(nw::Creature* creature)
{
    creature_ = creature;
    if (current_appearance_ == *creature_->appearance.id) {
        return;
    }
    current_appearance_ = *creature_->appearance.id;

    auto appearances_2da = nw::kernel::twodas().get("appearance");
    std::string model;
    // [TODO] Can't do parts based models yet..
    if (!appearances_2da->get_to(*creature_->appearance.id, "RACE", model) || model.length() <= 1) {
        current_model_.reset();
    } else {
        nw::Resref resref{model};

        current_model_ = load_model(resref.view(), funcs_);
        if (current_model_) {
            if (!current_model_->load_animation("pause1")) {
                current_model_->load_animation("cpause1");
            }
        }
    }

    update();
}

void BasicModelView::setModel(std::unique_ptr<Model> model)
{
    current_model_ = std::move(model);
    update();
}

void BasicModelView::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        last_pos_ = event->pos();
    }
}

void BasicModelView::mouseMoveEvent(QMouseEvent* event)
{
    if (event->buttons() & Qt::LeftButton) {
        int dx = int(event->position().x() - last_pos_.x());
        int dy = int(event->position().y() - last_pos_.y());
        azimuth_ -= dx * 0.5 * 0.01;
        declination_ += dy * 0.5 * 0.01;
        last_pos_ = event->pos();
        update();
    }
}

void BasicModelView::wheelEvent(QWheelEvent* event)
{
    int num_degrees = event->angleDelta().y() / 8;
    int num_steps = num_degrees / 15;
    distance_ += num_steps;
    update();
}

void BasicModelView::paintGL()
{
    auto gl = funcs_;
    gl->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    gl->glClearColor(0.2f, 0.3f, 0.3f, 1.0f);

    if (current_model_) {
        GLfloat camX = distance_ * sin(azimuth_) * cos(declination_);
        GLfloat camY = distance_ * sin(declination_);
        GLfloat camZ = distance_ * cos(azimuth_) * cos(declination_);
        auto view = glm::lookAt(glm::vec3{camX, camY, camZ}, glm::vec3{0, 0, 0}, glm::vec3{0, 1, 0});
        float aspect = width_ / height_;
        auto proj = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 100.0f);

        shader_.skin.use(gl);
        CHECK_GL_ERRORS();
        shader_.skin.set_uniform("view", view, gl);
        CHECK_GL_ERRORS();
        shader_.skin.set_uniform("projection", proj, gl);
        CHECK_GL_ERRORS();
        gl->glUseProgram(0);

        shader_.basic.use(gl);
        CHECK_GL_ERRORS();
        shader_.basic.set_uniform("view", view, gl);
        CHECK_GL_ERRORS();
        shader_.basic.set_uniform("projection", proj, gl);
        CHECK_GL_ERRORS();
        gl->glUseProgram(0);

        glm::mat4 mtx = glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), {0.0f, 0.0f, 1.0f});
        mtx = glm::rotate(mtx, glm::radians(90.0f), {1.0f, 0.0f, 0.0f});
        mtx = glm::translate(mtx, {0.0f, 1.0f, -2.0f});
        current_model_->draw(shader_, mtx, gl);
        gl->glUseProgram(0);
    }
}

void BasicModelView::onDataChanged()
{
    makeCurrent();
    setCreature(creature_);
}
