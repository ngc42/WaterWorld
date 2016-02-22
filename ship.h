/* This File is part of WaterWorld. License is GNU GPL Version 3.
 * Please see https://github.com/ngc42/WaterWorld/blob/master/LICENSE for details.
 * WaterWorld is (C) 2016 by Eike Lange (eike@ngc42.de)
 */


#ifndef SHIP_H
#define SHIP_H


#include <waterobject.h>
#include <universescene.h>
#include <QGraphicsRectItem>
#include <QPointF>
#include <QColor>
#include <math.h>

struct Target
{
    // targets can be ships, isles and just a place on the water.
    enum TargetEnum {T_SHIP, T_ISLE, T_WATER};
    TargetEnum tType;
    unsigned int id;  // ship or isle id
    // current pos of object. if this means a ship,
    // the pos should get updated in Manager::nextRound
    QPointF pos;
    bool visited;
};


enum ShipPositionEnum {S_ONISLE, S_PATROL, S_OCEAN, S_TRASH};


enum ShipTypeEnum {ST_BATTLESHIP, ST_COURIER, ST_COLONY, ST_FLEET};


/* Things we get to know, if we click on a ship */
struct ShipInfo
{
    uint id;
    ShipTypeEnum shipType;
    uint owner;
    QColor color;
    QPointF pos;
    ShipPositionEnum posType;
    uint isleId;        // if not on ocean
    bool hasTarget;
    bool cycleTargetList;   // true, if repeat the list of targets over and over
    float damage;
    float technology;
    QPointF attachPos;  // pos of last target (or this pos) to attach rubber band line for next target
};


class Ship : public WaterObject
{
public:
    Ship(UniverseScene *& inOutRefScene, const uint inId, const uint inOwner,
         const QPointF inPos, const QColor inColor, const ShipPositionEnum inPosType,
         const uint inIsleId, const float inTechnology);

    Ship(UniverseScene *& inOutRefScene, const ShipTypeEnum inShipType, const uint inId, const uint inOwner,
         const QPointF inPos, const QColor inColor, const ShipPositionEnum inPosType,
         const uint inIsleId, const float inTechnology);


    ~Ship();

    // getter
    QGraphicsRectItem* shape() const { return m_shape; }

    ShipInfo info() const;

    ShipPositionEnum positionType() const { return m_positionType; }

    // setter

    /**
      * @brief Sets the new owner of the ship
      *
      */
    void setOwner(const uint inOwner, const QColor inColor);

    void setPositionType(ShipPositionEnum inType);


    // -- Targets --

    void setCycleTargets(const bool inCycleTarget);

    // check, if ship REALLY has a valid target, else this breaks
    Target currentTarget() const { return m_targetList.at(m_currentTargetIndex); }

    QVector<Target> targets() const { return m_targetList; }

    void setTargetIsle(const uint inTargetIsleId, const QPointF inPos);

    void setTargetShip(const uint inTargetShipId, const QPointF inPos);

    void removeTargets();

    void removeTargetShip(const uint inShipId);

    void removeTargetByIndex(const int inIndex);

    void setTargetWater(const QPointF inPos);

    void setTargetFinished();

    /* if a ship has target T_SHIP, then we need to
    update the target pos every round */
    void updateTargetPos(const uint inShipId, const QPointF inPos);

    void landOnIsle(const uint inIsleId, const QPointF inPos);

    void addDamage(const float inDamageToAdd);

    bool nextRound();

    // tester
    bool pointInShip(const QPointF inPos);

private:
    ShipTypeEnum m_shipType;
    QGraphicsRectItem *m_shape;
    ShipPositionEnum m_positionType;
    uint m_onIsleById;
    float m_damage;         // sailing arround, patroling, and fighting increases damage. Repair on isle,
    float m_halfWidth;

    // all about targets
    QVector<Target> m_targetList;
    bool m_cycleTargetList;
    int m_currentTargetIndex;

    // after inserting / removing targets, m_currentTargetIndex and m_cycleTargetList need to get fixed
    void fixTargetIndex();

    /* add the current pos as a visted target in
     * case there are no targets, this shows up a better path
     */
    void addCurrentPosToTarget();
};

#endif // SHIP_H
