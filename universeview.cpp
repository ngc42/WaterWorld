/* This File is part of WaterWorld. License is GNU GPL Version 3.
 * Please see https://github.com/ngc42/WaterWorld/blob/master/LICENSE for details.
 * WaterWorld is (C) 2016 by Eike Lange (eike@ngc42.de)
 */


#include "universeview.h"
#include <math.h>
#include <QMouseEvent>
#include <QDebug>


UniverseView::UniverseView(QWidget *inParent) :
    QGraphicsView(inParent), m_shipWantsTarget(false), m_shipVelocity(1.0f)
{
    m_rubberBandLine = new QGraphicsLineItem(10, 10, 10, 10);
    m_journeyLengthDisplay = new QGraphicsSimpleTextItem();
}


void UniverseView::toggleShipWantsTarget(const QPointF inShipSourcePos, const uint inShipId, const float inShipVelocity)
{
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


void UniverseView::setScene(QGraphicsScene *inScene)
{
    QGraphicsView::setScene(inScene);
    scene()->addItem(m_rubberBandLine);
    scene()->addItem(m_journeyLengthDisplay);
}


void UniverseView::mousePressEvent(QMouseEvent *inMouseEvent)
{
    QPointF clickPoint = mapToScene(inMouseEvent->pos());
    //qDebug()  << "UniverseView sends Click: " << clickPoint;

    if(m_shipWantsTarget)
    {
        toggleShipWantsTarget({0, 0}, 0, 1.0f);
        //qInfo() << "UniverView pos=" << clickPoint;
        emit sigUniverseViewClickedFinishTarget(clickPoint, m_shipSourceId);
    }
    else
        emit sigUniverseViewClicked( mapToScene(inMouseEvent->pos()) );
}


void UniverseView::mouseMoveEvent(QMouseEvent *inMouseEvent)
{
    if(!m_shipWantsTarget)
        return;
    QPointF targetPos = mapToScene(inMouseEvent->pos());

    // distance from source to target
    qreal dx = targetPos.x() - m_shipSourcePos.x();
    qreal dy = targetPos.y() - m_shipSourcePos.y();
    uint journeyDuration = (uint) (std::sqrt( (dx * dx) + (dy * dy) ) / m_shipVelocity);

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


void UniverseView::slotMinimapClicked( QPointF scenePos )
{
    //qDebug() << "Universe received MinimapClick: " << scenePos;
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
