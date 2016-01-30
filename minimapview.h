#ifndef MINIMAPVIEW_H
#define MINIMAPVIEW_H

#include <QGraphicsView>

class MinimapView : public QGraphicsView
{
    Q_OBJECT
public:
    explicit MinimapView(QWidget *inParent = 0);

protected:
    void mousePressEvent(QMouseEvent *inMouseEvent);

signals:
    // send Click event over to someone else (universe might be interested, because of repositioning)
    void sigMinimapClicked(QPointF screenPos);
};

#endif // MINIMAPVIEW_H
