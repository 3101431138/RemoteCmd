#pragma once

#include "select_server.h"

#include <map>
#include <QStyle>
#include <QTimer>
#include <QTabWidget>
#include <QKeyEvent>
#include <QScrollBar>
#include <QMouseEvent>
#include <QtWidgets/QMainWindow>
#include "ui_mainwindow.h"
#include "shell.h"
#include "cmdsession.h"


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
private slots:
    void on_closebtn_clicked();
    void on_tabWidget_tabCloseRequested(int index);
private:
    void on_user_connect(session_ptr sess);
    void on_user_disconnect(session_ptr sess);
    void on_user_read_packet(session_ptr sess, packet_ptr pkt);

    void on_user_connect_ui(session_ptr sess);
    void on_user_disconnect_ui(session_ptr sess);
    void on_user_read_packet_ui(session_ptr sess, packet_ptr pkt);
private:
    Ui::MainWindowClass ui;

    QPoint ctrl_window_pos_pt_;
    bool   ctrl_window_pos_sta_;

    Shell  shell_;

    Server serv_;
    
    std::map<std::string, std::shared_ptr<CmdSession>> cmd_session_map_;
};
