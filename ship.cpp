/* This File is part of WaterWorld. License is GNU GPL Version 3.
 * Please see https://github.com/ngc42/WaterWorld/blob/master/LICENSE for details.
 * WaterWorld is (C) 2016 by Eike Lange (eike@ngc42.de)
 */


#include <ship.h>
#include <player.h>

#include <QBrush>
#include <QDebug>


Ship::Ship(UniverseScene *& inOutRefScene, const uint inId, const uint inOwner,
           const QPointF inPos, const QColor inColor, const ShipPositionEnum inPosType,
           const uint inIsleId, const float inTechnology)
    : Ship(inOutRefScene, ShipTypeEnum::ST_BATTLESHIP, inId,
           inOwner, inPos, inColor, inPosType, inIsleId, inTechnology)
{
}


Ship::Ship(UniverseScene *& inOutRefScene, const ShipTypeEnum inShipType, const uint inId, const uint inOwner,
           const QPointF inPos, const QColor inColor, const ShipPositionEnum inPosType,
           const uint inIsleId, const float inTechnology)
    :  WaterObject(inId, inOwner, inPos, inColor, inTechnology),
      m_shipType(inShipType), m_positionType(inPosType), m_onIsleById(inIsleId),
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


void Ship::debugReport()
{
    qInfo() << "Hello, i'm ship " << id() << " " << shipTypeName(m_shipType);
    qInfo() << " > Targets: ";
    for(Target t : m_targetList)
    {
        qInfo() << "  - " << t.id << " pos: " << t.pos << " " << t.tType;
    }

}


QString Ship::shipTypeName(const ShipTypeEnum inShipType)
{
    switch(inShipType)
    {
        case ShipTypeEnum::ST_BATTLESHIP:
            return "Battleship";
        case ShipTypeEnum::ST_COURIER:
            return "Courier";
        case ShipTypeEnum::ST_COLONY:
            return "Colony";
        case ShipTypeEnum::ST_FLEET:
            return "Fleet";
        default:
            return "Only Programmer knows";
    }
}


ShipInfo Ship::info() const
{
    ShipInfo outInfo;
    outInfo.id = m_id;
    outInfo.shipType = m_shipType;
    outInfo.owner = m_owner;
    outInfo.color = m_color;
    outInfo.pos = m_pos;
    outInfo.posType = m_positionType;
    outInfo.isleId = m_onIsleById;
    outInfo.hasTarget = m_targetList.count() > 0;
    outInfo.cycleTargetList = m_cycleTargetList;
    outInfo.damage = m_damage;          // only useful for ST_BATTLESHIP and ST_FLEET
    outInfo.technology = m_technology;
    if(outInfo.hasTarget)
    {
        Target t = m_targetList.at( m_targetList.count() - 1 );
        outInfo.attachPos = t.pos;
    }
    else
    {
        outInfo.attachPos = m_pos;
    }
    return outInfo;
}


void Ship::setOwner(const uint inOwner, const QColor inColor)
{
    m_owner = inOwner;
    m_color = inColor;
    m_shape->setBrush(QBrush(Player::colorForOwner(inOwner)));
    removeTargets();
    m_cycleTargetList = false;
    m_damage = 0.0f;        // @fixme: really?
}


void Ship::setPositionType(ShipPositionEnum inType)
{
    m_positionType = inType;
}


void Ship::setCycleTargets(const bool inCycleTarget)
{
    m_cycleTargetList = inCycleTarget;
}


void Ship::setTargetIsle(const uint inTargetIsleId, const QPointF inPos)
{
    addCurrentPosToTarget();
    Target t;
    t.id = inTargetIsleId;
    t.pos = inPos;
    t.tType = Target::TargetEnum::T_ISLE;
    t.visited = false;
    m_targetList.append(t);
    fixTargetIndex();
}


void Ship::setTargetShip(const uint inTargetShipId, const QPointF inPos)
{
    if(inTargetShipId != m_id)
    {
        addCurrentPosToTarget();
        Target t;
        t.id = inTargetShipId;
        t.pos = inPos;
        t.tType = Target::TargetEnum::T_SHIP;
        t.visited = false;
        m_targetList.append(t);
        fixTargetIndex();
    }
}


void Ship::removeTargets()
{
    m_targetList.clear();
    fixTargetIndex();
}


void Ship::removeTargetShip(const uint inShipId)
{
    for(int idx = m_targetList.count()-1; idx > -1 ; idx--)
    {
        Target t = m_targetList.at(idx);
        if(t.tType == Target::T_SHIP and t.id == inShipId)
        {
            m_targetList.removeAt(idx);
        }
    }
    fixTargetIndex();
}


void Ship::removeTargetByIndex(const int inIndex)
{
    if( (inIndex + 1) > m_targetList.count() )
        return;
    if(inIndex >= m_targetList.count())
        return;
    m_targetList.remove(inIndex);
    fixTargetIndex();
}


void Ship::setTargetWater(const QPointF inPos)
{
    addCurrentPosToTarget();
    Target t;
    t.id = 0;
    t.pos = inPos;
    t.tType = Target::TargetEnum::T_WATER;
    t.visited = false;
    m_targetList.append(t);
    fixTargetIndex();
}


void Ship::setTargetFinished()
{
    int numTargets = m_targetList.count();
    if(numTargets > 0)
    {
        m_targetList[m_currentTargetIndex].visited = true;
        m_currentTargetIndex++; // next target
        if(m_currentTargetIndex >= numTargets)
        {
            if(m_cycleTargetList)
            {
                // start from 0 again
                m_currentTargetIndex = 0;
                for(Target & t : m_targetList)
                    t.visited = false;
            }
            else
            {
                // finished the list
                removeTargets();
            }
        }
    }
}


void Ship::updateTargetPos(const uint inShipId, const QPointF inPos)
{
    for(Target & t : m_targetList)
        if(t.tType == Target::T_SHIP and t.id == inShipId)
            t.pos = inPos;
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
    // Courier and Colony ships cannot defend themselves
    m_damage = m_damage + inDamageToAdd;
    if(m_damage > 0.999f or
       (m_shipType == ShipTypeEnum::ST_COURIER) or (m_shipType == ShipTypeEnum::ST_COLONY))
    {
        setPositionType(ShipPositionEnum::S_TRASH);
        removeTargets();
    }
}


bool Ship::nextRound()
{
    if(m_positionType == ShipPositionEnum::S_TRASH)
    {
        return true;    // arrived in heaven;
    }

    if(m_targetList.count() == 0)
        return false;

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
    return false;
}


bool Ship::pointInShip(const QPointF inPos)
{
    if(m_positionType != ShipPositionEnum::S_OCEAN)
        return false;
    QPointF myPos = pos();
    return inPos.x() > (myPos.x() - 7.0f) and
            inPos.x() < (myPos.x() + 7.0f) and
            inPos.y() > (myPos.y() - 7.0f) and
            inPos.y() < (myPos.y() + 7.0f);
}


QString Ship::typeName(const ShipTypeEnum inShipType)
{
    switch(inShipType)
    {
        case ShipTypeEnum::ST_BATTLESHIP:
            return QString("Battleship");

        case ShipTypeEnum::ST_COLONY:
            return QString("Colony");

        case ShipTypeEnum::ST_COURIER:
            return QString("Courier");

        case ShipTypeEnum::ST_FLEET:
            return QString("Fleet");

        default:    // whenever a stupid programmer forgets to add a type here ;-)
            return QString("unknown type");
    }
}


void Ship::fixTargetIndex()
{
    int count = m_targetList.count();
    if(count == 0)
    {
        // stop the engines
        m_cycleTargetList = false;
        m_currentTargetIndex = -1;
        return;
    }

    // the target is now the first unvisited target
    for(int i = 0; i < count; i++)
    {
        Target t = m_targetList.at(i);
        if(! t.visited)
        {
            m_currentTargetIndex = i;
            return;
        }
    }

    // cycle again
    if(m_cycleTargetList and count > 1)
    {
        m_currentTargetIndex = 0;
        for(Target & t : m_targetList)
            t.visited = false;
        return;
    }

    // here, there is nothing to fix
    removeTargets();
}


void Ship::addCurrentPosToTarget()
{
    // only add current pos to empty targets
    if(m_targetList.isEmpty())
    {
        Target t;
        switch(m_positionType)
        {
            case ShipPositionEnum::S_ONISLE:
            case ShipPositionEnum::S_PATROL:    // fall trough
                t.id = m_onIsleById;
                t.tType = Target::T_ISLE;
                break;
            default:
                qInfo() << "?? Ship::addCurrentPosToTarget()";
                t.id = 0;
                t.tType = Target::T_WATER;
                break;
        }
        t.pos = m_pos;
        t.visited = true;
        m_targetList.append(t);
    }
}
