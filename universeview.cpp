/* This File is part of WaterWorld. License is GNU GPL Version 3.
 * Please see https://github.com/ngc42/WaterWorld/blob/master/LICENSE for details.
 * WaterWorld is (C) 2016 by Eike Lange (eike@ngc42.de)
 */


#include "universeview.h"
#include <math.h>
#include <QMouseEvent>
#include <QDebug>
#include <QPainterPath>

UniverseView::UniverseView(QWidget *inParent) :
    QGraphicsView(inParent), m_shipWantsTarget(false), m_shipVelocity(1.0f), m_isleWantsDefaultTarget(false)
{
    m_rubberBandLine = new QGraphicsLineItem(10, 10, 10, 10);
    m_journeyLengthDisplay = new QGraphicsSimpleTextItem();
    m_islePathItem = new GraphicsPathItem();
    m_islePathItem->hide();
    m_ShipVisitedPathItem = new GraphicsPathItem();
    m_ShipVisitedPathItem->hide();
    m_ShipUnvisitedPathItem = new GraphicsPathItem();
    m_ShipUnvisitedPathItem->hide();

}


void UniverseView::toggleShipWantsTarget(const QPointF inShipSourcePos, const uint inShipId, const float inShipVelocity)
{
    if(m_isleWantsDefaultTarget)
        return;             // we don't want to get confused
    if(m_shipWantsTarget)
    {
        m_shipWantsTarget = false;
        m_rubberBandLine->hide();
        m_journeyLengthDisplay->hide();
    }
    else
    {
        m_shipWantsTarget = true;
        m_shipSourcePos = inShipSourcePos;
        m_shipSourceId = inShipId;
        m_shipVelocity = inShipVelocity;
        m_rubberBandLine->show();
        m_journeyLengthDisplay->show();
    }
    setMouseTracking(m_shipWantsTarget);
}


void UniverseView::toggleIsleWantsTarget(const QPointF inIsleSourcePos, const uint inIsleId)
{
    if(m_shipWantsTarget)
        return;                     // we don't want to get confused
    if(m_isleWantsDefaultTarget)
    {
        m_isleWantsDefaultTarget = false;
        m_rubberBandLine->hide();
    }
    else
    {
        m_isleWantsDefaultTarget = true;
        m_isleSourcePos = inIsleSourcePos;
        m_isleSourceId = inIsleId;
        m_rubberBandLine->show();
    }
    setMouseTracking(m_isleWantsDefaultTarget);
}


void UniverseView::setScene(QGraphicsScene *inScene)
{
    QGraphicsView::setScene(inScene);
    scene()->addItem(m_rubberBandLine);
    scene()->addItem(m_journeyLengthDisplay);
    scene()->addItem(m_islePathItem);
    scene()->addItem(m_ShipVisitedPathItem);
    scene()->addItem(m_ShipUnvisitedPathItem);
}


void UniverseView::showIslePath(const QPointF inStartPoint, const QPointF inEndPoint)
{
    m_islePathItem->setIslePath(inStartPoint, inEndPoint);
    m_islePathItem->show();
}


void UniverseView::showShipPath(const QPointF inCurrentPos, const QVector<Target> inTargetList, const bool inRepeatPath)
{

    if(inTargetList.isEmpty())
        return;
    int numTargets = inTargetList.count();

    QVector<QPointF> visited;
    QVector<QPointF> unvisited;

    if(numTargets == 1)
    {

        unvisited.append(inTargetList.at(0).pos);
        m_ShipUnvisitedPathItem->setShipUnvisitedPath(inCurrentPos, unvisited, false);
        m_ShipUnvisitedPathItem->show();
        return;
    }

    bool lastInsertVisited = true;
    for(int i = 0; i < numTargets; i++)
    {
        Target t = inTargetList.at(i);
        if(t.visited)
        {
            visited.append(t.pos);
        }
        else
        {
            if(lastInsertVisited)
            {
                visited.append(inCurrentPos);
                lastInsertVisited = false;
            }
            unvisited.append(t.pos);
        }

    }
    if(visited.count() > 1)
    {
        m_ShipVisitedPathItem->setShipVisitedPath(visited);
        m_ShipVisitedPathItem->show();
    }
    if(unvisited.count() > 0)
    {
        m_ShipUnvisitedPathItem->setShipUnvisitedPath(inCurrentPos, unvisited, inRepeatPath);
        m_ShipUnvisitedPathItem->show();
    }
}


void UniverseView::hidePathItem()
{
    m_islePathItem->hide();
    m_ShipVisitedPathItem->hide();
    m_ShipUnvisitedPathItem->hide();
}


void UniverseView::mousePressEvent(QMouseEvent *inMouseEvent)
{
    QPointF clickPoint = mapToScene(inMouseEvent->pos());

    if(m_shipWantsTarget)
    {
        toggleShipWantsTarget({0, 0}, 0, 1.0f);
        emit sigUniverseViewClickedFinishShipTarget(clickPoint, m_shipSourceId);
    }
    else if(m_isleWantsDefaultTarget)
    {
        toggleIsleWantsTarget({0, 0}, 0);
        emit sigUniverseViewClickedFinishIsleTarget(clickPoint, m_isleSourceId);
    }
    else
        emit sigUniverseViewClicked( mapToScene(inMouseEvent->pos()) );
}


void UniverseView::mouseMoveEvent(QMouseEvent *inMouseEvent)
{
    if(! (m_shipWantsTarget or m_isleWantsDefaultTarget))
        return;
    QPointF targetPos = mapToScene(inMouseEvent->pos());

    if(m_shipWantsTarget)
    {
        // distance from source to target
        qreal dx = targetPos.x() - m_shipSourcePos.x();
        qreal dy = targetPos.y() - m_shipSourcePos.y();
        uint journeyDuration = (uint) (std::sqrt( (dx * dx) + (dy * dy) ) / m_shipVelocity) + 1;

        // write down
        QString jds = QString("%1 rounds").arg(journeyDuration);
        m_journeyLengthDisplay->setText(jds);

        // display text at 2/3 of the rubber band
        qreal px = m_shipSourcePos.x() + dx / 1.5f;
        qreal py = m_shipSourcePos.y() + dy / 1.5f;
        m_journeyLengthDisplay->setPos(px, py);

        m_rubberBandLine->setLine(m_shipSourcePos.x(), m_shipSourcePos.y(),
                                  targetPos.x(), targetPos.y());
    }
    else
    {
        m_rubberBandLine->setLine(m_isleSourcePos.x(), m_isleSourcePos.y(),
                                  targetPos.x(), targetPos.y());
    }
}


void UniverseView::slotMinimapClicked( QPointF scenePos )
{
    ensureVisible(scenePos.x() - 100, scenePos.y() - 100, 200, 200);
}


void UniverseView::slotZoomIn()
{
    scale(1.2f, 1.2f);
}


void UniverseView::slotZoomOut()
{
    scale(0.8f, 0.8f);
}


void UniverseView::slotZoomNorm()
{
    resetMatrix();
    qreal scaleX = size().width() / (scene()->sceneRect().width() + 10.0f);
    qreal scaleY = size().height() / (scene()->sceneRect().height() + 10.0f);
    qreal scaleFactor = scaleX > scaleY ? scaleY : scaleX;
    scale(scaleFactor, scaleFactor);
}
