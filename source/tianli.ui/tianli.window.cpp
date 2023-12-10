#include "tianli.window.h"
#include "form/mapview.h"
#include <QDebug>

TianLiWindow::TianLiWindow(QWidget *parent) : QWidget(parent)
{
    ui.setupUi(this);
    // 设置窗口透明
    setAttribute(Qt::WA_TranslucentBackground);
    // 设置窗口无边框
    // setWindowFlags(Qt::FramelessWindowHint);

    // 自动继承MapView
    // MapView *mapView = new MapView(ui.widget);
    this->layout()->addWidget(new MapView(this));

    /***********************/
}

TianLiWindow::~TianLiWindow()
{
}