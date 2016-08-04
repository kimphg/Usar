#ifndef QHOVERSENSITIVEBUTTON_H
#define QHOVERSENSITIVEBUTTON_H

#include <QWidget>
#include <QToolButton>
#include <qevent.h>
class QHoverSensitiveButton : public QToolButton
{
    Q_OBJECT
public:
    explicit QHoverSensitiveButton(QWidget *parent = 0);

protected:
    void highLight();
    void resetView();
    void hoverEnter(QHoverEvent *event);
    void hoverLeave(QHoverEvent *event);
    void hoverMove(QHoverEvent *event);
    bool event(QEvent *event);

signals:

public slots:
};

#endif // QHOVERSENSITIVEBUTTON_H
