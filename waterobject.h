/* This file is part of WaterWorld.
 * License GPL Version 3, see file "LICENSE" for details
 * Author Eike Lange
 */

#ifndef WATEROBJECT_H
#define WATEROBJECT_H

#include <SFML/Graphics.hpp>


struct  WaterObject
{
    // constructor
    WaterObject(const unsigned int inId,
                const unsigned int inOwner,
                const sf::Vector2f inPos,
                const sf::Color inColor)
      : m_id(inId),
        m_owner(inOwner),
        m_pos(inPos),
        m_color(inColor) {}

    // getter
    unsigned int id() const { return m_id; }
    unsigned int owner() const { return m_owner; }
    sf::Vector2f pos() const { return m_pos; }
    sf::Color color() const { return m_color; }


    /* *********************************** */
    /* Data begins here                    */
    /* *********************************** */


protected:
    unsigned int m_id;
    unsigned int m_owner;
    sf::Vector2f m_pos;
    sf::Color m_color;
};

#endif // WATEROBJECT_H
