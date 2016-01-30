#ifndef SHIP_H
#define SHIP_H

#include <waterobject.h>
#include <universescene.h>
#include <QGraphicsRectItem>
#include <QPointF>
#include <QColor>


struct Target
{
    enum TargetEnum {T_SHIP, T_ISLE, T_WATER};
    bool validTarget;
    TargetEnum tType;
    unsigned int id;  // ship or isle id
    // current pos of object. if this means a ship,
    // the pos should get updated in Manager::nextRound
    QPointF pos;
};

enum ShipPositionEnum {S_ONISLE, S_PATROL, S_OCEAN, S_TRASH};

/* Things we get to know, if we click on a ship */
struct ShipInfo
{
    uint id;
    uint owner;
    QColor color;
    QPointF pos;
    ShipPositionEnum posType;
    uint isleId;        // if not on ocean
    bool hasTarget;
    float damage;
    float technology;
};



class Ship : public WaterObject
{
public:

    Ship(UniverseScene *& inOutRefScene, const uint inId, const uint inOwner,
         const QPointF inPos, const QColor inColor, const ShipPositionEnum inPosType,
         const uint inIsleId, const float inTechnology);


    ~Ship();

    // getter
    QGraphicsRectItem* shape() const { return m_shape; }

    ShipInfo info() const
    {
        ShipInfo outInfo;
        outInfo.id = m_id;
        outInfo.owner = m_owner;
        outInfo.color = m_color;
        outInfo.pos = m_pos;
        outInfo.posType = m_positionType;
        outInfo.isleId = m_onIsleById;
        outInfo.hasTarget = m_target.validTarget;
        outInfo.damage = m_damage;
        outInfo.technology = m_technology;
        return outInfo;
    }

    ShipPositionEnum positionType() const { return m_positionType; }

    Target target() const { return m_target; }

    // setter

    /**
      * @brief Sets the new owner of the ship
      *
      */
    void setOwner(const uint inOwner, const QColor inColor);

    void setPositionType(ShipPositionEnum inType);

    void setTargetIsle(const uint inTargetIsleId, const QPointF inPos);

    void setTargetShip(const uint inTargetShipId, const QPointF inPos);

    void setTargetWater(const QPointF inPos);

    void setTargetFinished();

    void landOnIsle(const uint inIsleId, const QPointF inPos);

    void addDamage(const float inDamageToAdd);

    /* if a ship has target T_SHIP, then we need to
    update the target pos every round */
    void updateTargetPos(const QPointF inPos);

    bool nextRound();

    // tester
    bool pointInShip(const QPointF inPos);


    /* *********************************** */
    /* Data begins here                    */
    /* *********************************** */


private:
    QGraphicsRectItem *m_shape;
    ShipPositionEnum m_positionType;
    uint m_onIsleById;
    float m_damage;         // sailing arround, patroling, and fighting increases damage. Repair on isle,
    float m_halfWidth;
    Target m_target;
};

#endif // SHIP_H
