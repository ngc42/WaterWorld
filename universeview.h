/* This File is part of WaterWorld. License is GNU GPL Version 3.
 * Please see https://github.com/ngc42/WaterWorld/blob/master/LICENSE for details.
 * WaterWorld is (C) 2016 by Eike Lange (eike@ngc42.de)
 */


#ifndef UNIVERSEVIEW_H
#define UNIVERSEVIEW_H


#include <QGraphicsView>
#include <QGraphicsLineItem>


class UniverseView : public QGraphicsView
{
    Q_OBJECT
public:
    explicit UniverseView(QWidget *inParent = 0);

    // ship wants a new target. And exactly this here is the message we receive on it
    void toggleShipWantsTarget(const QPointF inShipSourcePos, const uint inShipId);

    void setScene(QGraphicsScene *inScene);

private:
    bool m_shipWantsTarget;     // true, if we are in search for a ship's target
    QPointF m_shipSourcePos;    // we save it here for the rubber band
    uint m_shipSourceId;        // we need ship id again, when we are finished
    QGraphicsLineItem *m_rubberBandLine;  // we show, if ship wants a new target

    void mousePressEvent(QMouseEvent *inMouseEvent);
    void mouseMoveEvent(QMouseEvent *inMouseEvent);

signals:
    void sigUniverseViewClicked(QPointF scenePos);
    void sigUniverseViewClickedFinishTarget(QPointF scenePos, uint shipId);

public slots:
    void slotMinimapClicked(QPointF scenePos);
    void slotZoomIn();
    void slotZoomOut();
    void slotZoomNorm();
};

#endif // UNIVERSEVIEW_H
