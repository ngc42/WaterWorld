#ifndef WATEROBJECT_H
#define WATEROBJECT_H

#include <qpoint.h>
#include <qcolor.h>

class WaterObject
{
public:
    explicit WaterObject(const uint inId, const uint inOwner, const QPointF inPos,
                         const QColor inColor, const float inTechnology);

    // getter
    uint id() const { return m_id; }
    QPointF pos() const { return m_pos; }

protected:
    uint m_id;
    uint m_owner;
    QPointF m_pos;
    QColor m_color;
    float m_technology;
};

#endif // WATEROBJECT_H
