/* This File is part of WaterWorld. License is GNU GPL Version 3.
 * Please see https://github.com/ngc42/WaterWorld/blob/master/LICENSE for details.
 * WaterWorld is (C) 2016 by Eike Lange (eike@ngc42.de)
 */


#include <math.h>
#include "graphicspathitem.h"
#include <QDebug>


GraphicsPathItem::GraphicsPathItem(QGraphicsItem *inParent)
    : QGraphicsPathItem(inParent), m_thisItemsType(PathType::P_ISLE)
{
}


void GraphicsPathItem::setIslePath(const QPointF inStartPoint, const QPointF inEndPoint)
{
    m_thisItemsType = PathType::P_ISLE;
    QPainterPath p(inStartPoint);
    // midpoint
    QPointF q = (inStartPoint + inEndPoint) / 2;
    p.lineTo(q);
    // direction vector
    QPointF q_dir = directionVector(inEndPoint - inStartPoint);
    // orthogonal to direction vector
    QPointF q_ortho = QPointF(-q_dir.y(), q_dir.x());
    // polygon like an arrow
    QPolygonF poly;
    poly << q << q + 10 * q_ortho <<  q + 10 * q_dir << q - 10 * q_ortho << q;
    p.addPolygon(poly);
    p.lineTo(inEndPoint);
    setPath(p);
}


void GraphicsPathItem::setShipVisitedPath(const QVector<QPointF> inVisitedPath)
{
    m_thisItemsType = PathType::P_VISITED;
    QPainterPath p(inVisitedPath.at(0));

    for(int i = 1; i < inVisitedPath.count(); i++)
    {
        p.lineTo(inVisitedPath.at(i));
    }
    setPath(p);
}


void GraphicsPathItem::setShipUnvisitedPath(const QPointF inStartPoint,
                          const QVector<QPointF> inUnvisitedPath, const bool inRepeatPath)
{
    m_thisItemsType = PathType::P_UNVISITED;
    QPainterPath p;

    p.moveTo(inUnvisitedPath.at(0));

    for(int i = 1; i < inUnvisitedPath.count(); i++)
    {
        p.lineTo(inUnvisitedPath.at(i));
    }
    if(inRepeatPath)
        p.lineTo(inStartPoint);
    setPath(p);
}


void GraphicsPathItem::paint(QPainter *inPainter, const QStyleOptionGraphicsItem *, QWidget *)
{
    QPen pen;
    switch(m_thisItemsType)
    {
        case PathType::P_ISLE:
            pen.setStyle(Qt::DashLine);
            pen.setColor(Qt::green);
            break;
        case PathType::P_UNVISITED:
            pen.setStyle(Qt::SolidLine);
            pen.setColor(Qt::green);
            break;
        case PathType::P_VISITED:
            pen.setStyle(Qt::DotLine);
            pen.setColor(Qt::lightGray);
            break;
    }

    inPainter->setPen(pen);
    inPainter->drawPath(path());
}


QPointF GraphicsPathItem::directionVector(const QPointF inVector) const
{
    qreal x = inVector.x();
    qreal y = inVector.y();
    qreal len = std::sqrt( x * x + y * y);
    return inVector / len;
}
