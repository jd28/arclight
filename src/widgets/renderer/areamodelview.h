#ifndef AREAMODELVIEW_H
#define AREAMODELVIEW_H

#include "../../services/renderer/model.hpp"
#include "renderwidget.h"

class QMouseEvent;
class QWheelEvent;

class ModelView : public RenderWidget {
    Q_OBJECT
public:
    ModelView(QWidget* parent = nullptr);
    void setNode(BasicTileArea* node);

    void keyPressEvent(QKeyEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

protected:
    // void initializeGL() override;
    void do_render() override;

private:
    BasicTileArea* node_ = nullptr;
    QPoint last_pos_;

    // Camera parameters
    glm::vec3 cameraPosition{0.0f, 45.0f, 0.0f};
    glm::vec3 cameraFront;
    glm::vec3 cameraUp;
    glm::vec3 cameraRight;
    float yaw = 0.0f;
    float pitch = 9.0f;

    // Camera movement and rotation speeds
    float movementSpeed = 0.5f;
    float rotationSpeed = 1.0f;
    float wheelSpeed = 3.0f;

    // Camera control functions
    void moveCameraForward();
    void moveCameraBackward();
    void moveCameraLeft();
    void moveCameraRight();
    void moveCameraUp();
    void moveCameraDown();
    void yawCameraLeft();
    void yawCameraRight();
    void increasePitch();
    void decreasePitch();
    void updateCameraVectors();
};

#endif // AREAMODELVIEW_H
