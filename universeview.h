/* This File is part of WaterWorld. License is GNU GPL Version 3.
 * Please see https://github.com/ngc42/WaterWorld/blob/master/LICENSE for details.
 * WaterWorld is (C) 2016 by Eike Lange (eike@ngc42.de)
 */


#ifndef UNIVERSEVIEW_H
#define UNIVERSEVIEW_H


#include <QGraphicsView>
#include <QGraphicsLineItem>
#include <QGraphicsSimpleTextItem>


class UniverseView : public QGraphicsView
{
    Q_OBJECT
public:
    explicit UniverseView(QWidget *inParent = 0);

    // ship wants a new target. And exactly this here is the message we receive on it
    void toggleShipWantsTarget(const QPointF inShipSourcePos, const uint inShipId, const float inShipVelocity);
    void toggleIsleWantsTarget(const QPointF inIsleSourcePos, const uint inIsleId);

    void setScene(QGraphicsScene *inScene);

private:
    bool m_shipWantsTarget;     // true, if we are in search for a ship's target
    QPointF m_shipSourcePos;    // we save it here for the rubber band
    uint m_shipSourceId;        // we need ship id again, when we are finished
    float m_shipVelocity;       // for calculation of the length of the journey. Display a hint at rubber band.
    QGraphicsLineItem *m_rubberBandLine;  // we show, if ship wants a new target
    QGraphicsSimpleTextItem *m_journeyLengthDisplay;

    // isle wants default target
    bool m_isleWantsDefaultTarget;  // true, if we are in search for a default isle target
    QPointF m_isleSourcePos;        // pos of isle
    uint m_isleSourceId;            // id of isle

    void mousePressEvent(QMouseEvent *inMouseEvent);
    void mouseMoveEvent(QMouseEvent *inMouseEvent);

signals:
    void sigUniverseViewClicked(QPointF scenePos);
    void sigUniverseViewClickedFinishTarget(QPointF scenePos, uint shipId);
    void sigUniverseViewClickedFinishIsleTarget(QPointF scenePos, uint isleId);

public slots:
    void slotMinimapClicked(QPointF scenePos);
    void slotZoomIn();
    void slotZoomOut();
    void slotZoomNorm();
};

#endif // UNIVERSEVIEW_H
