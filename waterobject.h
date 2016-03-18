/* This File is part of WaterWorld. License is GNU GPL Version 3.
 * Please see https://github.com/ngc42/WaterWorld/blob/master/LICENSE for details.
 * WaterWorld is (C) 2016 by Eike Lange (eike@ngc42.de)
 */


#ifndef WATEROBJECT_H
#define WATEROBJECT_H


#include <QPoint>
#include <QColor>


class WaterObject
{
public:
    explicit WaterObject(const uint inId, const uint inOwner, const QPointF inPos,
                         const QColor inColor, const float inTechnology);

    ~WaterObject() {}

    // getter
    uint id() const { return m_id; }

    QPointF pos() const { return m_pos; }

    // Force: subclass must implement these method

    virtual float force() const = 0;
    virtual void takeDamage(const float inOpponentForce) = 0;

protected:

    uint m_id;
    uint m_owner;
    QPointF m_pos;
    QColor m_color;
    float m_technology;
};

#endif // WATEROBJECT_H
