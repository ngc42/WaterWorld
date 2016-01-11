/* This file is part of WaterWorld.
 * License GPL Version 3, see file "LICENSE" for details
 * Author Eike Lange
 */

#ifndef SHIP_H
#define SHIP_H

#include "waterobject.h"


enum TTargetType {T_SHIP, T_ISLE, T_WATER};

struct Target
{
    bool validTarget;
    TTargetType tType;
    unsigned int id;  // ship or isle id
    sf::Vector2f pos;
};


enum TShipPosType {S_ONISLE, S_PATRUILLE, S_OCEAN, S_TRASH};

/* Things we get to know, if we click on a ship */
struct ShipInfo
{
    unsigned int id;
    unsigned int owner;
    sf::Color color;
    sf::Vector2f pos;
    TShipPosType posType;
    unsigned int isleId;        // if not on ocean
    bool hasTarget;
};


struct Ship : WaterObject
{
public:
    Ship(const unsigned int inId, const unsigned int inOwner, const sf::Vector2f inPos,
         const sf::Color inColor, const TShipPosType inPosType, unsigned int inIsleId)
        :  WaterObject(inId, inOwner, inPos, inColor),
          m_positionType(inPosType), m_onIsleById(inIsleId),
          m_halfWidth(10.0f)  // @fixme: hard coded

    {
        m_shape.setSize(sf::Vector2f(2.0f * m_halfWidth, 2.0f * m_halfWidth));
        m_shape.setPosition(inPos.x - m_halfWidth, inPos.y - m_halfWidth);
        m_shape.setFillColor(inColor);
        m_target.validTarget = false;
    }


    // getter
    sf::RectangleShape shape() const { return m_shape; }


    TShipPosType postionType() const { return m_positionType; }


    ShipInfo info()
    {
        ShipInfo outInfo;
        outInfo.id = m_id;
        outInfo.owner = m_owner;
        outInfo.color = m_color;
        outInfo.pos = m_pos;
        outInfo.posType = m_positionType;
        outInfo.isleId = m_onIsleById;
        outInfo.hasTarget = m_target.validTarget;
        return outInfo;
    }


    // setter
    void setPositionType(TShipPosType inType)
    {
        m_positionType = inType;
    }


    void setTargetIsle(const unsigned int inTargetIsleId, const sf::Vector2f inPos)
    {
        if((inTargetIsleId != m_onIsleById) or (m_positionType == TShipPosType::S_OCEAN))
        {
            m_target.id = inTargetIsleId;
            m_target.pos = inPos;
            m_target.tType = TTargetType::T_ISLE;
            m_target.validTarget = true;
        }
    }


    void setTargetShip(const unsigned int inTargetShipId, const sf::Vector2f inPos)
    {
        if(inTargetShipId != m_id)
        {
            m_target.id = inTargetShipId;
            m_target.pos = inPos;
            m_target.tType = TTargetType::T_SHIP;
            m_target.validTarget = true;
        }
    }


    void setTargetWater(const sf::Vector2f inPos)
    {
        m_target.id = 0;
        m_target.pos = inPos;
        m_target.tType = TTargetType::T_WATER;
        m_target.validTarget = true;

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
    sf::Vector2f m_pos;
    sf::RectangleShape m_shape;
    TShipPosType m_positionType;
    unsigned int m_onIsleById;
    float m_halfWidth;

    Target m_target;

};

#endif // SHIP_H
