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
    enum TargetEnum {T_NOTHING, T_WATER, T_ISLE};
    uint id;
    uint owner;
    QColor color;
    QPointF pos;
    float population;
    float technology;
    float buildlevel;
    TargetEnum defaultTargetType;   // default target
    uint defaultTargetIsle;
    QPointF defaultTargetPos;
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
        outInfo.buildlevel = m_buildlevel;
        outInfo.defaultTargetType = m_defaultTargetType;
        outInfo.defaultTargetIsle = m_defaultTargetIsle;
        outInfo.defaultTargetPos = m_defaultTargetPos;
        return outInfo;
    }

    // setter
    void setOwner(const uint inOwner, const QColor inColor);

    void setPopulation(const float inPopulation);

    // set technology, if inTechnology is higher than isle's tech
    void setMaxTechnology(const float inTechnology);

    void setDefaultTargetIsle(const QPointF inTargetPos, const uint inIsleId);
    void setDefaultTargetWater(const QPointF inTargetPos);
    void setDefaultTargetNothing();


    // test
    bool pointInIsle(const QPointF inPos);

    // nextround, returns true if we finished a new ship
    bool nextRound();

private:
    QGraphicsEllipseItem *m_shape;      // display of the isle
    float m_population;                 // number of people on island
    float m_buildlevel;     // percentage of building a new ship. 1 means, release a new ship during nextRound()

    // default target for newly created ships
    IsleInfo::TargetEnum m_defaultTargetType;
    uint m_defaultTargetIsle;   //
    QPointF m_defaultTargetPos;
};

#endif // ISLE_H
