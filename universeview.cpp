#include "universeview.h"
#include <QMouseEvent>
#include <qdebug.h>


UniverseView::UniverseView(QWidget *inParent) :
    QGraphicsView(inParent), m_shipWantsTarget(false)
{
    m_rubberBandLine = new QGraphicsLineItem(10, 10, 10, 10);
}


void UniverseView::toggleShipWantsTarget(const QPointF inShipSourcePos, const uint inShipId)
{
    if(m_shipWantsTarget)
    {
        m_shipWantsTarget = false;
        m_rubberBandLine->hide();
    }
    else
    {
        m_shipWantsTarget = true;
        m_shipSourcePos = inShipSourcePos;
        m_shipSourceId = inShipId;
        m_rubberBandLine->show();
    }
    setMouseTracking(m_shipWantsTarget);
}


void UniverseView::setScene(QGraphicsScene *inScene)
{
    QGraphicsView::setScene(inScene);
    scene()->addItem(m_rubberBandLine);
}


void UniverseView::mousePressEvent(QMouseEvent *inMouseEvent)
{
    QPointF clickPoint = mapToScene(inMouseEvent->pos());
    qDebug()  << "UniverseView sends Click: " << clickPoint;

    if(m_shipWantsTarget)
    {
        toggleShipWantsTarget({0, 0}, 0);
        //qInfo() << "UniverView pos=" << clickPoint;
        emit sigUniverseViewClickedFinishTarget(clickPoint, m_shipSourceId);
    }
    else
        emit sigUniverseViewClicked( mapToScene(inMouseEvent->pos()) );
}


void UniverseView::mouseMoveEvent(QMouseEvent *inMouseEvent)
{
    QPointF targetPos = mapToScene(inMouseEvent->pos());

    m_rubberBandLine->setLine(m_shipSourcePos.x(), m_shipSourcePos.y(),
                              targetPos.x(), targetPos.y());
}


void UniverseView::slotMinimapClicked( QPointF scenePos )
{
    qDebug() << "Universe received MinimapClick: " << scenePos;
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
