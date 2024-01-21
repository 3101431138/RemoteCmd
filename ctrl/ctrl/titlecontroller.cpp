#include "titlecontroller.h"

TitleController::TitleController(QObject* parent)
    : QObject(parent)
    , window_(nullptr)
    , title_(nullptr)
    , close_(nullptr)
    , pos_pt_(0, 0)
    , pos_sta_(false)
{
}

void TitleController::setObject(QMainWindow* window, QWidget* title, QPushButton* close)
{
    window_ = window;
    title_  = title;
    close_  = close;
    window_->setMouseTracking(true);
    window_->connect(close_, &QPushButton::clicked, this, &TitleController::on_close);
}

void TitleController::mousePressEvent(QMouseEvent* event)
{
    QPoint rel_wgtTitle_pt = title_->mapFrom(window_, event->pos());

    if (!title_->rect().contains(rel_wgtTitle_pt))
    {
        return;
    }
    if ((event->button() == Qt::LeftButton))
    {
        pos_sta_ = true;
        pos_pt_ = event->pos();
    }
}
void TitleController::mouseMoveEvent(QMouseEvent* event)
{
    if (pos_sta_)
    {
        window_->move(event->globalPos() - pos_pt_);
    }
}
void TitleController::mouseReleaseEvent(QMouseEvent* event)
{
    pos_sta_ = false;
}
void TitleController::on_close()
{
    window_->close();
}