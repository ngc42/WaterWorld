/* This File is part of WaterWorld. License is GNU GPL Version 3.
 * Please see https://github.com/ngc42/WaterWorld/blob/master/LICENSE for details.
 * WaterWorld is (C) 2016 by Eike Lange (eike@ngc42.de)
 */


#include "ship.h"
#include <QBrush>
#include <QDebug>


Ship::Ship(UniverseScene *& inOutRefScene, const uint inId, const uint inOwner,
           const QPointF inPos, const QColor inColor, const ShipPositionEnum inPosType,
           const uint inIsleId, const float inTechnology)
    :  WaterObject(inId, inOwner, inPos, inColor, inTechnology),
      m_positionType(inPosType), m_onIsleById(inIsleId),
      m_damage(0.0f), m_cycleTargetList(false), m_currentTargetIndex(-1)
{
    m_shape = new QGraphicsRectItem(-7.0f, -7.0f, 14.0f, 14.0f);
    m_shape->setPos(inPos.x(), inPos.y());
    m_shape->hide();
    inOutRefScene->addItem(m_shape);
    m_shape->setBrush(QBrush(inColor));
}


Ship::~Ship()
{
    delete m_shape;
}


ShipInfo Ship::info() const
{
    ShipInfo outInfo;
    outInfo.id = m_id;
    outInfo.owner = m_owner;
    outInfo.color = m_color;
    outInfo.pos = m_pos;
    outInfo.posType = m_positionType;
    outInfo.isleId = m_onIsleById;
    outInfo.hasTarget = m_currentTargetIndex >= 0;
    outInfo.damage = m_damage;
    outInfo.technology = m_technology;
    if(outInfo.hasTarget)
    {
        Target t = m_targetList.at(m_currentTargetIndex);
        qreal dx = m_pos.x() - t.pos.x();
        qreal dy = m_pos.y() - t.pos.y();
        outInfo.distanceTime = (uint) (std::sqrt( dx * dx + dy * dy ) / m_technology) + 1;
    }
    else
        outInfo.distanceTime = 0.0f;
    return outInfo;
}


void Ship::setOwner(const uint inOwner, const QColor inColor)
{
    m_owner = inOwner;
    m_color = inColor;
    m_shape->setBrush(QBrush(inColor));
    m_targetList.clear();
    m_currentTargetIndex = -1;
    m_damage = 0.0f;        // @fixme: really?
}


void Ship::setPositionType(ShipPositionEnum inType)
{
    m_positionType = inType;
}


void Ship::setTargetIsle(const uint inTargetIsleId, const QPointF inPos)
{
    Target t;
    t.id = inTargetIsleId;
    t.pos = inPos;
    t.tType = Target::TargetEnum::T_ISLE;
    t.validTarget = true;
    m_targetList.append(t);
}


void Ship::setTargetShip(const uint inTargetShipId, const QPointF inPos)
{
    if(inTargetShipId != m_id)
    {
        Target t;
        t.id = inTargetShipId;
        t.pos = inPos;
        t.tType = Target::TargetEnum::T_SHIP;
        t.validTarget = true;
        m_targetList.append(t);
    }
}


void Ship::deleteTargetShip(const uint inShipId)
{
    for(int idx = m_targetList.count()-1; idx > -1 ; idx--)
    {
        Target t = m_targetList.at(idx);
        if(t.tType == Target::T_SHIP and t.id == inShipId)
        {
            m_targetList.removeAt(idx);
        }
    }
}


void Ship::setTargetWater(const QPointF inPos)
{
    Target t;
    t.id = 0;
    t.pos = inPos;
    t.tType = Target::TargetEnum::T_WATER;
    t.validTarget = true;
    m_targetList.append(t);
}


void Ship::setTargetFinished()
{
    int numTargets = m_targetList.count();
    if(numTargets > 0)
    {
        m_currentTargetIndex++; // next target
        if(m_currentTargetIndex >= numTargets)
        {
            if(m_cycleTargetList)
            {
                // start from 0 again
                m_currentTargetIndex = 0;
            }
            else
            {
                // finished the list
                m_targetList.clear();       // nothing
                m_currentTargetIndex = -1;  // invalid index
            }
        }
    }
}


void Ship::landOnIsle(const uint inIsleId, const QPointF inPos)
{
    m_onIsleById = inIsleId;
    m_positionType = ShipPositionEnum::S_ONISLE;
    m_pos = inPos;
    m_shape->hide();
    setTargetFinished();
}


void Ship::addDamage(const float inDamageToAdd)
{
    m_damage = m_damage + inDamageToAdd;
    if(m_damage > 0.999f)
    {
        setPositionType(ShipPositionEnum::S_TRASH);
        m_targetList.clear();
        m_currentTargetIndex = -1;
    }
}


void Ship::updateTargetPos(const uint inShipId, const QPointF inPos)
{
    for(Target & t : m_targetList)
        if(t.tType == Target::T_SHIP and t.id == inShipId)
            t.pos = inPos;
}


bool Ship::nextRound()
{
    if(m_positionType == ShipPositionEnum::S_TRASH)
    {
        return true;    // arrived in heaven;
    }

    if(m_targetList.count() > 0 and m_currentTargetIndex < 0)
        m_currentTargetIndex = 0;   // start to sail

    if(m_targetList.count() > 0 and m_currentTargetIndex >= 0)
    {
        // Rod_Steward::Sailing, YouTube::DyIw0gcgfik
        Target currentTarget = m_targetList[m_currentTargetIndex];
        m_positionType = ShipPositionEnum::S_OCEAN;
        m_shape->show();
        float dx = currentTarget.pos.x() - m_pos.x();
        float dy = currentTarget.pos.y() - m_pos.y();

        float d = sqrt( dx * dx + dy * dy );

        if(d <= m_technology)
            return true;

        float ex = dx / d;
        float ey = dy / d;

        m_pos = QPointF{m_pos.x() + m_technology * ex, m_pos.y() + m_technology * ey};
        m_shape->setPos(m_pos);
    }
    return false;
}


bool Ship::pointInShip(const QPointF inPos)
{
    QPointF myPos = pos();
    return inPos.x() > (myPos.x() - 10.0f) and
            inPos.x() < (myPos.x() + 10.0f) and
            inPos.y() > (myPos.y() - 10.0f) and
            inPos.y() < (myPos.y() + 10.0f);
}

