#pragma once
#include <QWidget>
#include <QGraphicsView>
#include "ui_mapview.h"
#include "../../tianli.core/tianli.map.h"

class MapView : public QWidget
{
    Q_OBJECT

public:
    explicit MapView(QWidget *parent = 0);
    ~MapView();

public:
    Map map;

public:
    QPointF map_pos = QPointF(0, 0);
    double map_scale = 1.0;
    MapSprite view_sprite();

private:
    QPoint move_mouse_pos;
    bool is_move = false;

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private:
    QTimer *timer = nullptr;

private:
    Ui::MapViewClass ui;
};