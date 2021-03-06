/* This File is part of WaterWorld. License is GNU GPL Version 3.
 * Please see https://github.com/ngc42/WaterWorld/blob/master/LICENSE for details.
 * WaterWorld is (C) 2016 by Eike Lange (eike@ngc42.de)
 */


#include <isle.h>
#include <player.h>

#include <math.h>
#include <QBrush>
#include <QDebug>


Isle::Isle(const uint inId, const uint inOwner, const QPointF inPos,
     const QColor inColor)
    : WaterObject(inId, inOwner, inPos, inColor, 0.0f),
      m_shipToBuild(ShipTypeEnum::ST_BATTLESHIP)
{
    m_shape = new QGraphicsEllipseItem(inPos.x() - 10.0f, inPos.y() - 10.0f, 20.0f, 20.0f);
    m_shape->setBrush(QBrush(inColor));

    m_population  = inOwner > Player::PLAYER_UNSETTLED ? 100.1f : 0.0f;
    m_technology = inOwner > Player::PLAYER_UNSETTLED ? 1.01f : 0.0f;
    m_buildlevel = 0.0f;
    setDefaultTargetNothing();
}


void Isle::setOwner(const uint inOwner, const QColor inColor)
{
    m_owner = inOwner;
    m_color = inColor;
    m_shape->setBrush(QBrush(inColor));

    m_population  = inOwner > Player::PLAYER_UNSETTLED ? 100.1 : 0.0f;
    m_technology = inOwner > Player::PLAYER_UNSETTLED ? 1.01f : 0.0f;
    m_buildlevel = 0.0f;
    setDefaultTargetNothing();
}



void Isle::setPopulation(const float inPopulation)
{
    m_population = inPopulation;
}


void Isle::setShipToBuild(const ShipTypeEnum inShipToBuild)
{
    if(inShipToBuild == m_shipToBuild)
        return;
    m_shipToBuild = inShipToBuild;
    m_buildlevel = 0.0f;    // start fresh, sorry user
}


void Isle::setMaxTechnology(const float inTechnology)
{
    m_technology = m_technology < inTechnology ? inTechnology : m_technology;
}


void Isle::setDefaultTargetIsle(const QPointF inTargetPos, const uint inIsleId)
{
    m_defaultTargetType = IsleInfo::T_ISLE;
    m_defaultTargetIsle = inIsleId;
    m_defaultTargetPos = inTargetPos;
}


void Isle::setDefaultTargetWater(const QPointF inTargetPos)
{
    m_defaultTargetType = IsleInfo::T_WATER;
    m_defaultTargetIsle = 0;
    m_defaultTargetPos = inTargetPos;
}


void Isle::setDefaultTargetNothing()
{
    m_defaultTargetType = IsleInfo::T_NOTHING;
    m_defaultTargetIsle = 0;
    m_defaultTargetPos = QPointF(0, 0);
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
    if(m_owner == Player::PLAYER_UNSETTLED)
        return false;
    if(m_population < 100.0f)
    {   // too few people on isle, they die by loneliness, sad but thats nature...
        setOwner(Player::PLAYER_UNSETTLED, Player::colorForOwner(Player::PLAYER_UNSETTLED));
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
    // Libre Office (col A is tech, col B is population) =A1 + 0,1 +  0,2 * B1 / 60000
    m_technology = m_technology + 0.01 + 0.1 * m_population / max_population;

    // Libre Office: = C1 + 0,2 + 0,5 / A1 + 0,5 * B1 / 60000
    m_buildlevel = m_buildlevel + 0.08f + 0.1f * (1.0f / m_technology +  m_population / max_population);
    if(m_buildlevel >= 1.0f)
    {   // hurray we finished a ship
        m_buildlevel = 0.0f;
        return true;            // ask universe to release a ship
    }
    return false;
}


float Isle::force() const
{
    return m_technology * m_population / 1000.0f;
}


void Isle::takeDamage(const float inOpponentForce)
{
    m_population = m_population - inOpponentForce * 1000 / m_technology;
    if(m_population < 100.0f)
    {
        // die on too much damage
        setOwner(Player::PLAYER_UNSETTLED, Player::colorForOwner(Player::PLAYER_UNSETTLED));
    }
}
