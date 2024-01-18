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
		//��Ӱ				   û�б߿�                   С����      ���ڶ���
		qWidget.setWindowFlags(Qt::FramelessWindowHint | Qt::Tool /*| Qt::WindowStaysOnTopHint*/);
	}
	static void SetTransparent(QWidget& qWidget)
	{
		//����Ϊ͸��
		qWidget.setAttribute(Qt::WA_TranslucentBackground, true);
	}
	static void SetShadow(QWidget& qWidget)
	{
		//������Ӱ
		QGraphicsDropShadowEffect* shadow_effect = new QGraphicsDropShadowEffect(&qWidget);
		shadow_effect->setOffset(0, 0);
		shadow_effect->setColor(QColor(0xf4,0xf4,0xf4));
		shadow_effect->setBlurRadius(10);
		qWidget.setGraphicsEffect(shadow_effect);
	}
	static void SetShadow(QWidget& qWidget, const QColor& color, int iBlurRadius)
	{
		//������Ӱ
		QGraphicsDropShadowEffect* shadow_effect = new QGraphicsDropShadowEffect(&qWidget);
		shadow_effect->setOffset(0, 0);
		shadow_effect->setColor(color);
		shadow_effect->setBlurRadius(iBlurRadius);
		qWidget.setGraphicsEffect(shadow_effect);
	}
};

