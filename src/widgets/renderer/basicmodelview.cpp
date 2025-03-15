#include "basicmodelview.h"

#include "../../services/renderer/renderservice.h"

#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_transform.hpp"
#include <glm/ext.hpp>

#include <QMouseEvent>
#include <QTimer>
#include <QWheelEvent>

BasicModelView::BasicModelView(QWidget* parent)
    : RenderWidget(parent)
{

    azimuth_ = 0.0f;
    declination_ = 0.0f;
    distance_ = 8.0f;

    QTimer* timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [this]() {
        if (isVisible() && !isHidden() && isActiveWindow()) {
            if (current_model_) {
                current_model_->update(16);
            }
            render();
        }
    });
    timer->start(16);
}

void BasicModelView::setModel(std::unique_ptr<Model> model)
{
    current_model_ = std::move(model);
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
        azimuth_ -= dx * 0.01f;
        declination_ += dy * 0.01f;
        declination_ = std::clamp(declination_, glm::radians(-89.0f), glm::radians(89.0f)); // Avoid poles
        last_pos_ = event->pos();
    }
}

void BasicModelView::wheelEvent(QWheelEvent* event)
{
    int num_degrees = event->angleDelta().y() / 8;
    int num_steps = num_degrees / 15;
    distance_ -= num_steps;
    distance_ = std::clamp(distance_, 1.0f, 1000.0f);
}

void BasicModelView::do_render()
{
    if (!current_model_) {
        return;
    }

    float camX = distance_ * sin(azimuth_) * cos(declination_);
    float camY = distance_ * cos(azimuth_) * cos(declination_);
    float camZ = distance_ * sin(declination_);

    auto view = glm::lookAt(
        glm::vec3{camX, camY, camZ},
        glm::vec3{0.0f, 0.0f, 0.0f},
        glm::vec3{0.0f, 0.0f, 1.0f});

    float aspect = float(this->width()) / float(this->height());
    auto proj = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 100.0f);

    RenderContext ctx;
    ctx.view = view;
    ctx.projection = proj;

    auto mtx = glm::mat4(1.0f);
    current_model_->draw(ctx, mtx);
}
