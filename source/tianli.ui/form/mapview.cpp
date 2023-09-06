#include "mapview.h"
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsPixmapItem>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QTimer>
MapView::MapView(QWidget *parent) : QWidget(parent)
{
    ui.setupUi(this);
    this->timer = new QTimer(this);
    connect(this->timer, &QTimer::timeout, this, [=]
            { this->update(); });
    this->timer->start(1000 / 60);
}

MapView::~MapView()
{
}

MapSprite MapView::view_sprite()
{
    return MapSprite{cv::Point2d(this->map_pos.x(), this->map_pos.y()), this->map_scale, cv::Size(this->width(), this->height())};
}

void MapView::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        this->is_move = true;
        this->move_mouse_pos = event->pos();
    }
}

void MapView::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        this->is_move = false;
    }
}

void MapView::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton && this->is_move)
    {
        auto pos = event->pos();
        auto offset = pos - this->move_mouse_pos;
        this->move_mouse_pos = pos;
        this->map_pos -= offset / this->map_scale;
        update();
    }
}

void MapView::wheelEvent(QWheelEvent *event)
{
    auto pos = event->pos();
    auto delta = event->delta();
    if (delta > 0)
    {
        this->map_scale *= 1.1;
    }
    else
    {
        this->map_scale /= 1.1;
    }
    this->map_pos += (pos - this->rect().center()) / this->map_scale;
}

void MapView::paintEvent(QPaintEvent *event)
{
    // draw background
    QPainter painter(this);
    painter.setBrush(QBrush(QColor(0, 0, 0, 128)));
    painter.drawRect(this->rect());
    painter.setBrush(QBrush(QColor(255, 255, 255, 255)));
    painter.drawArc(this->rect(), 0, 360 * 16);
    painter.setBrush(QBrush(QColor(0, 255, 0, 255)));
    painter.drawText(this->rect(), Qt::AlignCenter, QString::number(this->map_pos.x()) + ", " + QString::number(this->map_pos.y()) + ", " + QString::number(this->map_scale));
    painter.setBrush(QBrush(QColor(255, 255, 255, 255)));

    auto sprite = this->view_sprite();
    auto rect = cv::Rect2d(sprite.pos.x, sprite.pos.y, sprite.size.width * sprite.scale, sprite.size.height * sprite.scale);
    auto rs = map.set->find(rect);
    for (auto &r : rs)
    {
        auto pos = r->pos();
        auto p = QPointF((pos.x - this->map_pos.x()) * this->map_scale, (pos.y - this->map_pos.y()) * this->map_scale);
        // p -= this->rect().center();
        painter.drawEllipse(p, 5, 5);
    }

    painter.end();
}