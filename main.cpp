#include <QApplication>
#include <QFile>

#include "calculator.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // 从 Qt 资源系统加载样式表
    QFile styleFile(":/qss/style.qss");
    if (styleFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        app.setStyleSheet(QString::fromUtf8(styleFile.readAll()));
    }

    Calculator window;
    window.show();

    return app.exec();
}
