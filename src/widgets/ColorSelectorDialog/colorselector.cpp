#include "colorselector.h"

#include <QMouseEvent>
#include <QPainter>

ColorSelector::ColorSelector(QWidget* parent)
    : QLabel(parent)
    , selected_(false)
{
}

void ColorSelector::setIndex(int index)
{
    selected_ = index;
    if (selected_ >= 0) { update(); }
}

void ColorSelector::setPaletteImage(QPixmap image)
{
    palette_ = image;
    setPixmap(palette_);
    if (selected_ >= 0) { update(); }
}

void ColorSelector::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        QPoint pos = event->pos();
        if (pos.x() < palette_.width() && pos.y() < palette_.height()) {
            int col = pos.x() / 32;
            int row = pos.y() / 32;
            auto new_index = row * 16 + col;
            if (new_index != selected_) {
                selected_ = new_index;
                update();
                emit indexChanged(selected_);
            }
        }
    }
}

void ColorSelector::paintEvent(QPaintEvent* event)
{
    QLabel::paintEvent(event);

    if (selected_ >= 0) {
        int col = selected_ % 16;
        int row = selected_ / 16;

        QPainter painter(this);
        painter.setPen(QPen(Qt::green, 2));

        QRect rect(col * 32, row * 32, 32, 32);
        painter.drawRect(rect);
    }
}
