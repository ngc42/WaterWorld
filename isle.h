/* This File is part of WaterWorld. License is GNU GPL Version 3.
 * Please see https://github.com/ngc42/WaterWorld/blob/master/LICENSE for details.
 * WaterWorld is (C) 2016 by Eike Lange (eike@ngc42.de)
 */


#ifndef ISLE_H
#define ISLE_H


#include <waterobject.h>
#include <ship.h>
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
    ShipTypeEnum shipToBuild;
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
        outInfo.shipToBuild = m_shipToBuild;
        outInfo.defaultTargetType = m_defaultTargetType;
        outInfo.defaultTargetIsle = m_defaultTargetIsle;
        outInfo.defaultTargetPos = m_defaultTargetPos;
        return outInfo;
    }

    // setter
    void setOwner(const uint inOwner, const QColor inColor);

    void setPopulation(const float inPopulation);

    void setShipToBuild(const ShipTypeEnum inShipToBuild);

    // set technology, if inTechnology is higher than isle's tech
    void setMaxTechnology(const float inTechnology);

    void setDefaultTargetIsle(const QPointF inTargetPos, const uint inIsleId);
    void setDefaultTargetWater(const QPointF inTargetPos);
    void setDefaultTargetNothing();


    // test
    bool pointInIsle(const QPointF inPos);

    // nextround, returns true if we finished a new ship
    bool nextRound();

    // all about fighting
    float force() const;
    void takeDamage(const float inOpponentForce);

private:
    QGraphicsEllipseItem *m_shape;      // display of the isle
    float m_population;                 // number of people on island
    float m_buildlevel;     // percentage of building a new ship. 1 means, release a new ship during nextRound()
    ShipTypeEnum m_shipToBuild; // we build this type of ship (user selects)

    // default target for newly created ships
    IsleInfo::TargetEnum m_defaultTargetType;
    uint m_defaultTargetIsle;   //
    QPointF m_defaultTargetPos;
};

#endif // ISLE_H
