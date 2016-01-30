#include "waterobject.h"

WaterObject::WaterObject(const uint inId, const uint inOwner, const QPointF inPos,
                         const QColor inColor, const float inTechnology)
    : m_id(inId), m_owner(inOwner), m_pos(inPos), m_color(inColor), m_technology(inTechnology)
{
}
