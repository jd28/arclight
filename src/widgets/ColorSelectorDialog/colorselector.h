#ifndef COLORSELECTOR_H
#define COLORSELECTOR_H

#include <QLabel>

class ColorSelector : public QLabel {
    Q_OBJECT

public:
    ColorSelector(QWidget* parent = nullptr);

    void setIndex(int index);
    void setPaletteImage(QPixmap image);

signals:
    void indexChanged(int index);

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void paintEvent(QPaintEvent* event) override;

private:
    QPixmap palette_;
    int selected_ = -1;
};
#endif // COLORSELECTOR_H
