#pragma once
#include <QWidget>
#include "ui_tianli.map.h"

class TianLiMap : public QWidget
{
    Q_OBJECT

public:
    explicit TianLiMap(QWidget *parent = 0);
    ~TianLiMap();

private:
    Ui::TianLiMapClass ui;
};