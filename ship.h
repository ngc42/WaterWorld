/* This file is part of WaterWorld.
 * License GPL Version 3, see file "LICENSE" for details
 * Author Eike Lange
 */

#ifndef SHIP_H
#define SHIP_H

#include "waterobject.h"

enum TShipPosType {S_ONISLE, S_PATRUILLE, S_OCEAN};

/* Things we get to know, if we click on a ship */
struct ShipInfo
{
    unsigned int id;
    unsigned int owner;
    sf::Color color;
    sf::Vector2f pos;
    TShipPosType posType;
    unsigned int isleId;        // if not on ocean
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
        return outInfo;
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

};

#endif // SHIP_H
