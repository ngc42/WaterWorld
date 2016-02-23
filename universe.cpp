/* This File is part of WaterWorld. License is GNU GPL Version 3.
 * Please see https://github.com/ngc42/WaterWorld/blob/master/LICENSE for details.
 * WaterWorld is (C) 2016 by Eike Lange (eike@ngc42.de)
 */


#include <universe.h>
#include <player.h>

#include <stdlib.h>     /* srand, rand */
#include <time.h>
#include <QBrush>
#include <QDebug>


Universe::Universe(QObject *inParent, UniverseScene *& inOutUniverseScene, const qreal inUniverseWidth,
                   const qreal inUniverseHeight, const uint inNumIsles, const uint numEnemies)
    : QObject(inParent), m_lastInsertedId(10)
{
    createIsles(inUniverseWidth, inUniverseHeight, inNumIsles);

    // add created isles to the scene
    for(Isle *isle : m_isles)
    {
        inOutUniverseScene->addItem(isle->shape());
    }

    for(uint i = 0; i < numEnemies; i++)
    {
        m_computerPlayers.append( new ComputerPlayer(Player::PLAYER_ENEMY_BASE + i) );
    }

    Isle *isle = m_isles.at(0);
    isle->setOwner(Player::PLAYER_HUMAN, Player::colorForOwner(Player::PLAYER_HUMAN));

    for(int i = 0; i < m_computerPlayers.count(); i++)
    {
        // if there are more enemies than isles, the enemies don't get an own isle
        // @fixme: should get marked as "dead".
        uint idx = i + 1;   // prevent warning: comparison signed/unsigned
        if( idx > inNumIsles )
            break;
        uint owner = m_computerPlayers.at(i)->owner();
        isle = m_isles.at(1 + i);
        isle->setOwner(owner, Player::colorForOwner(owner));
    }
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
    int shipIndex = shipIndexForId(inShipId);
    if(shipIndex >= 0)
    {
        Ship *s = m_ships[shipIndex];
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
    }
}


QPointF Universe::shipPosById(const uint inShipId)
{
    int shipIndex = shipIndexForId(inShipId);
    if(shipIndex >= 0)
        return m_ships.at(shipIndex)->pos();
    return QPointF(0, 0);
}


float Universe::shipTechById(const uint inShipId)
{
    int shipIndex = shipIndexForId(inShipId);
    if(shipIndex >= 0)
        return m_ships.at(shipIndex)->info().technology;
    return 0.0f;
}


void Universe::shipForId(const uint inShipId, ShipInfo & outShipInfo)
{
    outShipInfo.id = 0;
    int index = shipIndexForId(inShipId);
    if(index >= 0)
        outShipInfo = m_ships.at(index)->info();
}


void Universe::shipForId(const uint inShipId, ShipInfo & outShipInfo, QVector<Target> & outShipTargets)
{
    outShipInfo.id = 0;
    int index = shipIndexForId(inShipId);
    if(index < 0)
        return;
    outShipInfo = m_ships.at(index)->info();
    outShipTargets = m_ships.at(index)->targets();
}


void Universe::isleForId(const uint inIsleId, IsleInfo & outIsleInfo)
{
    outIsleInfo.id = 0;
    int isleIndex = isleIndexForId(inIsleId);
    if(isleIndex >= -1)
        outIsleInfo = m_isles.at(isleIndex)->info();
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


void Universe::removeDefaultIsleTarget(const uint inIsleId)
{
    int isleIndex = isleIndexForId(inIsleId);
    if(isleIndex >= 0)
        m_isles[isleIndex]->setDefaultTargetNothing();
}


void Universe::removeAllShipTargets(const uint inShipId)
{
    int index = shipIndexForId(inShipId);
    if(index < 0)
        return;
    m_ships[index]->removeTargets();
}


void Universe::removeShipTargetByIndex(const uint inShipId, const int inIndex)
{
    int index = shipIndexForId(inShipId);
    if(index < 0)
        return;
    m_ships[index]->removeTargetByIndex(inIndex);
}


void Universe::shipSetCycleTargets(const uint inShipId, const uint inCycle)
{
    int index = shipIndexForId(inShipId);
    if(index < 0)
        return;
    m_ships[index]->setCycleTargets(inCycle);
}


void Universe::nextRound(UniverseScene *& inOutUniverseScene)
{
    qInfo() << "BEGIN NEXTROUND ==================";
    prepareStrategies();
    for(ComputerPlayer *player : m_computerPlayers)
    {
        QList<ComputerMove> computerMoves;
        player->nextRound(computerMoves);
        processStrategyCommands(player->owner(), computerMoves);
    }

    for(Isle *isle : m_isles)
    {
        if(isle->nextRound())
        {
            createShipOnIsle(inOutUniverseScene, isle->info());
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

                if(isleInfo.owner == Player::PLAYER_UNSETTLED)
                {   // isle has no inhabitants

                    // @fixme: colony ship MUST land on isle first
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
                                int isleIndex = isleIndexForId(target.id);
                                if(isleIndex >= 0)
                                    m_isles[isleIndex]->setMaxTechnology(local_tech_max);
                            }
                        }
                    }
                }
            }
            else if(target.tType == Target::TargetEnum::T_SHIP)
            {   // fight or rendez vous

                // find the other ship
                Ship *otherShip;

                int shipIndex = shipIndexForId(target.id);
                if(shipIndex >= 0)
                    otherShip = m_ships[shipIndex];

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
                int shipIndex = shipIndexForId(t.id);
                if(shipIndex >= 0)
                {
                    targetIsAlive = true;
                    QPointF newPos = m_ships.at(shipIndex)->info().pos;
                    updateShip->updateTargetPos(t.id, newPos);
                }

                if(! targetIsAlive)
                {
                    // the target ship was deleted, so delete the target too
                    updateShip->removeTargetShip(t.id);
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
        if(isleInfo.owner == Player::PLAYER_HUMAN)
        {
            // human
            showHumanIsle(isleInfo);
        }
        else
            emit sigShowInfoIsle(isleInfo);
    }
    else if(inPage == InfoscreenPage::PAGE_HUMAN_ISLE)
    {
        IsleInfo isleInfo;
        isleForId(inIsleInfo.id, isleInfo);
        // owner may have changed
        if(isleInfo.owner == Player::PLAYER_HUMAN)
            showHumanIsle(isleInfo);
        else
            emit sigShowInfoIsle(isleInfo);
    }
    else if(inPage == InfoscreenPage::PAGE_SHIP)
    {
        ShipInfo shipInfo;
        shipForId(inShipInfo.id, shipInfo);
        emit sigShowInfoShip(shipInfo);
    }
    else if(inPage == InfoscreenPage::PAGE_WATER)
        emit sigShowInfoWater();
    else if(inPage == InfoscreenPage::PAGE_HUMAN_SHIP)
    {
        ShipInfo shipInfo;
        QVector<Target> targetList;
        shipForId(inShipInfo.id, shipInfo, targetList);
        if(shipInfo.owner == Player::PLAYER_HUMAN)
            emit sigShowInfoHumanShip(shipInfo, targetList);
        else
        {   // ship is dead or captured
            if(shipInfo.id == 0)
            {
                // ship is dead
                emit sigShowInfoWater();
            }
            else
            {
                // captured
                emit sigShowInfoShip(shipInfo);
            }
        }
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

        Isle *isle = new Isle(m_lastInsertedId++, Player::PLAYER_UNSETTLED, QPointF(x, y), Player::colorForOwner(Player::PLAYER_UNSETTLED));
        m_isles.append(isle);
    }
}


void Universe::createShipOnIsle(UniverseScene *& inOutUniverseScene, const IsleInfo isleInfo)
{

    Ship *s = new Ship(inOutUniverseScene, isleInfo.shipToBuild, m_lastInsertedId++, isleInfo.owner,
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
}


void Universe::shipFightShip(Ship *& inOutAttacker, Ship *& inOutDefender)
{
    ShipInfo info1 = inOutAttacker->info();
    ShipInfo info2 = inOutDefender->info();

    if(info1.posType == ShipPositionEnum::S_TRASH)
        return;
    if(info2.posType == ShipPositionEnum::S_TRASH)
        return;


    // Battleship (attack) against colony or courier
    if(info1.shipType == ShipTypeEnum::ST_BATTLESHIP and
       (info2.shipType == ShipTypeEnum::ST_COLONY or info2.shipType == ShipTypeEnum::ST_COURIER))
        inOutDefender->addDamage(2.0);

    // Battleship (defender) against colony or courier
    if(info2.shipType == ShipTypeEnum::ST_BATTLESHIP and
       (info1.shipType == ShipTypeEnum::ST_COLONY or info1.shipType == ShipTypeEnum::ST_COURIER))
        inOutAttacker->addDamage(2.0);

    // both ships are courier or colony -> nothing happens
    // @fixme: that way, diffent player can stay in the same orbit ???
    if((info1.shipType == ShipTypeEnum::ST_COLONY or info1.shipType == ShipTypeEnum::ST_COURIER) and
        (info2.shipType == ShipTypeEnum::ST_COLONY or info2.shipType == ShipTypeEnum::ST_COURIER))
        return;

    // the rest is about Battleships and/or fleets
    // @fixme: make sure, fleets have something like a force !!!

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
    // @fixme: need to select for ship types
    // @fixme: need to check, if there are other ships in orbit of unowned isles

    ShipInfo info1 = inOutAttacker->info();
    IsleInfo info2;
    isleForId(inIsleId, info2);
    if(info1.posType == ShipPositionEnum::S_TRASH)
        return false;
    if(info2.owner == Player::PLAYER_UNSETTLED or info2.owner == info1.owner)
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
            setIsleOwnerById(info2.id, Player::PLAYER_UNSETTLED, Player::colorForOwner(Player::PLAYER_UNSETTLED));
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
            setIsleOwnerById(info2.id, Player::PLAYER_UNSETTLED, Player::colorForOwner(Player::PLAYER_UNSETTLED));
    }
    else
    {   // magic, if this happens: both forces are the same
        inOutAttacker->setPositionType(ShipPositionEnum::S_TRASH);
        setIsleOwnerById(info2.id, Player::PLAYER_UNSETTLED, Player::colorForOwner(Player::PLAYER_UNSETTLED));
    }

    return false;   // ship has lost
}


void Universe::shipLandOnIsle(Ship *& inOutShipToLand)
{
    // @fixme: are we allowed to land on this isle?

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

    int isleIndex = isleIndexForId(targetInfo.id);
    if(isleIndex >= 0)
    {
        targetIsle = m_isles[isleIndex];
        isleInfo = targetIsle->info();
    }


    if(isleInfo.id == 0 or shipInfo.owner != isleInfo.owner)
    {   // just to get sure
        qDebug() << "ERR 2 in Universe::shipLandOnIsle()";
        return;
    }

    // transfer technology to the isle
    targetIsle->setMaxTechnology(shipInfo.technology);

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


int Universe::isleIndexForId(const uint inIsleId) const
{
    int upperIndex = m_isles.count() - 1;
    int midIndex = upperIndex / 2;
    int lowerIndex = 0;
    bool found = false;
    while(! found)
    {
        uint id = m_isles.at(midIndex)->id();
        if(id < inIsleId)
        {
            lowerIndex = midIndex + 1;
            midIndex = (lowerIndex + upperIndex) / 2;
        }
        else if(id > inIsleId)
        {
            upperIndex = midIndex - 1;
            midIndex = (lowerIndex + upperIndex) / 2;
        }
        else
        {   // equal
            found = true;
        }
        if(lowerIndex > upperIndex)
            break;
    }
    if(found)
        return midIndex;
    else
    {
        // this did not happen during lots of tests, so everything looks good here
        Q_ASSERT(false);
        return -1;
    }
}


void Universe::setIsleOwnerById(const uint inIsleId, const uint inNewOwner, const QColor inNewColor)
{
    int isleIndex = isleIndexForId(inIsleId);
    if(isleIndex >= 0)
        m_isles[isleIndex]->setOwner(inNewOwner, inNewColor);
}


void Universe::setIslePopulationById(const uint inIsleId, const float inNewPopulation)
{
    int isleIndex = isleIndexForId(inIsleId);
    if(isleIndex >= 0)
        m_isles[isleIndex]->setPopulation(inNewPopulation);
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


int Universe::shipIndexForId(const uint inShipId) const
{
    int upperIndex = m_ships.count() - 1;
    int midIndex = upperIndex / 2;
    int lowerIndex = 0;
    bool found = false;
    while(! found)
    {
        uint id = m_ships.at(midIndex)->id();
        if(id < inShipId)
        {
            lowerIndex = midIndex + 1;
            midIndex = (lowerIndex + upperIndex) / 2;
        }
        else if(id > inShipId)
        {
            upperIndex = midIndex - 1;
            midIndex = (lowerIndex + upperIndex) / 2;
        }
        else
        {   // equal
            found = true;
        }
        if(lowerIndex > upperIndex)
            break;
    }
    if(found)
        return midIndex;
    else
    {
        // this did not happen during lots of tests, so everything looks good here
        Q_ASSERT(false);
        return -1;
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
            s->removeTargetShip(inShipId);
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
        if(info.owner == Player::PLAYER_HUMAN and
                (info.posType != ShipPositionEnum::S_OCEAN) and
                info.isleId == inIsleInfo.id)
            sList.append(info);
    }

    emit sigShowInfoHumanIsle(inIsleInfo, sList);
}


void Universe::prepareStrategies()
{
    for(ComputerPlayer *player : m_computerPlayers)
    {
        QList<IsleInfo> isleInfosPublic;
        QList<IsleInfo> isleInfosPrivate;

        for(Isle *isle : m_isles)
        {
            IsleInfo isleInfo = isle->info();
            if(isleInfo.owner == player->owner())
                isleInfosPrivate.append(isleInfo);
            else
                isleInfosPublic.append(isleInfo);

        }
        player->setIsles(isleInfosPublic, isleInfosPrivate);

        QList<ShipInfo> shipInfosPublic;
        QList<ShipInfo> shipInfosPrivate;

        for(Ship *ship : m_ships)
        {
            ShipInfo shipInfo = ship->info();
            if(shipInfo.owner == player->owner())
                shipInfosPrivate.append(shipInfo);
            else
            {
                if(shipInfo.posType == ShipPositionEnum::S_OCEAN)
                    // just append if visible to everyone
                    shipInfosPublic.append(shipInfo);
            }
        }
        player->setShips(shipInfosPublic, shipInfosPrivate);
    }
}


void Universe::processStrategyCommands(const uint inOwner, const QList<ComputerMove> inComputerMoves)
{
    // @fixme: we really need to check, if strategy is cheating
    if(inComputerMoves.isEmpty())
        return;
    for(ComputerMove cmd : inComputerMoves)
    {
        qDebug() << "cmd owner=" << inOwner <<  " wants to send ship " << cmd.shipId << " to id " << cmd.targetId;

        int shipIndex = shipIndexForId(cmd.shipId);

        // check nonexistent ships
        if(shipIndex < 0)
            return;
        Ship *sourceShip = m_ships[shipIndex];
        ShipInfo sourceShipInfo = sourceShip->info();

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
        if(isleInfo.owner == Player::PLAYER_HUMAN)
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
            if(shipInfo.owner == Player::PLAYER_HUMAN)
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
    int shipIndex = shipIndexForId(shipId);
    Ship *sourceShip = m_ships[shipIndex];

    Q_ASSERT(sourceShip);

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

    // call the infoscreen again, as the reason for a new target is one of 2 infoscreen-buttons
    emit sigRecallInfoscreen();
}


void Universe::slotUniverseViewClickedFinishIsleTarget(QPointF scenePos, uint isleId)
{
    Isle *sourceIsle = 0;

    // find source isle
    int isleIndex = isleIndexForId(isleId);
    if(isleIndex >= 0)
        sourceIsle = m_isles[isleIndex];

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
