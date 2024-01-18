#pragma once

#include <QStyle>
#include <QTimer>
#include <QKeyEvent>
#include <QScrollBar>
#include <QMouseEvent>
#include <QMainWindow>
#include "ui_cmdsession.h"
#include "select_server.h"

class CmdSession : public QMainWindow
{
    Q_OBJECT

public:
    CmdSession(session_ptr sess, QWidget *parent = nullptr);
    ~CmdSession();
    void dispaly_cmd_result(const QString& cmd_result);
protected:
    bool eventFilter(QObject* watched, QEvent* event);

private:
    void on_runbtn_clicked();
    void on_cmd_complete(const char* cmd_result, uint32_t length);
private slots:
    void on_cmd_complete_ui(QString cmd_result);
private:
    Ui::CmdSessionClass ui;
    session_ptr sess_;
};
