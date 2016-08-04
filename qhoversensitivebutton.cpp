#include "qhoversensitivebutton.h"

QHoverSensitiveButton::QHoverSensitiveButton(QWidget *parent) : QToolButton(parent)
{
    setMouseTracking(true);
    setAttribute(Qt::WA_Hover);
    setCursor(Qt::ArrowCursor);
    resetView();
}

void QHoverSensitiveButton::hoverEnter(QHoverEvent *)
{

    repaint();
}

void QHoverSensitiveButton::hoverLeave(QHoverEvent *)
{
    resetView();
}

void QHoverSensitiveButton::hoverMove(QHoverEvent *)
{
    highLight();

}
void QHoverSensitiveButton::highLight()
{
    this->setStyleSheet("color:rgb(255, 255, 255);border: 2px solid white;font: 75 12pt \"MS Shell Dlg 2\";");
    repaint();
}
void QHoverSensitiveButton::resetView()
{
    this->setStyleSheet("color:rgb(0, 128, 0);\nfont: bold 15px \"MS Shell Dlg 2\";\n border: 1px solid green;\n");
    repaint();

}
bool QHoverSensitiveButton::event(QEvent *event)
{
    switch(event->type())
    {
    case QEvent::HoverEnter:
        hoverEnter(static_cast<QHoverEvent*>(event));
        return true;
        break;
    case QEvent::HoverLeave:
        hoverLeave(static_cast<QHoverEvent*>(event));
        return true;
        break;
    case QEvent::HoverMove:
        hoverMove(static_cast<QHoverEvent*>(event));
        return true;
        break;
    default:
        break;
    }
    return QWidget::event(event);
}
