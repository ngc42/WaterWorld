#ifndef ISLE_H
#define ISLE_H

#include "waterobject.h"


/* Things we get to know, if we click on an isle */
struct IsleInfo
{
    unsigned int id;
    unsigned int owner;
    sf::Color color;
};


struct Isle : WaterObject
{
public:
    // constructor
    Isle(const unsigned int inId,
         const unsigned int inOwner,
         const sf::Vector2f inPos,
         const sf::Color inColor)
        : WaterObject(inId, inOwner, inPos, inColor),
          m_radius(10.0f)
    {
        m_shape.setRadius(m_radius);
        m_shape.setPosition(inPos.x - m_radius, inPos.y - m_radius);
        m_shape.setFillColor(inColor);
    }



    // getter
    sf::CircleShape shape() const { return m_shape; }


    IsleInfo info()
    {
        IsleInfo outInfo;
        outInfo.id = m_id;
        outInfo.owner = m_owner;
        outInfo.color = m_color;
        return outInfo;
    }

    // setter
    void setOwner(const unsigned int inOwner, const sf::Color inColor)
    {
        m_owner = inOwner;
        m_color = inColor;
        m_shape.setFillColor(inColor);
    }


    // tester
    bool pointInIsle(const int inX, const int inY)
    {
        sf::Vector2f myPos = pos();
        int dx = myPos.x - inX;
        int dy = myPos.y - inY;
        return (dx * dx + dy * dy) < (m_radius * m_radius);
    }


private:
    float m_radius; // @fixme: hardcoded here and in ships.{h,cpp}
    sf::CircleShape m_shape;
};

#endif // ISLE_H
