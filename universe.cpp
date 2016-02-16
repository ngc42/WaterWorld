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
    createIsles(inUniverseWidth, inUniverseHeight, inNumIsles);

    // add created isles to the scene
    for(Isle *isle : m_isles)
    {
        inOutUniverseScene->addItem(isle->shape());
    }

    // @fixme: hardcoded owner number, shared with createIsles()
    m_strategy = new Strategy(2);
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
            s->removeTargets();
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


float Universe::shipTechById(const uint inShipId)
{
    for(Ship *s : m_ships)
        if(s->id() == inShipId)
            return s->info().technology;
    return 1.0f;
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


void Universe::getAllIsleInfos(QList<IsleInfo> & outIsleInfos)
{
    outIsleInfos.clear();
    for(Isle *isle : m_isles)
        outIsleInfos.append(isle->info());
}


void Universe::getAllShipInfos(QList<ShipInfo> & outShipInfo)
{
    outShipInfo.clear();
    for(Ship *ship : m_ships)
        outShipInfo.append(ship->info());
}


void Universe::nextRound(UniverseScene *& inOutUniverseScene)
{
    qInfo() << "BEGIN NEXTROUND ==================";
    prepareStrategies();
    QList<StrategyCommand> strategyCommands;
    m_strategy->nextRound(strategyCommands);
    processStrategyCommands(m_strategy->owner(), strategyCommands);

    for(Isle *isle : m_isles)
    {
        if(isle->nextRound())
        {
            qDebug() << "isle " << isle->id() << "finished a ship";
            createShipOnIsle(inOutUniverseScene, isle->id());
        }
    }
    for(Ship *ship : m_ships)
    {
        ShipInfo shipInfo = ship->info();

        if(shipInfo.posType == ShipPositionEnum::S_TRASH or shipInfo.damage >= 1.0f)
        {
            if(shipInfo.posType != ShipPositionEnum::S_TRASH)
                qDebug() << " -- WRONG: id:" << shipInfo.id  << " owner:" << shipInfo.owner << shipInfo.damage;
            continue;
        }

        if( ship->nextRound() ) // ship->nextRound() returns true on arrive
        {   // arrived

            // as ships which arrived in heaven get cought above, the ships here MUST
            // have a target, so it is save to call:
            Target target = ship->currentTarget();

            if(target.tType == Target::TargetEnum::T_ISLE)
            {
                // land or fight
                IsleInfo isleInfo;
                isleForId(target.id, isleInfo);

                if(isleInfo.owner == 0)
                {   // isle has no inhabitants
                    setIsleOwnerById(isleInfo.id, shipInfo.owner, shipInfo.color);
                    shipLandOnIsle(ship);
                }
                else if(isleInfo.owner == shipInfo.owner)
                {   // own isle
                    shipLandOnIsle(ship);
                }
                else
                {   // enemy isle -> fight

                    // 1. fight isles patrol
                    shipFightIslePatol(ship, target.id);

                    // 2. fight isle
                    if( shipFightIsle(ship, target.id) )
                    {   // ship has won
                        shipLandOnIsle(ship);

                        // every other enemy ship on this isle is now owned by the winner
                        float local_tech_max = 0.1f;
                        for(Ship *isleShip : m_ships)
                        {
                            ShipInfo isleShipInfo = isleShip->info();
                            if(isleShipInfo.posType == ShipPositionEnum::S_ONISLE and
                               isleShipInfo.isleId == target.id)
                            {   // set new owner
                                isleShip->setOwner(shipInfo.owner, shipInfo.color);
                                // find the maximum technology for pirated ships
                                local_tech_max = isleShipInfo.technology > local_tech_max ? isleShipInfo.technology : local_tech_max;
                            }
                            if(local_tech_max > shipInfo.technology)
                            {   // maybe, one of the pirated ships has higher tech than the ship which landed
                                for(Isle *newShipsIsle : m_isles)
                                    if(newShipsIsle->id() == target.id)
                                    {
                                        newShipsIsle->setMaxTechnology(local_tech_max);
                                        break;
                                    }
                            }
                        }
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
        ShipInfo dmgShipInfo = deleteThatShip->info();
        if(dmgShipInfo.posType == ShipPositionEnum::S_TRASH or dmgShipInfo.damage >= 1.0f)
            qDebug() << " -- should delete: " << dmgShipInfo.id;

        if(deleteThatShip->positionType() == ShipPositionEnum::S_TRASH)
        {
            qDebug() << " -- delete " << dmgShipInfo.id;
            deleteShip(deleteThatShip->id());
        }
    }


    // update the target position for ships with tType T_SHIP, because they could have moved
    // this is very straight forward (for every ship check target ships and update pos or delete target entry)
    // but it is worse in the meaning of complexity O(n^4) :-(
    for( Ship *updateShip : m_ships )
    {
        QVector<Target> targets = updateShip->targets();
        for(Target t : targets)
        {
            if(t.tType == Target::T_SHIP)
            {
                bool targetIsAlive = false;
                for( Ship *otherShip : m_ships)
                {
                    ShipInfo otherShipInfo = otherShip->info();
                    if(otherShipInfo.id == t.id)
                    {
                        targetIsAlive = true;
                        updateShip->updateTargetPos(t.id, otherShipInfo.pos);
                        break;
                    }
                }
                if(! targetIsAlive)
                {
                    // the target ship was deleted, so delete the target too
                    updateShip->deleteTargetShip(t.id);
                }
            }
        }
    }

    qInfo() << "END NEXTROUND ==================";
}


void Universe::callInfoScreen(const InfoscreenPage inPage, const IsleInfo inIsleInfo, const ShipInfo inShipInfo)
{
    if(inPage == InfoscreenPage::PAGE_ISLE)
    {
        IsleInfo isleInfo;
        isleForId(inIsleInfo.id, isleInfo);
        // owner may have changed
        if(isleInfo.owner == 1)
            // human
            showHumanIsle(isleInfo);
        else
            emit sigShowInfoIsle(isleInfo);
    }
    else if(inPage == InfoscreenPage::PAGE_HUMAN_ISLE)
    {
        IsleInfo isleInfo;
        isleForId(inIsleInfo.id, isleInfo);
        // owner may have changed
        if(isleInfo.owner == 1)
            showHumanIsle(isleInfo);
        else
            emit sigShowInfoIsle(isleInfo);
    }
    else if(inPage == InfoscreenPage::PAGE_SHIP)
    {
        ShipInfo shipInfo;
        QVector<Target> targetsUnused;
        shipForId(inShipInfo.id, shipInfo, targetsUnused);
        emit sigShowInfoShip(inShipInfo);
    }
    else if(inPage == InfoscreenPage::PAGE_WATER)
        emit sigShowInfoWater();
    else if(inPage == InfoscreenPage::PAGE_HUMAN_SHIP)
    {
        ShipInfo sInfo;
        QVector<Target> targetList;
        shipForId(inShipInfo.id, sInfo, targetList);
        emit sigShowInfoHumanShip(sInfo, targetList);
    }
    // else -> PAGE_NOTHING -> ignore
}


void Universe::createIsles(const qreal inUniverseWidth, const qreal inUniverseHeight, const uint inNumIsles)
{
    srand(time(NULL));

    const uint maxWidth = (uint) inUniverseWidth;
    const uint maxHeight = (uint) inUniverseHeight;

    qreal x, y;
    bool tooClose = true;  // pos is too close to another isle
    for(uint i = 0; i < inNumIsles; i++)
    {
        do
        {
            x = rand() % maxWidth;      // random pos
            y = rand() % maxHeight;
            tooClose = false;
            for(Isle *isla : m_isles)
            {
                QPointF p = isla->pos() - QPointF(x, y);
                if(p.manhattanLength() < 50.0f)     // |p.x| + |p.y| < 50 is too close
                {
                    tooClose = true;
                    break;
                }
            }
        } while(tooClose);

        Isle *isle = new Isle(m_lastInsertedId++, 0, QPointF(x, y), Qt::gray);
        m_isles.append(isle);
    }

    Isle *isle = m_isles.at(0);
    isle->setOwner(1, Qt::green);

    isle = m_isles.at(1);
    isle->setOwner(2, Qt::red);
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
            // default target
            switch(isleInfo.defaultTargetType)
            {
                case IsleInfo::T_ISLE:
                    s->setTargetIsle(isleInfo.defaultTargetIsle, isleInfo.defaultTargetPos);
                    break;
                case IsleInfo::T_WATER:
                    s->setTargetWater(isleInfo.defaultTargetPos);
                    break;
                default:
                    break;
            }

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

    qDebug() << "Ship fight ship, forces: " << force1 << " " << force2;

    if(force1 > force2)
    {   // Attacker has won
        inOutAttacker->addDamage(force2 / force1);
        inOutDefender->setPositionType(ShipPositionEnum::S_TRASH);
    }
    else if(force2 > force1)
    {   // defender has won
        inOutDefender->addDamage(force1 / force2);
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
    if(info2.owner < 1 or info2.owner == info1.owner)
    {
        //qInfo() << "BUG: unsettled or own isle, automatic won, no damage added";
        return true;
    }
    if(info1.damage > 0.999f)
    {
        //qInfo() << "BUG: A totally damaged ship wants to conquer an isle --> rejected";
        return false;
    }

    float force1 = 2.0f * info1.technology * (1.0 - info1.damage) * 10.0f;
    float force2 = info2.technology * (1.0f + info2.population / 1000.0f);

    if(force1 > force2)
    {   // attacker (ship) has won
        float damage_to_add = force2 / force1;
        inOutAttacker->addDamage(damage_to_add);
        info1 = inOutAttacker->info();

        if(info1.posType == ShipPositionEnum::S_TRASH)
        {   // isle is now empty without owner
            setIsleOwnerById(info2.id, 0, Qt::gray);
            return false;   // ship is too damaged
        }
        else
        {   // ship is alive
            setIsleOwnerById(info2.id, info1.owner, info1.color);
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


void Universe::shipLandOnIsle(Ship *& inOutShipToLand)
{
    ShipInfo shipInfo = inOutShipToLand->info();
    if(! shipInfo.hasTarget)
    {   // we don'd have a target but are forced to land on an target isle, stange!
        qInfo() << "ERR 1002 in Universe::shipLandOnIsle()";
        return;
    }
    Target targetInfo = inOutShipToLand->currentTarget();
    if(targetInfo.tType != Target::TargetEnum::T_ISLE)
    {   // just to get sure
        qDebug() << "ERR 1 in Universe::shipLandOnIsle()";
        return;
    }

    Isle *targetIsle = 0;
    IsleInfo isleInfo;
    isleInfo.id = 0;

    for(Isle *isle : m_isles )
        if(isle->id() == targetInfo.id)
        {
            targetIsle = isle;
            isleInfo = isle->info();
            break;
        }

    if(isleInfo.id == 0 or shipInfo.owner != isleInfo.owner)
    {   // just to get sure
        qDebug() << "ERR 2 in Universe::shipLandOnIsle()";
        return;
    }

    // transfer technology to the isle
    qInfo() << "Set max technology: ship has" << shipInfo.technology << " isle has: " << targetIsle->info().technology;
    targetIsle->setMaxTechnology(shipInfo.technology);
    qInfo() << " now isle has " << targetIsle->info().technology;

    // and realy land
    inOutShipToLand->landOnIsle(isleInfo.id, isleInfo.pos);
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


int Universe::shipIndexForPoint(const QPointF inScenePoint) const
{
    for(int index = 0; index < m_ships.count(); index++)
    {
        Ship *s = m_ships.at(index);
        if(s->pointInShip(inScenePoint))
            return index;
    }
    return -1;
}


void Universe::shipForPoint(const QPointF inScenePoint, ShipInfo & outShipInfo, QVector<Target> & outShipTargets)
{
    outShipInfo.id = 0;
    int index = shipIndexForPoint(inScenePoint);
    if(index < 0)
        return;
    outShipInfo = m_ships.at(index)->info();
    outShipTargets = m_ships.at(index)->targets();
}


void Universe::shipForPoint(const QPointF inScenePoint, ShipInfo & outShipInfo)
{
    outShipInfo.id = 0;
    int index = shipIndexForPoint(inScenePoint);
    if(index < 0)
        return;
    outShipInfo = m_ships.at(index)->info();
}


void Universe::shipForId(const uint inShipId, ShipInfo & outShipInfo, QVector<Target> & outShipTargets)
{
    outShipInfo.id = 0;
    for(Ship *ship : m_ships)
    {
        if(ship->id() == inShipId)
        {
            outShipInfo = ship->info();
            outShipTargets = ship->targets();
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
            // @fixme: we call really every other's ship deleteTargetShip() method, expensive!
            s->deleteTargetShip(inShipId);
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


void Universe::prepareStrategies()
{
    QList<IsleInfo> isleInfosPublic;
    QList<IsleInfo> isleInfosPrivate;

    for(Isle *isle : m_isles)
    {
        IsleInfo isleInfo = isle->info();
        if(isleInfo.owner == m_strategy->owner())
            isleInfosPrivate.append(isleInfo);
        else
            isleInfosPublic.append(isleInfo);

    }
    m_strategy->setIsles(isleInfosPublic, isleInfosPrivate);

    QList<ShipInfo> shipInfosPublic;
    QList<ShipInfo> shipInfosPrivate;

    for(Ship *ship : m_ships)
    {
        ShipInfo shipInfo = ship->info();
        if(shipInfo.owner == m_strategy->owner())
            shipInfosPrivate.append(shipInfo);
        else
        {
            if(shipInfo.posType == ShipPositionEnum::S_OCEAN)
                // just append if visible to everyone
                shipInfosPublic.append(shipInfo);
        }
    }
    m_strategy->setShips(shipInfosPublic, shipInfosPrivate);
}


void Universe::processStrategyCommands(const uint inOwner, const QList<StrategyCommand> inCommands)
{
    // @fixme: we really need to check, if strategy is cheating
    if(inCommands.isEmpty())
        return;
    for(StrategyCommand cmd : inCommands)
    {
        qDebug() << "cmd owner=" << inOwner <<  " wants to send ship " << cmd.shipId << " to id " << cmd.targetId;

        Ship *sourceShip;
        ShipInfo sourceShipInfo;
        bool foundShip = false;

        for(Ship *s : m_ships)
        {
            if(s->id() == cmd.shipId)
            {
                foundShip = true;
                sourceShip = s;
                sourceShipInfo = s->info();
                break;
            }
        }

        // check nonexistent ships
        if(! foundShip)
            return;

        // check for ownership
        if(sourceShipInfo.owner != inOwner)
            return;

        if(cmd.targetType == Target::TargetEnum::T_ISLE)
        {
            IsleInfo isleInfo;
            isleForId(cmd.targetId, isleInfo);
            // check valid isle
            if(isleInfo.id == 0)
                return;
            sourceShip->setTargetIsle(isleInfo.id, isleInfo.pos);
        }
        else if(cmd.targetType == Target::TargetEnum::T_SHIP)
        {
            ShipInfo shipInfo;
            shipForId(cmd.targetId, shipInfo);
            // check valid ship
            if(shipInfo.id == 0)
                return;
            sourceShip->setTargetShip(shipInfo.id, shipInfo.pos);
        }
        else // must be water
        {
            sourceShip->setTargetWater(cmd.pos);
        }

    }
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
        QVector<Target> targets;
        shipForPoint(scenePos, shipInfo, targets);
        if(shipInfo.id > 0)
        {   // human or enemy ship?
            if(shipInfo.owner == 1)
                emit sigShowInfoHumanShip(shipInfo, targets);
            else
                emit sigShowInfoShip(shipInfo);
        }
        else
            emit sigShowInfoWater();
    }
}


void Universe::slotUniverseViewClickedFinishShipTarget(QPointF scenePos, uint shipId)
{
    // find the source ship which needs new target
    Ship *sourceShip = 0;
    for(Ship *s : m_ships)
        if(s->id() == shipId)
        {
            sourceShip = s;
            break;
        }
    ShipInfo sourceShipInfo = sourceShip->info();

    IsleInfo isleInfo;
    isleForPoint(scenePos, isleInfo);
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
            sourceShip->setTargetShip(targetShipInfo.id, targetShipInfo.pos);
        }
        else
        {   // ship with id shipId wants target water at scenePos
            sourceShip->setTargetWater(scenePos);
        }
    }

    // shipInfo describes old state. if this ship was on an isle
    // then we have to show this isle again
    // this updates the graphics which shows, that this ship has a target
    if( (sourceShipInfo.posType == S_ONISLE) or (sourceShipInfo.posType == S_PATROL) )
    {
        // redraw isle info
        IsleInfo iInfo;
        isleForId(sourceShipInfo.isleId, iInfo);
        showHumanIsle(iInfo);
    }
}


void Universe::slotUniverseViewClickedFinishIsleTarget(QPointF scenePos, uint isleId)
{
    Isle *sourceIsle = 0;

    // find source isle
    for(Isle *s : m_isles)
        if(s->id() == isleId)
        {
            sourceIsle = s;
            break;
        }

    IsleInfo targetIsleInfo;
    isleForPoint(scenePos, targetIsleInfo);
    if(targetIsleInfo.id > 0)
    {   // we found a target isle
        if(targetIsleInfo.id == isleId)
            // source and target isle are the same, disable default target
            sourceIsle->setDefaultTargetNothing();
        else
            sourceIsle->setDefaultTargetIsle(targetIsleInfo.pos, targetIsleInfo.id);
    }
    else
    {   // just water (we do not allow ships as default targets)
        sourceIsle->setDefaultTargetWater(scenePos);
    }

    IsleInfo iInfo = sourceIsle->info();
    showHumanIsle(iInfo);
}
