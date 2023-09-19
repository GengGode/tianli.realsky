#include "mapview.h"
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsPixmapItem>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QTimer>
#include <QDebug>
#include "../../tianli.utils/utils.operation.image.h"
#include "../../tianli.utils/utils.convect.image.h"
#include "../../tianli.utils/utils.convect.string.h"

MapView::MapView(QWidget *parent) : QWidget(parent)
{
    ui.setupUi(this);
    // this->timer = new QTimer(this);
    //  connect(this->timer, &QTimer::timeout, this, [=]
    //          {
    //              static MapSprite old = this->view_sprite();
    //              if (old.pos != this->view_sprite().pos || old.scale != this->view_sprite().scale)
    //                  this->update();
    //              old = this->view_sprite(); });
    //  this->timer->start(1000 / 60);
    QImage mask(":/form/resource/form/mapview/rect_mask.png");

    map.set_mask(utils::qimage_to_mat(mask));
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
    auto old_scale = this->map_scale;
    if (delta > 0)
    {
        this->map_scale *= 1.1;
    }
    else
    {
        this->map_scale /= 1.1;
    }
    qDebug() << (pos - this->rect().center());
    this->map_pos += (pos - this->rect().center()) * (old_scale - this->map_scale);
    update();
}

void MapView::paintEvent(QPaintEvent *event)
{
    static bool is_paint = false;
    if (is_paint)
        return;
    is_paint = true;
    QPainter painter(this);

    auto sprite = this->view_sprite();
    cv::Mat mat = map.view(sprite);
    painter.drawImage(this->rect(), utils::mat_to_qimage(mat));
    painter.end();
    is_paint = false;
}