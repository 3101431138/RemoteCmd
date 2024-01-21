#pragma once

#include "select_server.h"

#include <map>
#include <QtWinExtras/QtWinExtras>
#include <QStyle>
#include <QTimer>
#include <QDebug>
#include <QTabWidget>
#include <QKeyEvent>
#include <QScrollBar>
#include <QMouseEvent>
#include <QPropertyAnimation>
#include <QtWidgets/QMainWindow>
#include "ui_mainwindow.h"
#include "shell.h"
#include "cmdsession.h"
#include "titlecontroller.h"

enum class Area
{
    NONE,
    LEFT_TOP,
    MID_TOP,
    RIGHT_TOP,
    LEFT_MID,
    RIGHT_MID,
    LEFT_BOTTOM,
    MID_BOTTOM,
    RIGHT_BOTTOM,
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void mouseMoveEvent(QMouseEvent* event);
    void mousePressEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
    bool eventFilter(QObject* obj, QEvent* event);
private slots:
    void on_tabWidget_tabCloseRequested(int index);
    void on_icon_btn_clicked();
private:
    void on_user_connect(session_ptr sess);
    void on_user_disconnect(session_ptr sess);
    void on_user_read_packet(session_ptr sess, packet_ptr pkt);

    void on_user_connect_ui(session_ptr sess);
    void on_user_disconnect_ui(session_ptr sess);
    void on_user_read_packet_ui(session_ptr sess, packet_ptr pkt);
private:
    Ui::MainWindowClass ui;

    Server serv_;

    TitleController title_ctrl_;

    bool   framework_pressed_;
    QPoint framework_point_;
    Area   framework_area_;
    
    std::map<std::string, std::shared_ptr<CmdSession>> cmd_session_map_;
};
