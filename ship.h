/* This file is part of WaterWorld.
 * License GPL Version 3, see file "LICENSE" for details
 * Author Eike Lange
 */

#ifndef SHIP_H
#define SHIP_H

#include <cmath>
#include "waterobject.h"


struct Target
{
    enum TargetEnum {T_SHIP, T_ISLE, T_WATER};
    bool validTarget;
    TargetEnum tType;
    unsigned int id;  // ship or isle id
    // current pos of object. if this means a ship,
    // the pos should get updated in Manager::nextRound
    sf::Vector2f pos;
};


enum ShipPositionEnum {S_ONISLE, S_PATRUILLE, S_OCEAN, S_TRASH};

/* Things we get to know, if we click on a ship */
struct ShipInfo
{
    unsigned int id;
    unsigned int owner;
    sf::Color color;
    sf::Vector2f pos;
    ShipPositionEnum posType;
    unsigned int isleId;        // if not on ocean
    bool hasTarget;
    float damage;
};



struct Ship : WaterObject
{
public:
    Ship(const unsigned int inId, const unsigned int inOwner, const sf::Vector2f inPos,
         const sf::Color inColor, const ShipPositionEnum inPosType, unsigned int inIsleId)
        :  WaterObject(inId, inOwner, inPos, inColor),
          m_positionType(inPosType), m_onIsleById(inIsleId),
          m_damage(0.0f),
          m_halfWidth(10.0f)  // @fixme: hard coded

    {
        m_shape.setSize(sf::Vector2f(2.0f * m_halfWidth, 2.0f * m_halfWidth));
        m_shape.setPosition(inPos.x - m_halfWidth, inPos.y - m_halfWidth);
        m_shape.setFillColor(inColor);
        m_target.validTarget = false;
    }


    // getter
    sf::RectangleShape shape() const { return m_shape; }


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
        return outInfo;
    }

    ShipPositionEnum positionType() const { return m_positionType; }

    Target target() const { return m_target; }


    // setter

    /**
     * @brief Sets the new owner of the ship
     *
     */
    void setOwner(const unsigned int inOwner, const sf::Color inColor)
    {
        m_owner = inOwner;
        m_color = inColor;
        m_shape.setFillColor(inColor);
        m_target.validTarget = false;
        m_damage = 0.0f;
    }


    void setPositionType(ShipPositionEnum inType)
    {
        m_positionType = inType;
    }





    void setTargetIsle(const unsigned int inTargetIsleId, const sf::Vector2f inPos)
    {
        if((inTargetIsleId != m_onIsleById) or (m_positionType == ShipPositionEnum::S_OCEAN))
        {
            m_target.id = inTargetIsleId;
            m_target.pos = inPos;
            m_target.tType = Target::TargetEnum::T_ISLE;
            m_target.validTarget = true;
        }
    }


    void setTargetShip(const unsigned int inTargetShipId, const sf::Vector2f inPos)
    {
        if(inTargetShipId != m_id)
        {
            m_target.id = inTargetShipId;
            m_target.pos = inPos;
            m_target.tType = Target::TargetEnum::T_SHIP;
            m_target.validTarget = true;
        }
    }


    void setTargetWater(const sf::Vector2f inPos)
    {
        m_target.id = 0;
        m_target.pos = inPos;
        m_target.tType = Target::TargetEnum::T_WATER;
        m_target.validTarget = true;
    }


    void setTargetFinished()
    {
        m_target.id = 0;
        m_target.pos = {0.0f, 0.0f};
        m_target.tType = Target::TargetEnum::T_WATER;
        m_target.validTarget = false;
    }


    void landOnIsle(const unsigned int inIsleId, const sf::Vector2f inPos)
    {
        m_onIsleById = inIsleId;
        m_positionType = ShipPositionEnum::S_ONISLE;
        m_pos = inPos;
        setTargetFinished();
    }


    void addDamage(const float inDamageToAdd)
    {
        m_damage = m_damage + inDamageToAdd;
        if(m_damage >= 1.0 )
            setPositionType(ShipPositionEnum::S_TRASH);
    }


    bool nextRound()
    {
        if(m_positionType == ShipPositionEnum::S_TRASH)
        {
            return true;    // arrived in heaven;
        }
        if(m_target.validTarget)
        {
            // Rod_Steward::Sailing, YouTube::DyIw0gcgfik

            m_positionType = ShipPositionEnum::S_OCEAN;
            float dx = m_target.pos.x - m_pos.x;
            float dy = m_target.pos.y - m_pos.y;

            float d = sqrt( dx * dx + dy * dy );

            // @fixme: hardcoded velocity
            if(d <= m_halfWidth)
                return true;

            float ex = dx / d;
            float ey = dy / d;

            // @fixme: hardcoded velocity
            m_pos = sf::Vector2f{m_pos.x + 10.0f * ex, m_pos.y + 10.0f * ey};

            // @fixme hardcoded rect
            m_shape.setPosition(m_pos - sf::Vector2f(m_halfWidth, m_halfWidth));
        }

        return false;
    }


    // tester
    bool pointInShip(const int inX, const int inY)
    {
        sf::Vector2f myPos = pos();
        return inX > (myPos.x - m_halfWidth) and
               inX < (myPos.x + m_halfWidth) and
               inY > (myPos.y - m_halfWidth) and
               inY < (myPos.y + m_halfWidth);
    }


    /* *********************************** */
    /* Data begins here                    */
    /* *********************************** */


private:
    sf::RectangleShape m_shape;
    ShipPositionEnum m_positionType;
    unsigned int m_onIsleById;
    float m_damage;         // sailing arround, patroling, and fighting increases damage. Repair on isle,
    float m_halfWidth;
    Target m_target;

};

#endif // SHIP_H
