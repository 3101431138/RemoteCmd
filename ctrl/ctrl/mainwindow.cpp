#include "mainwindow.h"
#include "qtutil.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ctrl_window_pos_pt_(0, 0)
    , ctrl_window_pos_sta_(false)
{
    ui.setupUi(this);

    QTUtil::RemoveFrame(*this);
    QTUtil::SetTransparent(*this);
    QTUtil::SetShadow(*ui.wgtBack);
    QTUtil::SetShadow(*ui.lbTitle, QColor(0xf4, 0xf4, 0xf4), 5);

    connect(ui.close_btn, &QPushButton::clicked, this, &MainWindow::on_closebtn_clicked);

    //cmd_session_map_[std::string("localhost")] = new CmdSession();
    //ui.tabWidget->addTab(cmd_session_map_[std::string("localhost")], "localhost");

    ui.tabWidget->setTabsClosable(true);
    connect(ui.tabWidget, &QTabWidget::tabCloseRequested, this, &MainWindow::on_tabWidget_tabCloseRequested);

    serv_.set_connect_cb(std::bind(&MainWindow::on_user_connect, this, std::placeholders::_1));
    serv_.set_disconnect_cb(std::bind(&MainWindow::on_user_disconnect, this, std::placeholders::_1));
    serv_.set_read_packet_cb(std::bind(&MainWindow::on_user_read_packet, this, std::placeholders::_1, std::placeholders::_2));
    serv_.run();

    QMetaObject::invokeMethod(this, [](){
        Sleep(2000);
        OutputDebugStringA("AAA");
    }, Qt::AutoConnection);

    QMetaObject::invokeMethod(this, []()
    {
        Sleep(1000);
        OutputDebugStringA("BBB");
    }, Qt::AutoConnection);

    //QTimer* timer = new QTimer;
    //QObject::connect(timer, &QTimer::timeout, [timer, this]()
    //{
    //    ui.input_cmd_edt->setFocus();
    //    delete timer;
    //});
    //timer->start(1000);

    //shell_.set_cmd_complete_cb(std::bind(&MainWindow::on_cmd_complete, this, std::placeholders::_1, std::placeholders::_2));
    //shell_.invoke();
}

void MainWindow::on_closebtn_clicked()
{
    QApplication::exit(0);
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
    if (!ui.wgtTitle->rect().contains(event->pos()))
    {
        return;
    }
    if ((event->button() == Qt::LeftButton))
    {
        ctrl_window_pos_sta_ = true;
        ctrl_window_pos_pt_ = event->globalPos() - this->pos();
    }
}
void MainWindow::mouseMoveEvent(QMouseEvent* event)
{
    if (ctrl_window_pos_sta_)
    {
        move(event->globalPos() - ctrl_window_pos_pt_);
    }
}
void MainWindow::mouseReleaseEvent(QMouseEvent* event)
{
    ctrl_window_pos_sta_ = false;
}

MainWindow::~MainWindow()
{
}
