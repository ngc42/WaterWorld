/* This File is part of WaterWorld. License is GNU GPL Version 3.
 * Please see https://github.com/ngc42/WaterWorld/blob/master/LICENSE for details.
 * WaterWorld is (C) 2016 by Eike Lange (eike@ngc42.de)
 */


#include "universe.h"
#include <stdlib.h>     /* srand, rand */
#include <time.h>
#include <QBrush>
#include <QDebug>


Universe::Universe(QObject *inParent, UniverseScene *& inOutUniverseScene, const qreal inUniverseWidth,
                   const qreal inUniverseHeight, const uint inNumIsles)
    : QObject(inParent), m_lastInsertedId(10)
{
    createIsles(inOutUniverseScene, inUniverseWidth, inUniverseHeight, inNumIsles);
}


void Universe::deleteShipOnIsle(const uint inShipId)
{
    uint isleId = 0;
    for(Ship *s : m_ships)
    {
        ShipInfo sInfo = s->info();
        if(sInfo.id == inShipId)
        {
            isleId = sInfo.isleId;
            // just to get sure
            if((sInfo.posType != ShipPositionEnum::S_OCEAN) and isleId > 0)
            {
                deleteShip(inShipId);
                IsleInfo iInfo;
                isleForId(isleId, iInfo);
                showHumanIsle(iInfo);
            }
            break;
        }
    }
}


void Universe::setShipPatrolsIsle(const uint inShipId)
{
    for(Ship *s : m_ships)
    {
        ShipInfo sInfo = s->info();
        if(sInfo.id == inShipId)
        {
            ShipInfo sInfo = s->info();
            if(sInfo.posType == ShipPositionEnum::S_OCEAN)
                return;
            if(sInfo.hasTarget)
                s->setTargetFinished();
            // toggle Patruille status:
            if(sInfo.posType == ShipPositionEnum::S_ONISLE)
                s->setPositionType(ShipPositionEnum::S_PATROL);
            else
                s->setPositionType(ShipPositionEnum::S_ONISLE);
            // redraw isle info
            IsleInfo iInfo;
            isleForId(sInfo.isleId, iInfo);
            showHumanIsle(iInfo);

            break;
        }
    }
}


QPointF Universe::shipPosById(const uint inShipId)
{
    for(Ship *s : m_ships)
        if(s->id() == inShipId)
            return s->pos();
    return QPointF(0, 0);
}


void Universe::nextRound()
{
    for(Isle *isle : m_isles)
        isle->nextRound();
    for(Ship *ship : m_ships)
    {
        ShipInfo shipInfo = ship->info();
        Target target = ship->target();


        // update the target position for ships with T_SHIP,
        // because they move
        if(target.validTarget and target.tType == Target::TargetEnum::T_SHIP)
        {
            for(Ship *targetShip : m_ships)
            {
                ShipInfo targetShipInfo = targetShip->info();
                if(targetShipInfo.id == target.id)
                {
                    ship->updateTargetPos(targetShipInfo.pos);
                    break;
                }
            }
        }

        if( ship->nextRound() )
        {
            // arrived
            if(target.tType == Target::TargetEnum::T_ISLE)
            {
                // land or fight
                IsleInfo isleInfo;
                isleForId(target.id, isleInfo);

                if(isleInfo.owner == 0)
                {   // isle has no inhabitants
                    setIsleOwnerById(isleInfo.id, shipInfo.owner, shipInfo.color);
                    ship->landOnIsle(target.id, target.pos);
                }
                else if(isleInfo.owner == shipInfo.owner)
                {   // own isle
                    ship->landOnIsle(target.id, target.pos);
                }
                else
                {   // enemy isle -> fight

                    qDebug() <<" ship fights an isle" << ship->id();

                    // 1. fight isles patrol
                    shipFightIslePatol(ship, target.id);

                    qDebug() <<" after isle patrol" << ship->id();

                    // 2. fight isle
                    if( shipFightIsle(ship, target.id) )
                    {   // ship has won
                        ship->landOnIsle(target.id, target.pos);

                        // @fixme: every other enemy ship on this isle
                        // now owned by winner

                        qDebug() <<" ship now lands on isle" << ship->id();
                    }
                }
            }
            else if(target.tType == Target::TargetEnum::T_SHIP)
            {   // fight or rendez vous

                // find the other ship
                Ship *otherShip;
                for(Ship *s : m_ships)
                {
                    if(s->id() == target.id)
                    {
                        otherShip = s;
                        break;
                    }
                }

                ShipInfo otherShipInfo = otherShip->info();

                if(shipInfo.owner == otherShipInfo.owner)
                {   // same owner: just rendez vous
                    ship->setTargetFinished();
                }
                else
                {   // different owner -> fight
                    shipFightShip(ship, otherShip);
                    shipInfo = ship->info();    // update info
                    if(shipInfo.posType != ShipPositionEnum::S_TRASH)
                        ship->setTargetFinished();
                }
            }
            else // Target::TargetEnum::T_WATER
            {
                // nothing to do here
                ship->setTargetFinished();
            }

        }
        else
        {
            // repair ships on isle
            if(shipInfo.posType == ShipPositionEnum::S_ONISLE and shipInfo.damage > 0 and shipInfo.damage < 1.0f)
            {
                // 5%
                ship->addDamage(shipInfo.damage > 0.05f ? -0.05f : -shipInfo.damage );
            }

        }
    }

    // empty trash
    for(Ship *deleteThatShip : m_ships)
    {
        if(deleteThatShip->positionType() == ShipPositionEnum::S_TRASH)
            deleteShip(deleteThatShip->id());
    }

}


void Universe::callInfoScreen(const InfoscreenPage inPage, const IsleInfo inIsleInfo, const ShipInfo inShipInfo)
{
    if(inPage == InfoscreenPage::PAGE_ISLE)
    {
        IsleInfo isleInfo;
        isleForId(inIsleInfo.id, isleInfo);
        emit sigShowInfoIsle(isleInfo);
    }
    else if(inPage == InfoscreenPage::PAGE_HUMAN_ISLE)
    {
        IsleInfo isleInfo;
        isleForId(inIsleInfo.id, isleInfo);
        showHumanIsle(isleInfo);
    }
    else if(inPage == InfoscreenPage::PAGE_SHIP)
    {
        ShipInfo shipInfo;
        shipForId(inShipInfo.id, shipInfo);
        emit sigShowInfoShip(inShipInfo);
    }
    else if(inPage == InfoscreenPage::PAGE_WATER)
        emit sigShowInfoWater();
    // else -> PAGE_NOTHING -> ignore
}


void Universe::createIsles(UniverseScene *& inOutUniverseScene, const qreal inUniverseWidth, const qreal inUniverseHeight,
                           const uint inNumIsles)
{
    srand(time(NULL));

    const uint maxWidth = (uint) inUniverseWidth;
    const uint maxHeight = (uint) inUniverseHeight;

    qreal x, y;

    for(uint i = 0; i < inNumIsles; i++)
    {
        x = rand() % maxWidth;
        y = rand() % maxHeight;
        Isle *isle = new Isle(m_lastInsertedId++, 0, QPointF(x, y), Qt::gray);
        m_isles.append(isle);
    }

    Isle *isle = m_isles.at(0);
    isle->setOwner(1, Qt::green);

    createShipOnIsle(inOutUniverseScene, isle->id());
    createShipOnIsle(inOutUniverseScene, isle->id());

    isle = m_isles.at(1);
    isle->setOwner(2, Qt::red);
    createShipOnIsle(inOutUniverseScene, isle->id());

    Ship *s = m_ships.at( m_ships.count() -1);
    s->setTargetWater({500, 500});
}


void Universe::createShipOnIsle(UniverseScene *& inOutUniverseScene, uint inIsleId)
{
    for(Isle *isle : m_isles)
    {
        IsleInfo isleInfo = isle->info();
        if(isleInfo.id == inIsleId)
        {
            Ship *s = new Ship(inOutUniverseScene, m_lastInsertedId++, isleInfo.owner,
                               isleInfo.pos, isleInfo.color, ShipPositionEnum::S_ONISLE,
                               isleInfo.id, isleInfo.technology);
            m_ships.push_back(s);
            break;
        }
    }
}


void Universe::shipFightShip(Ship *& inOutAttacker, Ship *& inOutDefender)
{
    ShipInfo info1 = inOutAttacker->info();
    ShipInfo info2 = inOutDefender->info();

    if(info1.posType == ShipPositionEnum::S_TRASH)
        return;
    if(info2.posType == ShipPositionEnum::S_TRASH)
        return;

    // ship 1 is in advantage, as this is the attacker
    float force1 = (1.0f - info1.damage) * info1.technology * 1.1f;
    float force2 = (1.0f - info2.damage) * info2.technology;

    if(force1 > force2)
    {   // Attacker has won
        inOutAttacker->addDamage(force2 / force1);
        inOutDefender->setPositionType(ShipPositionEnum::S_TRASH);
    }
    else if(force2 > force1)
    {   // defender has won
        inOutDefender->addDamage(force2 / force1);
        inOutAttacker->setPositionType(ShipPositionEnum::S_TRASH);
    }
    else
    {
        // both forces are the same --> both die
        // should not happen with floats, but i better check
        inOutAttacker->setPositionType(ShipPositionEnum::S_TRASH);
        inOutDefender->setPositionType(ShipPositionEnum::S_TRASH);
    }
}


void Universe::shipFightIslePatol(Ship *& inOutAttacker, const uint inIsleId)
{
    for(Ship *defender : m_ships)
    {
        ShipInfo defenderInfo = defender->info();
        if(defenderInfo.posType == ShipPositionEnum::S_PATROL and
                defenderInfo.isleId == inIsleId)
            shipFightShip(inOutAttacker, defender);
    }
}


bool Universe::shipFightIsle(Ship *& inOutAttacker, const uint inIsleId)
{
    ShipInfo info1 = inOutAttacker->info();
    IsleInfo info2;
    isleForId(inIsleId, info2);
    if(info1.posType == ShipPositionEnum::S_TRASH)
        return false;
    if(info2.owner <= 1)
    {
        qErrnoWarning("Universe::shipFightIsle(): something went wrong");
        return false;
    }

    float force1 = 2.0f * info1.technology * (1.0 - info1.damage) * 10.0f;
    float force2 = info2.technology * (1.0f + info2.population / 1000.0f);

    if(force1 > force2)
    {   // attacker (ship) has won

        float damage_to_add = (force1 - force2) / (force1 + force2);
        inOutAttacker->addDamage(damage_to_add);
        info1 = inOutAttacker->info();

        if(info1.posType == ShipPositionEnum::S_TRASH)
        {   // isle is now empty without owner
            setIsleOwnerById(info2.id, 0, Qt::gray);
        }
        else
        {   // ship is alive
            setIsleOwnerById(info1.id, info1.owner, info1.color);
            return true;
        }

    }
    else if(force1 < force2)
    {   // isle has won
        inOutAttacker->setPositionType(ShipPositionEnum::S_TRASH);
        float new_population = info2.population * (force2 - force1) / (force1 + force2);

        if(new_population > 100.0)
            // reduce population, this is similar to damage on ships
            setIslePopulationById(info2.id, new_population);
        else
            // want to buy a new isle?
            setIsleOwnerById(info2.id, 0, Qt::gray);
    }
    else
    {   // magic, if this happens: both forces are the same
        inOutAttacker->setPositionType(ShipPositionEnum::S_TRASH);
        setIsleOwnerById(info2.id, 0, Qt::gray);
    }

    return false;   // ship has lost
}


void Universe::isleForPoint(const QPointF inScenePoint, IsleInfo & outIsleInfo)
{
    outIsleInfo.id = 0;
    for(Isle* isle : m_isles)
    {
        if(isle->pointInIsle(inScenePoint))
        {
            outIsleInfo = isle->info();
            break;
        }
    }
}


void Universe::isleForId(const uint inIsleId, IsleInfo & outIsleInfo)
{
    outIsleInfo.id = 0;
    for(Isle* isle : m_isles)
    {
        if(isle->id() == inIsleId)
        {
            outIsleInfo = isle->info();
            break;
        }
    }
}


void Universe::setIsleOwnerById(const uint inIsleId, const uint inNewOwner, const QColor inNewColor)
{
    for(Isle *isle : m_isles)
    {
        if(isle->id() == inIsleId)
            isle->setOwner(inNewOwner, inNewColor);
    }
}


void Universe::setIslePopulationById(const uint inIsleId, const float inNewPopulation)
{
    for(Isle *isle : m_isles)
    {
        if(isle->id() == inIsleId)
            isle->setPopulation(inNewPopulation);
    }
}


void Universe::shipForPoint(const QPointF inScenePoint, ShipInfo & outShipInfo)
{
    outShipInfo.id = 0;
    for(Ship *ship : m_ships)
    {
        if(ship->pointInShip(inScenePoint))
        {
            outShipInfo = ship->info();
            break;
        }
    }
}


void Universe::shipForId(const uint inShipId, ShipInfo & outShipInfo)
{
    outShipInfo.id = 0;
    for(Ship *ship : m_ships)
    {
        if(ship->id() == inShipId)
        {
            outShipInfo = ship->info();
            break;
        }
    }
}


void Universe::deleteShip(const uint inShipId)
{
    Ship *shipToDelete = 0;

    for(Ship *s : m_ships)
    {
        // for every other ship: if this ship has target ship with
        // id inShipId, then remove the target

        ShipInfo shipInfo = s->info();
        if(shipInfo.hasTarget and shipInfo.id != inShipId)
        {
            Target target = s->target();
            if(target.tType == Target::T_SHIP and target.id == inShipId)
                s->setTargetFinished();
        }

        if(s->id() == inShipId)
            shipToDelete = s;
    }

    // now, no other ship has target ship with id inShipId
    m_ships.removeOne(shipToDelete);
    delete shipToDelete;

}


void Universe::showHumanIsle(const IsleInfo inIsleInfo)
{
    QList<ShipInfo> sList;

    for(Ship *ship : m_ships)
    {
        ShipInfo info = ship->info();
        if(info.owner == 1 and
                (info.posType != ShipPositionEnum::S_OCEAN) and
                info.isleId == inIsleInfo.id)
            sList.append(info);
    }

    emit sigShowInfoHumanIsle(inIsleInfo, sList);
}


void Universe::slotUniverseViewClicked(QPointF scenePos)
{
    IsleInfo isleInfo;
    isleForPoint(scenePos, isleInfo);

    if(isleInfo.id > 0)
    {
        if(isleInfo.owner == 1)
        {
            showHumanIsle(isleInfo);
        }
        else
            // enemy or empty isle
            emit sigShowInfoIsle(isleInfo);
    }
    else
    {   // ship or water
        ShipInfo shipInfo;
        shipForPoint(scenePos, shipInfo);
        if(shipInfo.id > 0)
            emit sigShowInfoShip(shipInfo);
        else
            emit sigShowInfoWater();
    }
}


void Universe::slotUniverseViewClickedFinishTarget(QPointF scenePos, uint shipId)
{
    IsleInfo isleInfo;
    isleForPoint(scenePos, isleInfo);

    Ship *sourceShip;
    for(Ship *s : m_ships)
        if(s->id() == shipId)
        {
            sourceShip = s;
            break;
        }

    ShipInfo sourceShipInfo = sourceShip->info();

    if(isleInfo.id > 0)
    {
        // ship with id shipId wants target isle with id isleInfo.id
        sourceShip->setTargetIsle(isleInfo.id, isleInfo.pos);
    }
    else
    {
        ShipInfo targetShipInfo;
        shipForPoint(scenePos, targetShipInfo);
        if(targetShipInfo.id > 0 and targetShipInfo.id != sourceShipInfo.id)
        {   // target other ship
            qDebug() << "SET Target SHIP at " << scenePos << " id= " << sourceShipInfo.id;
            sourceShip->setTargetShip(targetShipInfo.id, targetShipInfo.pos);
        }
        else
        {   // ship with id shipId wants target water at scenePos
            qDebug() << "SET Target WATER at " << scenePos;
            sourceShip->setTargetWater(scenePos);
        }
    }

    // shipInfo describes old state. if this ship was on an isle
    // then we have to show this isle again
    if( (sourceShipInfo.posType == S_ONISLE) or (sourceShipInfo.posType == S_PATROL) )
    {
        // redraw isle info
        IsleInfo iInfo;
        isleForId(sourceShipInfo.isleId, iInfo);
        showHumanIsle(iInfo);
    }
}