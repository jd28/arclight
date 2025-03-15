#include "areamodelview.h"

#include <nw/objects/Area.hpp>

#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_transform.hpp"

#include "../../services/renderer/renderservice.h"

#include <QMouseEvent>
#include <QTimer>
#include <QWheelEvent>

ModelView::ModelView(QWidget* parent)
    : RenderWidget(parent)
{
    setFocusPolicy(Qt::StrongFocus);
    setAttribute(Qt::WA_InputMethodEnabled);

    // Initialize camera parameters
    cameraPosition = glm::vec3(0.0f, 0.0f, 50.0f);
    cameraFront = glm::vec3(0.0f, 1.0f, 0.0f);
    cameraUp = glm::vec3(0.0f, 0.0f, 1.0f);
    cameraRight = glm::vec3(1.0f, 0.0f, 0.0f);
    yaw = 0.0f;
    pitch = -89.0f;
    updateCameraVectors();

    QTimer* timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [this]() {
        if (isVisible() && !isHidden() && isActiveWindow()) {
            if (node_) {
                node_->update(16);
            }
            render();
        }
    });
    timer->start(16);
}

void ModelView::setNode(BasicTileArea* node)
{
    node_ = node;

    if (node_) {
        float centerX = (node_->area_->width * 10.0f) / 2.0f;
        float centerY = (node_->area_->height * 10.0f) / 2.0f;

        cameraPosition = glm::vec3(centerX, centerY, 50.0f);
        updateCameraVectors();
    }

    QTimer::singleShot(0, this, [this]() {
        this->setFocus(Qt::OtherFocusReason);
        this->activateWindow();
    });
}

void ModelView::keyPressEvent(QKeyEvent* event)
{
    Qt::KeyboardModifiers modifiers = event->modifiers();

    switch (event->key()) {
    case Qt::Key_W:
        moveCameraForward();
        break;
    case Qt::Key_S:
        moveCameraBackward();
        break;
    case Qt::Key_A:
        moveCameraLeft();
        break;
    case Qt::Key_D:
        moveCameraRight();
        break;
    case Qt::Key_Left:
        yawCameraLeft();
        break;
    case Qt::Key_Right:
        yawCameraRight();
        break;
    }

    if (modifiers & Qt::ControlModifier) {
        switch (event->key()) {
        case Qt::Key_Up:
            increasePitch();
            break;
        case Qt::Key_Down:
            decreasePitch();
            break;
        }
    } else {
        switch (event->key()) {
        case Qt::Key_Up:
            moveCameraUp();
            break;
        case Qt::Key_Down:
            moveCameraDown();
            break;
        }
    }
}

void ModelView::wheelEvent(QWheelEvent* event)
{
    if (event->angleDelta().y() > 0) {
        cameraPosition += cameraFront * wheelSpeed;
    } else {
        cameraPosition -= cameraFront * wheelSpeed;
    }
    cameraPosition.z = std::max(1.0f, cameraPosition.z);
}

void ModelView::do_render()
{

    if (node_) {
        glm::mat4 view = glm::lookAt(cameraPosition, cameraPosition + cameraFront, cameraUp);

        float aspect = static_cast<float>(this->width()) / static_cast<float>(this->height());
        auto proj = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 1000.0f);

        RenderContext ctx{view, proj};
        glm::mat4 mtx{1.0f};
        node_->draw(ctx, mtx);
    }
}

void ModelView::yawCameraLeft()
{
    yaw -= rotationSpeed;
    // Keep yaw in the range [0, 360] for consistency
    if (yaw < 0.0f) yaw += 360.0f;
    updateCameraVectors();
}

void ModelView::yawCameraRight()
{
    yaw += rotationSpeed;
    // Keep yaw in the range [0, 360] for consistency
    if (yaw >= 360.0f) yaw -= 360.0f;
    updateCameraVectors();
}

void ModelView::increasePitch()
{
    pitch += rotationSpeed;
    if (pitch > 89.0f) pitch = 89.0f;
    updateCameraVectors();
}

void ModelView::decreasePitch()
{
    pitch -= rotationSpeed;
    if (pitch < -89.0f) pitch = -89.0f;
    updateCameraVectors();
}

void ModelView::moveCameraForward()
{
    // Move in XY plane while preserving Z height
    glm::vec3 forward = glm::normalize(glm::vec3(cameraFront.x, cameraFront.y, 0.0f));
    cameraPosition += forward * movementSpeed;
}

void ModelView::moveCameraBackward()
{
    // Move in XY plane while preserving Z height
    glm::vec3 forward = glm::normalize(glm::vec3(cameraFront.x, cameraFront.y, 0.0f));
    cameraPosition -= forward * movementSpeed;
}

void ModelView::moveCameraLeft()
{
    cameraPosition -= cameraRight * movementSpeed;
}

void ModelView::moveCameraRight()
{
    cameraPosition += cameraRight * movementSpeed;
}

void ModelView::moveCameraUp()
{
    cameraPosition += cameraUp * movementSpeed; // Now moves along Z-axis
}

void ModelView::moveCameraDown()
{
    cameraPosition -= cameraUp * movementSpeed;          // Now moves along Z-axis
    cameraPosition.z = std::max(1.0f, cameraPosition.z); // Restrict minimum Z height
    updateCameraVectors();
}

void ModelView::updateCameraVectors()
{
    // Clamp pitch to avoid gimbal lock
    if (pitch > 89.0f) pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;

    // Calculate front vector correctly for a Z-up world
    glm::vec3 front;
    front.x = cos(glm::radians(pitch)) * sin(glm::radians(yaw));
    front.y = cos(glm::radians(pitch)) * cos(glm::radians(yaw));
    front.z = sin(glm::radians(pitch));

    cameraFront = glm::normalize(front);

    // Calculate right vector (always perpendicular to world up and front)
    cameraRight = glm::normalize(glm::cross(cameraFront, glm::vec3(0.0f, 0.0f, 1.0f)));
}
