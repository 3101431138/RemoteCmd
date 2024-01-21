#pragma once

#include <QWidget>
#include <QPushButton>
#include <QMainWindow>
#include <QMouseEvent>

class TitleController : public QObject
{
    Q_OBJECT
public:
    TitleController(QObject* parent = nullptr);
    void setObject(QMainWindow* window, QWidget* title, QPushButton* close);
    void mouseMoveEvent(QMouseEvent* event);
    void mousePressEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
private:
    void on_close();
private:
    QMainWindow* window_;
    QWidget*     title_;
    QPushButton* close_;
    QPoint       pos_pt_;
    bool         pos_sta_;
};

