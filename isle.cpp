/* This File is part of WaterWorld. License is GNU GPL Version 3.
 * Please see https://github.com/ngc42/WaterWorld/blob/master/LICENSE for details.
 * WaterWorld is (C) 2016 by Eike Lange (eike@ngc42.de)
 */


#include "isle.h"
#include <math.h>
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


void Isle::setMaxTechnology(const float inTechnology)
{
    m_technology = m_technology < inTechnology ? inTechnology : m_technology;
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
    // population formula is based on a logistic function for populations,
    // see https://en.wikipedia.org/wiki/Logistic_function for details.
    // the magic factor is try-and-error with Libre Office Calc, try out:
    // =(60000 * B1 * EXP(Param3) ) / (60000 + (B1 * (EXP(Param3) - 1))) - 1
    const float max_population = 60000.0f;
    const float magic_population_factor = 0.097f;
    m_population = (max_population * m_population * exp(magic_population_factor)) /
            (max_population + (m_population * exp(magic_population_factor))) ;

    // The values here are try and error too. Idea is, that more population can
    // grow tech faster.
    // Libre Office (col A is tech, col B is population) =A1 + 0,3 +  0,2 * B1 / 60000
    m_technology = m_technology + 0.3 + 0.2 * m_population / max_population;

    // Libre Office: = C1 + 0,2 + 0,5 / A1 + 0,5 * B1 / 60000
    m_buildlevel = m_buildlevel + 0.2f + 0.5f * (1.0f / m_technology +  m_population / max_population);
    if(m_buildlevel >= 1.0f)
    {   // hurray we finished a ship
        m_buildlevel = 0.0f;
        return true;            // ask universe to release a ship
    }
    return false;
}
