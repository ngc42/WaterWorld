/* This File is part of WaterWorld. License is GNU GPL Version 3.
 * Please see https://github.com/ngc42/WaterWorld/blob/master/LICENSE for details.
 * WaterWorld is (C) 2016 by Eike Lange (eike@ngc42.de)
 */


#ifndef ISLE_H
#define ISLE_H


#include <waterobject.h>
#include <QGraphicsEllipseItem>
#include <QPointF>
#include <QColor>


struct IsleInfo
{
    uint id;
    uint owner;
    QColor color;
    QPointF pos;
    float population;
    float technology;
};


class Isle : public WaterObject
{
public:
    Isle(const uint inId, const uint inOwner, const QPointF inPos,
         const QColor inColor);

    QGraphicsEllipseItem* shape() const { return m_shape; }

    IsleInfo info() const {
        IsleInfo outInfo;
        outInfo.id = m_id;
        outInfo.owner = m_owner;
        outInfo.color = m_color;
        outInfo.pos = m_pos;
        outInfo.population = m_population;
        outInfo.technology = m_technology;
        return outInfo;
    }

    // setter
    void setOwner(const uint inOwner, const QColor inColor);

    void setPopulation(const float inPopulation);

    // test
    bool pointInIsle(const QPointF inPos);

    // nextround
    void nextRound();

private:
    QGraphicsEllipseItem *m_shape;  // display of the isle
    float m_population;               // number of people on island
};

#endif // ISLE_H