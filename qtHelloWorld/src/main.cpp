#include <QApplication>

#include "MainWindow.h"

int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    MainWindow window(nullptr);
    window.showNormal();

    return app.exec();
}
