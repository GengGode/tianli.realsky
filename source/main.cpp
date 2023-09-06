#include <QApplication>
#include <QFontDatabase>
#include <QDirIterator>
#include <QDebug>
#include "tianli.init.h"
#include "tianli.ui/tianli.window.h"

int main(int argc, char *argv[])
{
    tianli_init(argc, argv);

    QApplication app(argc, argv);
    TianLiWindow window;

    // 设置窗口标题
    window.setWindowTitle("天理");
    window.setWindowIcon(QIcon(":/icon/resource/icon/tianli.png"));

    window.show();
    return app.exec();
}