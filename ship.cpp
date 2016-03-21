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
      m_damage(0.0f), m_carryTechnology(0.0f), m_cycleTargetList(false), m_currentTargetIndex(-1)
{
    m_shape = new QGraphicsRectItem(-7.0f, -7.0f, 14.0f, 14.0f);
    m_shape->setPos(inPos.x(), inPos.y());
    m_shape->hide();
    inOutRefScene->addItem(m_shape);
    m_shape->setBrush(QBrush(inColor));
    setCarryTechnology(inTechnology);   // for ST_COURIER
    m_fleetId = 0;                      // not part of fleet
}


Ship::~Ship()
{
    if(m_shipType == ShipTypeEnum::ST_FLEET)
        for(Ship * &s : m_fleetShips)
            delete s;
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
    outInfo.carryTechnology = m_carryTechnology;
    outInfo.fleetId = m_fleetId;
    return outInfo;
}


void Ship::setOwner(const uint inOwner, const QColor inColor)
{
    m_owner = inOwner;
    m_color = inColor;
    m_shape->setBrush(QBrush(Player::colorForOwner(inOwner)));
    removeTargets();
    m_cycleTargetList = false;
    // if this is a fleet, do the same for all members
    if(m_shipType == ShipTypeEnum::ST_FLEET)
        for(Ship *s : m_fleetShips)
            s->setOwner(inOwner, inColor);
}


void Ship::setPositionType(ShipPositionEnum inType)
{
    if(inType == ShipPositionEnum::SP_OCEAN)
        m_shape->show();
    else
        m_shape->hide();
    m_positionType = inType;
}


void Ship::setCarryTechnology(const float inTechlevel)
{
    // courier ships taking the maximum
    if( (m_shipType == ShipTypeEnum::ST_COURIER) and (inTechlevel > m_carryTechnology))
        m_carryTechnology = inTechlevel;
    // @fixme: fleets may contain a courier
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
    setPositionType(ShipPositionEnum::SP_ONISLE);
    m_pos = inPos;
    setTargetFinished();
}


bool Ship::nextRound()
{
    if(m_positionType == ShipPositionEnum::SP_TRASH or
       m_positionType == ShipPositionEnum::SP_IN_FLEET)
    {
        return false;    // we don't care, because universe/fleet does
    }

    if(m_targetList.count() == 0)
        return false;

    // Rod_Steward::Sailing, YouTube::DyIw0gcgfik
    Target currentTarget = m_targetList[m_currentTargetIndex];
    setPositionType(ShipPositionEnum::SP_OCEAN);
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


float Ship::force() const
{
    if(isDead())
        return 0.0f;
    if(m_shipType == ShipTypeEnum::ST_BATTLESHIP)
        return (1.0 - m_damage) * m_technology;
    if(m_shipType == ShipTypeEnum::ST_FLEET)
    {
        float f = 0.0;
        for(Ship *s : m_fleetShips)
            f = f + s->force();
        return f;
    }
    return 0.0f;
}


void Ship::takeDamage(const float inOpponentForce)
{
    if(m_shipType == ShipTypeEnum::ST_BATTLESHIP)
    {
        m_damage  =  m_damage  + inOpponentForce/m_technology;
        if(m_damage < 0.99f)
            return;
    }
    else if(m_shipType == ShipTypeEnum::ST_FLEET)
    {
        // count battleships and fleets in this fleet
        uint numMember = 0;
        for(Ship *s : m_fleetShips)
        {
            ShipTypeEnum memberType = s->info().shipType;
            if(memberType == ShipTypeEnum::ST_FLEET or memberType == ShipTypeEnum::ST_BATTLESHIP)
                numMember++;
        }
        if(numMember == 0)
        {
            setDead();
            return;
        }
        // every battleship/fleet member gets a part of the damage
        float opponentForcePart = inOpponentForce / numMember;
        for(Ship *s : m_fleetShips)
        {
            ShipTypeEnum memberType = s->info().shipType;
            if(memberType == ShipTypeEnum::ST_FLEET or memberType == ShipTypeEnum::ST_BATTLESHIP)
                s->takeDamage(opponentForcePart);
        }
        updateFleet();
        return;
    }

    // every other shiptype:
    setDead();
}


bool Ship::isDead() const
{
    return m_positionType == ShipPositionEnum::SP_TRASH or m_damage >= 1.0f;
}


void Ship::setDead()
{
    setPositionType(ShipPositionEnum::SP_TRASH);
    removeTargets();
    if(m_shipType == ShipTypeEnum::ST_FLEET)
    {
        for(Ship *s : m_fleetShips)
            s->setDead();
        updateFleet();
    }
}


void Ship::repair()
{
    if(isDead())
        return;
    if(m_shipType == ShipTypeEnum::ST_FLEET)
    {
        for(Ship *s : m_fleetShips)
            s->repair();
        updateFleet();
    }
    else
    {
        m_damage = m_damage - 0.05;
        if(m_damage < 0)
            m_damage = 0.0;
    }
}


bool Ship::pointInShip(const QPointF inPos)
{
    if(m_positionType != ShipPositionEnum::SP_OCEAN)
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
            case ShipPositionEnum::SP_ONISLE:
            case ShipPositionEnum::SP_PATROL:    // fall trough
                t.id = m_onIsleById;
                t.tType = Target::T_ISLE;
                break;
            case ShipPositionEnum::SP_IN_FLEET:
                // there is nothing we should do here
                return;
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


void Ship::addShipToFleet(Ship* & inOtherShip)
{
    Q_ASSERT(m_shipType == ShipTypeEnum::ST_FLEET);
    m_fleetShips.append(inOtherShip);
    updateFleet();
}


Ship* Ship::removeShipFromFleet(const uint inShipId)
{
    Q_ASSERT(m_shipType == ShipTypeEnum::ST_FLEET);
    Ship* shipToRemove = 0;
    for(int i = 0; i < m_fleetShips.count(); i++)
    {
        if(m_fleetShips.at(i)->id() == inShipId)
        {
            shipToRemove = m_fleetShips[i];
            m_fleetShips.remove(i);
            break;
        }
    }

    Q_ASSERT(shipToRemove);

    // tell shipToRemove to be removed from fleet
    ShipInfo fleetInfo = info();
    shipToRemove->removeFromFleet(fleetInfo);

    // fleet is empty
    if(m_fleetShips.count() == 0)
    {   // we removed the last one
        setDead();
    }

    updateFleet();
    return shipToRemove;
}


void Ship::addToFleet(const uint inFleetId)
{
    // we don't have own targets
    removeTargets();
    if(m_positionType == ShipPositionEnum::SP_TRASH)
        return;
    setPositionType(ShipPositionEnum::SP_IN_FLEET);
    m_fleetId = inFleetId;
}


void Ship::removeFromFleet(const ShipInfo inFleetInfo)
{
    if(m_positionType == ShipPositionEnum::SP_TRASH)
        return;
    setPositionType(inFleetInfo.posType);
    m_onIsleById = inFleetInfo.isleId;
    m_carryTechnology = inFleetInfo.carryTechnology;
    m_pos = inFleetInfo.pos;
    m_fleetId = 0;
}


void Ship::updateFleet()
{
    Q_ASSERT(m_shipType == ShipTypeEnum::ST_FLEET);
    float techlevel = 0.0;
    float maxTech = 0.0;
    float damage = 0.0;
    m_carryTechnology = 0.0;
    for(Ship * &s : m_fleetShips)
    {
        ShipInfo shipInfo = s->info();
        if(shipInfo.shipType == ShipTypeEnum::ST_FLEET)
        {
            s->updateFleet();
            // update info
            shipInfo = s->info();

        }
        if(s->isDead())
        {
            m_fleetShips.removeOne(s);
            // delete all elements, if s is a fleet
            deleteFleetContent(s);
            delete s;
        }
        else
        {
            // @fixme: I'm not sure, if this is all correct
            maxTech = shipInfo.technology > maxTech ? shipInfo.technology : maxTech;
            techlevel += shipInfo.technology;
            damage = damage + shipInfo.technology * shipInfo.damage;
            if(shipInfo.carryTechnology > m_carryTechnology)
                m_carryTechnology = shipInfo.carryTechnology;
        }
    }
    m_technology = maxTech;
    m_damage = damage / techlevel;

    if(m_damage >= 0.99f or m_fleetShips.count() == 0)
        setPositionType(ShipPositionEnum::SP_TRASH);
}


void Ship::deleteFleetContent(Ship * &inDeleteShip)
{
    if(inDeleteShip->info().shipType == ShipTypeEnum::ST_FLEET)
    {
        Ship *elementShip;
        while( ! m_fleetShips.isEmpty() )
        {
            // delete the content of this fleet recursive
            elementShip = m_fleetShips.takeFirst();
            deleteFleetContent(elementShip);
            delete elementShip;
        }
    }
}


bool Ship::fleetContainsShipType(const ShipTypeEnum shipType)
{
    Q_ASSERT(m_shipType == ShipTypeEnum::ST_FLEET);

    for(Ship *s : m_fleetShips)
    {
        ShipInfo sInfo = s->info();
        if(sInfo.shipType == shipType)
            return true;
        if(sInfo.shipType == ShipTypeEnum::ST_FLEET)
            if(s->fleetContainsShipType(shipType))
                return true;
    }
    return false;
}


bool Ship::fleetRemoveFirstColonyShip()
{
    Q_ASSERT(m_shipType == ShipTypeEnum::ST_FLEET);
    for(Ship * &s : m_fleetShips)
    {
        ShipInfo sInfo = s->info();
        if(sInfo.shipType == ShipTypeEnum::ST_COLONY)
        {
            s->setDead();
            return true;
        }
        if(sInfo.shipType == ShipTypeEnum::ST_FLEET)
        {
            bool killed = s->fleetRemoveFirstColonyShip();
            if(killed)
                return true;
        }

    }
    return false;
}
