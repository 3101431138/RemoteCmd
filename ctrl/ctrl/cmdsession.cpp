#include "cmdsession.h"

CmdSession::CmdSession(session_ptr sess, QWidget *parent)
    : QMainWindow(parent)
    , sess_(sess)
{
    ui.setupUi(this);

    connect(ui.run_btn, &QPushButton::clicked, this, &CmdSession::on_runbtn_clicked);

    ui.display_cmd_edt->setReadOnly(true);
    ui.display_cmd_edt->verticalScrollBar()->style()->polish(ui.display_cmd_edt->verticalScrollBar());
    ui.display_cmd_edt->horizontalScrollBar()->style()->polish(ui.display_cmd_edt->horizontalScrollBar());

    ui.input_cmd_edt->installEventFilter(this);
}

bool CmdSession::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == ui.input_cmd_edt && event->type() == QEvent::KeyPress)
    {
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
        if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter)
        {
            on_runbtn_clicked();
            return true;
        }
    }
    // 其他事件的默认处理
    return QObject::eventFilter(watched, event);
}

void CmdSession::on_runbtn_clicked()
{
    QString input_cmd = ui.input_cmd_edt->text();
    std::string cmd = input_cmd.toStdString() + "\r\n";

    packet_ptr pkt(new Packet);
    pkt->data = cmd;
    pkt->length = pkt->data.length();
    sess_->send(pkt);

    ui.input_cmd_edt->setText("");
}

void CmdSession::on_cmd_complete(const char* cmd_result, uint32_t length)
{
    QMetaObject::invokeMethod(this, "on_cmd_complete_ui", Qt::QueuedConnection, Q_ARG(QString, QString::fromLocal8Bit(cmd_result, length)));
}

void CmdSession::on_cmd_complete_ui(QString cmd_result)
{
    ui.display_cmd_edt->append(cmd_result);
}

CmdSession::~CmdSession()
{
    sess_->close();
}

void CmdSession::dispaly_cmd_result(const QString& cmd_result)
{
    ui.display_cmd_edt->append(cmd_result);
}
