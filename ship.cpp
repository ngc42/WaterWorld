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
      m_damage(0.0f)
{
    m_shape = new QGraphicsRectItem(-7.0f, -7.0f, 14.0f, 14.0f);
    m_shape->setPos(inPos.x(), inPos.y());
    m_shape->hide();
    inOutRefScene->addItem(m_shape);
    m_shape->setBrush(QBrush(inColor));
    m_target.validTarget = false;
}


Ship::~Ship()
{
    delete m_shape;
}


void Ship::setOwner(const uint inOwner, const QColor inColor)
{
    m_owner = inOwner;
    m_color = inColor;
    m_shape->setBrush(QBrush(inColor));
    m_target.validTarget = false;
    m_damage = 0.0f;
}


void Ship::setPositionType(ShipPositionEnum inType)
{
    m_positionType = inType;
}


void Ship::setTargetIsle(const uint inTargetIsleId, const QPointF inPos)
{
    if((inTargetIsleId != m_onIsleById) or (m_positionType == ShipPositionEnum::S_OCEAN))
    {
        m_target.id = inTargetIsleId;
        m_target.pos = inPos;
        m_target.tType = Target::TargetEnum::T_ISLE;
        m_target.validTarget = true;
    }
}


void Ship::setTargetShip(const uint inTargetShipId, const QPointF inPos)
{
    if(inTargetShipId != m_id)
    {
        m_target.id = inTargetShipId;
        m_target.pos = inPos;
        m_target.tType = Target::TargetEnum::T_SHIP;
        m_target.validTarget = true;
    }
}


void Ship::setTargetWater(const QPointF inPos)
{
    m_target.id = 0;
    m_target.pos = inPos;
    m_target.tType = Target::TargetEnum::T_WATER;
    m_target.validTarget = true;
}


void Ship::setTargetFinished()
{
    m_target.id = 0;
    m_target.pos = {0.0f, 0.0f};
    m_target.tType = Target::TargetEnum::T_WATER;
    m_target.validTarget = false;
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
        setTargetFinished();
    }

    qDebug() << "Ship " << id() <<  " has damage " << m_damage;
}


void Ship::updateTargetPos(const QPointF inPos)
{
    m_target.pos = inPos;
}


bool Ship::nextRound()
{
    if(m_positionType == ShipPositionEnum::S_TRASH)
    {
        return true;    // arrived in heaven;
    }
    if(m_target.validTarget)
    {
        // Rod_Steward::Sailing, YouTube::DyIw0gcgfik

        //qInfo() << "sailing";

        m_positionType = ShipPositionEnum::S_OCEAN;
        m_shape->show();
        float dx = m_target.pos.x() - m_pos.x();
        float dy = m_target.pos.y() - m_pos.y();

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

