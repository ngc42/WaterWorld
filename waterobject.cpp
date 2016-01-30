/* This File is part of WaterWorld. License is GNU GPL Version 3.
 * Please see https://github.com/ngc42/WaterWorld/blob/master/LICENSE for details.
 * WaterWorld is (C) 2016 by Eike Lange (eike@ngc42.de)
 */


#include "waterobject.h"


WaterObject::WaterObject(const uint inId, const uint inOwner, const QPointF inPos,
                         const QColor inColor, const float inTechnology)
    : m_id(inId), m_owner(inOwner), m_pos(inPos), m_color(inColor), m_technology(inTechnology)
{
}
