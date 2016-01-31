/* This File is part of WaterWorld. License is GNU GPL Version 3.
 * Please see https://github.com/ngc42/WaterWorld/blob/master/LICENSE for details.
 * WaterWorld is (C) 2016 by Eike Lange (eike@ngc42.de)
 */


#include "isle.h"
#include <QBrush>
#include <QDebug>


Isle::Isle(const uint inId, const uint inOwner, const QPointF inPos,
     const QColor inColor)
    : WaterObject(inId, inOwner, inPos, inColor, 0.0f)
{
    m_shape = new QGraphicsEllipseItem(inPos.x() - 10.0f, inPos.y() - 10.0f, 20.0f, 20.0f);
    m_shape->setBrush(QBrush(inColor));

    m_population  = inOwner > 0 ? 100.1f : 0.0f;
    m_technology = inOwner > 0 ? 1.01f : 0.0f;
    m_buildlevel = 0.0f;
}


void Isle::setOwner(const uint inOwner, const QColor inColor)
{
    m_owner = inOwner;
    m_color = inColor;
    m_shape->setBrush(QBrush(inColor));

    m_population  = inOwner > 0 ? 100.1 : 0.0f;
    m_technology = inOwner > 0 ? 1.01f : 0.0f;
    m_buildlevel = 0.0f;
}


void Isle::setPopulation(const float inPopulation)
{
    m_population = inPopulation;
}


bool Isle::pointInIsle(const QPointF inPos)
{
    QPointF myPos = pos();
    qreal dx = myPos.x() - inPos.x();
    qreal dy = myPos.y() - inPos.y();
    return (dx * dx + dy * dy) < 100.0f;
}


bool Isle::nextRound()
{
    if(m_owner == 0)
        return false;
    if(m_population < 100.0f)
    {   // too few people on isle, they die by loneliness, sad but thats nature...
        setOwner(0, Qt::gray);
        return false;
    }

    // some magic numbers here:
    // population and technology should not grow too fast.
    // @fixme: I'm not happy with linear growth rate
    //         population, technology and buildlevel should depend on each other
    //         these things should depend on user's input parameters
    m_population = m_population * 1.05;
    if(m_population > 60000)
        m_population = m_population / 1.0987654;

    m_technology = m_technology + 0.1 * m_population / 60000.0f;
    m_buildlevel = m_buildlevel + 0.1f / m_technology;
    if(m_buildlevel >= 1.0f)
    {   // hurray we finished a ship
        m_buildlevel = 0.0f;
        return true;            // ask universe to release a ship
    }
    return false;
}
