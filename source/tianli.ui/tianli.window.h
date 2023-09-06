#pragma once
#include <QWidget>
#include "ui_tianli.window.h"

class TianLiWindow : public QWidget
{
    Q_OBJECT

public:
    explicit TianLiWindow(QWidget *parent = 0);
    ~TianLiWindow();

private:
    Ui::TianLiWindowClass ui;
};
