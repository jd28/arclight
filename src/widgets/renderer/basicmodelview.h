#ifndef CREATUREMODELVIEW_H
#define CREATUREMODELVIEW_H

#include "../../services/renderer/model.hpp"
#include "renderwidget.h"

#include <memory>

class QMouseEvent;
class QWheelEvent;

class BasicModelView : public RenderWidget {
    Q_OBJECT
public:
    BasicModelView(QWidget* parent = nullptr);

    void setModel(std::unique_ptr<Model> model);

    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

protected:
    void do_render() override;

private:
    std::unique_ptr<Model> current_model_ = nullptr;

    QPoint last_pos_;
    float azimuth_;
    float declination_;
    float distance_;
    int current_appearance_ = -1;
};

#endif // CREATUREMODELVIEW_H
