/* This File is part of WaterWorld. License is GNU GPL Version 3.
 * Please see https://github.com/ngc42/WaterWorld/blob/master/LICENSE for details.
 * WaterWorld is (C) 2016 by Eike Lange (eike@ngc42.de)
 */


#ifndef GRAPHICSPATHITEM_H
#define GRAPHICSPATHITEM_H


#include <QGraphicsPathItem>
#include <QPainter>
#include <QPainterPath>


/* Description:
 * UniverseView has 3 GraphicsPathItems:
 * - One to show the default target for an isle
 * - unvisited targets for a ship (and an extra point if the targets will be repeated)
 * - already visited targets for ship
 *
 * Each of them has an own setter in this class to provide the data. This data is then
 * drawn using the paint method.
*/
class GraphicsPathItem : public QGraphicsPathItem
{
public:

    GraphicsPathItem(QGraphicsItem *inParent = 0);

    void setIslePath(const QPointF inStartPoint, const QPointF inEndPoint);

    void setShipVisitedPath(const QVector<QPointF> inVisitedPath);

    // inStartPoint only active, if inRepeatPath is true
    void setShipUnvisitedPath(const QPointF inStartPoint,
                              const QVector<QPointF> inUnvisitedPath, const bool inRepeatPath);

    void paint(QPainter *inPainter, const QStyleOptionGraphicsItem *inOption, QWidget *inWidget);


private:

    enum PathType {P_ISLE, P_VISITED, P_UNVISITED};
    PathType m_thisItemsType;

    bool m_isIslePath;

    QPointF directionVector(const QPointF inVector) const;
};

#endif // GRAPHICSPATHITEM_H
