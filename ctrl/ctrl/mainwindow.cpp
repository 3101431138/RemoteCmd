#include "mainwindow.h"
#include "qtutil.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);

    title_ctrl_.setObject(this, ui.wgtTitle, ui.close_btn);

    QTUtil::RemoveFrame(*this);
    QTUtil::SetTransparent(*this);
    QTUtil::SetShadow(*ui.wgtBack);
    QTUtil::SetShadow(*ui.lbTitle, QColor(0xf4, 0xf4, 0xf4), 5);

    ui.tabWidget->setTabsClosable(true);

    ui.icon_btn->installEventFilter(this);
    ui.lbTitle->installEventFilter(this);
    installEventFilter(this);

    connect(ui.tabWidget, &QTabWidget::tabCloseRequested, this, &MainWindow::on_tabWidget_tabCloseRequested);
    connect(ui.icon_btn, &QPushButton::clicked, this, &MainWindow::on_icon_btn_clicked);

    serv_.set_connect_cb(std::bind(&MainWindow::on_user_connect, this, std::placeholders::_1));
    serv_.set_disconnect_cb(std::bind(&MainWindow::on_user_disconnect, this, std::placeholders::_1));
    serv_.set_read_packet_cb(std::bind(&MainWindow::on_user_read_packet, this, std::placeholders::_1, std::placeholders::_2));
    serv_.run();
}

void MainWindow::on_tabWidget_tabCloseRequested(int index)
{
    std::map<std::string, std::shared_ptr<CmdSession>>::iterator it_dst = cmd_session_map_.end();
    for(std::map<std::string, std::shared_ptr<CmdSession>>::iterator it = cmd_session_map_.begin(); it != cmd_session_map_.end(); it++)
    {
        if(index == ui.tabWidget->indexOf(it->second.get()))
        {
            ui.tabWidget->removeTab(index);
            it_dst = it;
            break;
        }
    }
    if(it_dst != cmd_session_map_.end())
    {
        cmd_session_map_.erase(it_dst);
    }
}


void MainWindow::on_icon_btn_clicked()
{
    QSize expand  (200, ui.tabWidget->height());
    QSize collapse(000, 000);

    if(ui.icon_btn->isChecked())
    {
        //为侧边栏的最小大小创建属性动画
        QPropertyAnimation* animation = new QPropertyAnimation(ui.sideBar, "minimumSize");
        animation->setDuration(500);                                        //设置动画完成的时间长度
        animation->setStartValue(collapse);                                 //设置动画的开始值
        animation->setEndValue(expand);                                     //设置动画的结束值
        animation->setEasingCurve(QEasingCurve::OutBounce);                 //设置插值效果
        animation->start(QAbstractAnimation::DeleteWhenStopped);            //启动动画
    }
    else
    {
        //为侧边栏的最小大小创建属性动画
        QPropertyAnimation* animation = new QPropertyAnimation(ui.sideBar, "minimumSize");
        animation->setDuration(500);                                        //设置动画完成的时间长度
        animation->setStartValue(expand);                                   //设置动画的开始值
        animation->setEndValue(collapse);                                   //设置动画的结束值
        animation->setEasingCurve(QEasingCurve::OutBounce);                 //设置插值效果
        animation->start(QAbstractAnimation::DeleteWhenStopped);            //启动动画
    }
}

void MainWindow::on_user_connect(session_ptr sess)
{
    QMetaObject::invokeMethod(this, [this, sess]()
    {
        on_user_connect_ui(sess);
    });
}

void MainWindow::on_user_disconnect(session_ptr sess)
{
    QMetaObject::invokeMethod(this, [this, sess]()
    {
        on_user_disconnect_ui(sess);
    });
}

void MainWindow::on_user_read_packet(session_ptr sess, packet_ptr pkt)
{
    QMetaObject::invokeMethod(this, [this, sess, pkt]()
    {
        on_user_read_packet_ui(sess, pkt);
    });
}

void MainWindow::on_user_connect_ui(session_ptr sess)
{
    cmd_session_map_[sess->id()] = std::make_shared<CmdSession>(sess);
    ui.tabWidget->addTab(cmd_session_map_[sess->id()].get(), sess->id().c_str());
}

void MainWindow::on_user_disconnect_ui(session_ptr sess)
{
    ui.tabWidget->removeTab(ui.tabWidget->indexOf(cmd_session_map_[sess->id()].get()));
}

void MainWindow::on_user_read_packet_ui(session_ptr sess, packet_ptr pkt)
{
    cmd_session_map_[sess->id()]->dispaly_cmd_result(QString::fromLocal8Bit(pkt->data.c_str()));
}

void MainWindow::mousePressEvent(QMouseEvent* event)
{
    title_ctrl_.mousePressEvent(event);
}
void MainWindow::mouseMoveEvent(QMouseEvent* event)
{
    title_ctrl_.mouseMoveEvent(event);
}
void MainWindow::mouseReleaseEvent(QMouseEvent* event)
{
    title_ctrl_.mouseReleaseEvent(event);
}

bool MainWindow::eventFilter(QObject* obj, QEvent* event)
{
    int SIZE = 10;

    if(event->type() == QEvent::HoverMove)
    {
        QHoverEvent* eve = static_cast<QHoverEvent*>(event);

        setCursor(Qt::ArrowCursor);
        
        QRect left_top_rect (0, 0, SIZE, SIZE);
        QRect mid_top_rect  (SIZE, 0, width() - 2 * SIZE, SIZE);
        QRect right_top_rect(width() - SIZE, 0, SIZE, SIZE);

        QRect left_mid_rect (0, SIZE, SIZE, height() - 2 * SIZE);
        QRect right_mid_rect(width() - SIZE, SIZE, SIZE, height() - 2 * SIZE);

        QRect left_bottom_rect(0, height() - SIZE, SIZE, SIZE);
        QRect mid_bottom_rect (SIZE, height() - SIZE, width() - 2 * SIZE, SIZE);
        QRect right_bottom_rect(width() - SIZE, height() - SIZE, SIZE, SIZE);

        if (left_top_rect.contains(eve->pos()))
        {
            setCursor(Qt::SizeFDiagCursor);
        }

        if (mid_top_rect.contains(eve->pos()))
        {
            setCursor(Qt::SizeVerCursor);
        }

        if (right_top_rect.contains(eve->pos()))
        {
            setCursor(Qt::SizeBDiagCursor);
        }

        if (left_mid_rect.contains(eve->pos()))
        {
            setCursor(Qt::SizeHorCursor);
        }

        if (right_mid_rect.contains(eve->pos()))
        {
            setCursor(Qt::SizeHorCursor);
        }

        if (left_bottom_rect.contains(eve->pos()))
        {
            setCursor(Qt::SizeBDiagCursor);
        }

        if (mid_bottom_rect.contains(eve->pos()))
        {
            setCursor(Qt::SizeVerCursor);
        }

        if (right_bottom_rect.contains(eve->pos()))
        {
            setCursor(Qt::SizeFDiagCursor);
        }
    }

    if(event->type() == QEvent::MouseButtonPress)
    {
        QMouseEvent* eve = static_cast<QMouseEvent*>(event);
        
        QRect left_top_rect(0, 0, SIZE, SIZE);
        QRect mid_top_rect(SIZE, 0, width() - 2 * SIZE, SIZE);
        QRect right_top_rect(width() - SIZE, 0, SIZE, SIZE);

        QRect left_mid_rect(0, SIZE, SIZE, height() - 2 * SIZE);
        QRect right_mid_rect(width() - SIZE, SIZE, SIZE, height() - 2 * SIZE);

        QRect left_bottom_rect(0, height() - SIZE, SIZE, SIZE);
        QRect mid_bottom_rect(SIZE, height() - SIZE, width() - 2 * SIZE, SIZE);
        QRect right_bottom_rect(width() - SIZE, height() - SIZE, SIZE, SIZE);

        if (left_top_rect.contains(eve->pos()))
        {
            framework_area_ = Area::LEFT_TOP;
        }

        if (mid_top_rect.contains(eve->pos()))
        {
            framework_area_ = Area::MID_TOP;
        }

        if (right_top_rect.contains(eve->pos()))
        {
            framework_area_ = Area::RIGHT_TOP;
        }

        if (left_mid_rect.contains(eve->pos()))
        {
            framework_area_ = Area::LEFT_MID;
        }

        if (right_mid_rect.contains(eve->pos()))
        {
            framework_area_ = Area::RIGHT_MID;
        }

        if (left_bottom_rect.contains(eve->pos()))
        {
            framework_area_ = Area::LEFT_BOTTOM;
        }

        if (mid_bottom_rect.contains(eve->pos()))
        {
            framework_area_ = Area::MID_BOTTOM;
        }

        if (right_bottom_rect.contains(eve->pos()))
        {
            framework_area_ = Area::RIGHT_BOTTOM;
        }
    
        if (framework_area_ != Area::NONE)
        {
            framework_pressed_ = true;
            framework_point_ = eve->globalPos();
        }
    }

    if (event->type() == QEvent::MouseMove)
    {
        QMouseEvent* eve = static_cast<QMouseEvent*>(event);

        if(framework_pressed_)
        {
            QPoint new_point = eve->globalPos();
            QPoint distance = new_point - framework_point_;
            framework_point_ = new_point;

            switch (framework_area_)
            {
            case Area::NONE:
                break;
            case Area::LEFT_TOP:
                setGeometry(x() + distance.x(), y() + distance.y(), width() - distance.x(), height() - distance.y());
                break;
            case Area::MID_TOP:
                setGeometry(x(), y() + distance.y(), width(), height() - distance.y());
                break;
            case Area::RIGHT_TOP:
                setGeometry(x(), y() + distance.y(), width() + distance.x(), height() - distance.y());
                break;
            case Area::LEFT_MID:
                setGeometry(x() + distance.x(), y(), width() - distance.x(), height());
                break;
            case Area::RIGHT_MID:
                setGeometry(x(), y(), width() + distance.x(), height());
                break;
            case Area::LEFT_BOTTOM:
                setGeometry(x() + distance.x(), y(), width() - distance.x(), height() + distance.y());
                break;
            case Area::MID_BOTTOM:
                setGeometry(x(), y(), width(), height() + distance.y());
                break;
            case Area::RIGHT_BOTTOM:
                setGeometry(x(), y(), width() + distance.x(), height() + distance.y());
                break;
            default:
                break;
            }
        }
    }

    if (event->type() == QEvent::MouseButtonRelease)
    {
        QMouseEvent* eve = static_cast<QMouseEvent*>(event);

        framework_pressed_ = false;
        framework_area_ = Area::NONE;
    }

    return QWidget::eventFilter(obj, event);
}

MainWindow::~MainWindow()
{
}
