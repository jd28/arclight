#include "mainwindow.h"

#include <ZFontIcon/ZFontIcon.h>
#include <ZFontIcon/ZFont_fa6.h> // FA6 helpers
#include <nowide/args.hpp>
#include <nw/log.hpp>

#include <QApplication>
#include <QStyleFactory>

int main(int argc, char* argv[])
{
    nowide::args(argc, argv);
    nw::init_logger(argc, argv);

    QApplication app{argc, argv};
    QCoreApplication::setApplicationName("erfherder");
    QCoreApplication::setApplicationVersion("1.0.0");

#if defined(Q_OS_WIN)
    app.setStyle(QStyleFactory::create("Fusion"));
    QPalette darkPalette;
    darkPalette.setColor(QPalette::Window, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::WindowText, Qt::white);
    darkPalette.setColor(QPalette::Base, QColor(25, 25, 25));
    darkPalette.setColor(QPalette::AlternateBase, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
    darkPalette.setColor(QPalette::ToolTipText, Qt::white);
    darkPalette.setColor(QPalette::Text, Qt::white);
    darkPalette.setColor(QPalette::Button, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::ButtonText, Qt::white);
    darkPalette.setColor(QPalette::BrightText, Qt::red);
    darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::HighlightedText, Qt::black);
    app.setPalette(darkPalette);
    app.setStyleSheet("QToolTip { color: #ffffff; background-color: #2a82da; border: 1px solid white; }");
#endif

    // FA6 Free
    ZFontIcon::addFont(":/fa6/" + Fa6::FA6_TTF_FILE_FREE_SOLID);

    MainWindow main;
    main.restoreWindow();
    main.show();
    return app.exec();
}
