/* This File is part of WaterWorld. License is GNU GPL Version 3.
 * Please see https://github.com/ngc42/WaterWorld/blob/master/LICENSE for details.
 * WaterWorld is (C) 2016 by Eike Lange (eike@ngc42.de)
 */

#include <math.h>
#include "graphicspathitem.h"
#include <QDebug>

GraphicsPathItem::GraphicsPathItem(QGraphicsItem *inParent)
    : QGraphicsPathItem(inParent), m_isIslePath(true)
{
}


void GraphicsPathItem::setIslePath(const QPointF inStartPoint, const QPointF inEndPoint)
{
    m_isIslePath = true;
    QPainterPath p;
    p.moveTo(inStartPoint);
    QPointF q = (inStartPoint + inEndPoint) / 2;
    p.lineTo(q);
    QPointF q_dir = directionVector(q);
    QPointF q_ortho = QPointF(-q_dir.y(), q_dir.x());
    QPolygonF poly;
    poly << q << q + 10 * q_ortho <<  q + 10 * q_dir << q - 10 * q_ortho << q;
    p.addPolygon(poly);
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


QPointF GraphicsPathItem::directionVector(const QPointF inVector) const
{
    qreal x = inVector.x();
    qreal y = inVector.y();
    qreal len = std::sqrt( x * x + y * y);
    return inVector / len;
}
