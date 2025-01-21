#ifndef TEXTBOXDIALOG_H
#define TEXTBOXDIALOG_H

#include <QDialog>
#include <QObject>

class TextBoxDialog : public QDialog {
    Q_OBJECT

public:
    explicit TextBoxDialog(const QString& summary, int width = 400, int height = 300, QWidget* parent = nullptr);
};

#endif // TEXTBOXDIALOG_H
