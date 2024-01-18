#pragma once
#include <QDir>
#include <QWidget>
#include <Windows.h>
#include <QGraphicsDropShadowEffect>

class QTUtil
{
public:
	static void RemoveFrame(QWidget& qWidget)
	{
		//阴影				   没有边框                   小部件      至于顶层
		qWidget.setWindowFlags(Qt::FramelessWindowHint | Qt::Tool /*| Qt::WindowStaysOnTopHint*/);
	}
	static void SetTransparent(QWidget& qWidget)
	{
		//设置为透明
		qWidget.setAttribute(Qt::WA_TranslucentBackground, true);
	}
	static void SetShadow(QWidget& qWidget)
	{
		//设置阴影
		QGraphicsDropShadowEffect* shadow_effect = new QGraphicsDropShadowEffect(&qWidget);
		shadow_effect->setOffset(0, 0);
		shadow_effect->setColor(QColor(0xf4,0xf4,0xf4));
		shadow_effect->setBlurRadius(10);
		qWidget.setGraphicsEffect(shadow_effect);
	}
	static void SetShadow(QWidget& qWidget, const QColor& color, int iBlurRadius)
	{
		//设置阴影
		QGraphicsDropShadowEffect* shadow_effect = new QGraphicsDropShadowEffect(&qWidget);
		shadow_effect->setOffset(0, 0);
		shadow_effect->setColor(color);
		shadow_effect->setBlurRadius(iBlurRadius);
		qWidget.setGraphicsEffect(shadow_effect);
	}
};

