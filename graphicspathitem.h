/* This File is part of WaterWorld. License is GNU GPL Version 3.
 * Please see https://github.com/ngc42/WaterWorld/blob/master/LICENSE for details.
 * WaterWorld is (C) 2016 by Eike Lange (eike@ngc42.de)
 */

#ifndef GRAPHICSPATHITEM_H
#define GRAPHICSPATHITEM_H


#include <QGraphicsPathItem>
#include <QPainter>


class GraphicsPathItem : public QGraphicsPathItem
{
public:
    GraphicsPathItem(QGraphicsItem *inParent = 0);

    void setIslePath(const QPointF inStartPoint, const QPointF inEndPoint);

    void paint(QPainter *inPainter, const QStyleOptionGraphicsItem *inOption, QWidget *inWidget);


private:
    bool m_isIslePath;

    QPointF directionVector(const QPointF inVector) const;

};


#endif // GRAPHICSPATHITEM_H
