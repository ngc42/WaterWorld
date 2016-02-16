/* This File is part of WaterWorld. License is GNU GPL Version 3.
 * Please see https://github.com/ngc42/WaterWorld/blob/master/LICENSE for details.
 * WaterWorld is (C) 2016 by Eike Lange (eike@ngc42.de)
 */


#include "graphicspathitem.h"


GraphicsPathItem::GraphicsPathItem(QGraphicsItem *inParent)
    : QGraphicsPathItem(inParent), m_isIslePath(true)
{
}


void GraphicsPathItem::setIslePath(const QPointF inStartPoint, const QPointF inEndPoint)
{
    m_isIslePath = true;
    QPainterPath p;
    p.moveTo(inStartPoint);
    p.lineTo(inEndPoint);
    setPath(p);
}


void GraphicsPathItem::paint(QPainter *inPainter, const QStyleOptionGraphicsItem *, QWidget *)
{
    QPen pen;
    if(m_isIslePath)
    {
        pen.setStyle(Qt::DashLine);
        pen.setColor(Qt::green);
        inPainter->setPen(pen);
    }
    inPainter->drawPath(path());
}
