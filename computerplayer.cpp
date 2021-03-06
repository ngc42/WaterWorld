/* This File is part of WaterWorld. License is GNU GPL Version 3.
 * Please see https://github.com/ngc42/WaterWorld/blob/master/LICENSE for details.
 * WaterWorld is (C) 2016 by Eike Lange (eike@ngc42.de)
 */


#include <computerplayer.h>

#include <QPair>
#include <QSet>
#include <QDebug>


ComputerPlayer::ComputerPlayer(const uint inOwner)
    : Player(inOwner), m_homeIsleId(0)
{

}


void ComputerPlayer::setIsles(const QList<IsleInfo> inPublicIsleInfos, const QList<IsleInfo> inPrivateIsleInfos)
{
    m_publicIsles = inPublicIsleInfos;
    m_privateIsles = inPrivateIsleInfos;

    // test for unowned isles
    m_thereAreUnownedIsles = false;
    for(IsleInfo isleInfo : m_publicIsles)
        if(isleInfo.owner == Player::PLAYER_UNSETTLED)
        {
            m_thereAreUnownedIsles = true;
            break;
        }

    // center of isles
    m_centerOfMyIsles = {0, 0};
    for(IsleInfo isleInfo : m_privateIsles)
        m_centerOfMyIsles += isleInfo.pos;
    m_centerOfMyIsles /= m_privateIsles.count();

    // set the home isle
    // @fixme: we should do this once at start, not each time we call this method (every turn)
    if(m_homeIsleId == 0 and m_privateIsles.count() == 1)
    {
        m_homeIsleId = m_privateIsles.at(0).id;
    }
}


void ComputerPlayer::setShips(const QList<ShipInfo> inPublicShipInfos, const QList<ExtendedShipInfo> inPrivateShipInfos)
{
    m_publicShips = inPublicShipInfos;
    m_privateShips = inPrivateShipInfos;
}


void ComputerPlayer::nextRound(QList<ComputerMove> & outMoves)
{
    uint numMyIsles = m_privateIsles.count();

    if(numMyIsles == 0)
    {   // stage 0, no isles, we are close to lose the game

        // send every ship we have to out home isle
        for(ExtendedShipInfo esi : m_privateShips)
        {
            uint shipId = esi.shipInfo.id;
            makeMoveShipSetTargetIsle(outMoves, shipId, m_homeIsleId, true);
        }
        return;
    }
    if(m_thereAreUnownedIsles and numMyIsles == 1)
    {   // stage 1: try to get a second isle


        QList<uint> unownedIsles = orderedUnsettledOrEnemyIsleFromCenter(true);
        // remove from unowned isles every already set target
        for(ExtendedShipInfo esi : m_privateShips)
        {
            ShipInfo si = esi.shipInfo;
            if(si.hasTarget and si.shipType == ShipTypeEnum::ST_COLONY)
            {
                for(Target t : esi.targets)
                {
                    if(t.tType == Target::T_ISLE)
                    {
                        unownedIsles.removeOne(t.id);
                    }
                }
            }
        }

        // every new colony ship should now get a target
        if(unownedIsles.count() > 0)
        {
            IsleInfo isleInfo = m_privateIsles.at(0);
            // make sure, we build colony ships
            if(isleInfo.shipToBuild != ShipTypeEnum::ST_COLONY)
            {
                makeMoveIsleBuildShiptype(outMoves, isleInfo.id, ShipTypeEnum::ST_COLONY);
            }

            for(ExtendedShipInfo esi : m_privateShips)
            {
                ShipInfo si = esi.shipInfo;
                if(!si.hasTarget and si.shipType == ShipTypeEnum::ST_COLONY)
                {
                    makeMoveShipSetTargetIsle(outMoves, si.id, unownedIsles.at(0), true);
                    unownedIsles.removeFirst();
                }
                if(unownedIsles.count() < 1)
                    break;

            }
        }
        return;
    }
    // stage 3, shoot them down

    // check, if carelist is still up-to-date
    if(m_careList.count() > 0)
    {
        for(QPair<uint, uint> p : m_careList)
        {
            uint myIsle = p.first;
            uint targetIsle = p.second;
            // myisle is still our isle
            bool deleteEntry = true;
            bool targetIsOur = false;
            for(IsleInfo isleInfo : m_privateIsles)
            {
                if(isleInfo.id == myIsle)
                {
                    deleteEntry = false;
                }
                if(isleInfo.id == targetIsle)
                {
                    targetIsOur = true;
                    // target isle should build battleships!
                    if(isleInfo.shipToBuild != ShipTypeEnum::ST_BATTLESHIP)
                        makeMoveIsleBuildShiptype(outMoves, targetIsle, ShipTypeEnum::ST_BATTLESHIP);
                }
            }
            if(deleteEntry)
                // we have lost the source isle
                m_careList.removeOne(p);
            else
            {
                if(targetIsOur)
                {
                    // find out, if the new isle is protected enough
                    uint countBattleshipsOnTarge = 0;
                    for(ExtendedShipInfo esi : m_privateShips)
                    {
                        ShipInfo shipInfo = esi.shipInfo;
                        if(shipInfo.shipType == ShipTypeEnum::ST_BATTLESHIP and shipInfo.isleId == targetIsle)
                        {
                            if(shipInfo.posType == ShipPositionEnum::SP_ONISLE)
                            {
                                makeMoveShipSetPatrol(outMoves, shipInfo.id, targetIsle);
                                countBattleshipsOnTarge++;
                            }
                            else if(shipInfo.posType == ShipPositionEnum::SP_PATROL)
                            {
                                countBattleshipsOnTarge++;
                            }
                        }
                    }
                    // this is just a number of ships to protect the new isle
                    if(countBattleshipsOnTarge >= 6)
                    {   // protected enough
                        m_careList.removeOne(p);
                    }
                }
                else    // the target is not our
                {
                    bool needColonyForTarget;
                    for(IsleInfo targetIsleInfo : m_publicIsles)
                    {
                        if(targetIsleInfo.id == targetIsle)
                        {
                            if(targetIsleInfo.owner == 0)
                                needColonyForTarget = true;
                            break;
                        }
                    }

                    if(needColonyForTarget)
                    {
                        // colony underway?
                        bool colonyUnderway = false;
                        for(ExtendedShipInfo esi : m_privateShips)
                        {
                            ShipInfo shipInfo = esi.shipInfo;
                            if(shipInfo.shipType == ShipTypeEnum::ST_COLONY)
                            {
                               if(shipInfo.isleId == myIsle and shipInfo.posType == ShipPositionEnum::SP_ONISLE)
                               {
                                   // send it to target
                                   makeMoveShipSetTargetIsle(outMoves, shipInfo.id, targetIsle, true);
                                   colonyUnderway = true;
                                   break;
                               }
                               if(shipInfo.posType == ShipPositionEnum::SP_OCEAN)
                               {
                                   for(Target t : esi.targets)
                                   {
                                       if(t.tType == Target::T_ISLE and t.id == targetIsle)
                                       {
                                           colonyUnderway = true;
                                           break;
                                       }
                                   }
                               }
                            }
                            if(colonyUnderway)
                                break;   // break outer for-loop
                        }
                        // make sure we build a colony ship on source isle if we have not
                        // or a Battleship, if we already have
                        for(IsleInfo ii : m_privateIsles)
                        {
                            if(ii.id == myIsle)
                            {
                                if((!colonyUnderway) and ii.shipToBuild != ShipTypeEnum::ST_COLONY)
                                    makeMoveIsleBuildShiptype(outMoves, ii.id, ShipTypeEnum::ST_COLONY);
                                else if (colonyUnderway and ii.shipToBuild != ShipTypeEnum::ST_BATTLESHIP)
                                    makeMoveIsleBuildShiptype(outMoves, ii.id, ShipTypeEnum::ST_BATTLESHIP);
                                break;
                            }
                        }
                    }
                    else
                    {
                        // need battleships for target
                        // make sure we build battle ships
                        for(IsleInfo ii : m_privateIsles)
                        {
                            if(ii.id == myIsle)
                            {
                                if (ii.shipToBuild != ShipTypeEnum::ST_BATTLESHIP)
                                    makeMoveIsleBuildShiptype(outMoves, ii.id, ShipTypeEnum::ST_BATTLESHIP);
                                break;
                            }
                        }
                        // send all battleships on source isle to target isle
                        for(ExtendedShipInfo esi : m_privateShips)
                        {
                            ShipInfo shipInfo = esi.shipInfo;
                            if(shipInfo.isleId == myIsle and
                               (shipInfo.posType == ShipPositionEnum::SP_ONISLE or shipInfo.posType == ShipPositionEnum::SP_PATROL) and
                               shipInfo.shipType == ShipTypeEnum::ST_BATTLESHIP)
                                makeMoveShipSetTargetIsle(outMoves, shipInfo.id, targetIsle, true);
                        }
                    }

                }
            }
        }
    }

    // fill up carelist, one at a time
    QList<uint> enemyIsles = orderedUnsettledOrEnemyIsleFromCenter(false);

    // remove all enemy isles we already care about
    for(QPair<uint, uint> p : m_careList)
    {
        uint targetIsle = p.second;
        enemyIsles.removeOne(targetIsle);
    }

    if(enemyIsles.count() == 0)
        return;
    uint newTargetIsle = enemyIsles.at(0);

    for(IsleInfo isleInfo : m_privateIsles)
    {
        bool foundPair = false;

        for(QPair<uint, uint> p : m_careList)
        {
            uint myIsle = p.first;
            if(myIsle == isleInfo.id)
            {
                foundPair = true;
                break;
            }
        }

        if(! foundPair)
        {
            // isleInfo.id does not care for an target
            QPair<uint, uint> p(isleInfo.id, newTargetIsle);

            // unsettled or enemy?
            for(IsleInfo targetIsleInfo : m_publicIsles)
            {
                if(targetIsleInfo.id == newTargetIsle)
                {
                    if(targetIsleInfo.owner == 0)
                    {
                        makeMoveIsleBuildShiptype(outMoves, isleInfo.id, ShipTypeEnum::ST_COLONY);
                    }
                    else
                    {   // enemy isle
                        makeMoveIsleBuildShiptype(outMoves, isleInfo.id, ShipTypeEnum::ST_BATTLESHIP);

                    }
                    m_careList.append(p);
                    break;
                }
            }
            break;
        }

    }



}


void ComputerPlayer::makeMoveIsleBuildShiptype(QList<ComputerMove> & outMoves, const uint inIsleId, const ShipTypeEnum inShipType)
{
    ComputerMove cmd;
    cmd.moveType = ComputerMove::MT_ISLE_BUILD_SHIPTYPE;
    cmd.sourceId = inIsleId;
    cmd.shipTypeToBuild = inShipType;
    outMoves.append(cmd);
}


void ComputerPlayer::makeMoveIsleAllShipsToPatrol(QList<ComputerMove> & outMoves, const uint inIsleId)
{
    ComputerMove cmd;
    cmd.moveType = ComputerMove::MT_ISLE_ALL_SHIPS_TO_PATROL;
    cmd.sourceId = inIsleId;
    outMoves.append(cmd);
}


void ComputerPlayer::makeMoveShipSetTargetShip(QList<ComputerMove> & outMoves, const uint inSourceShipId, const uint inTargetShipId, bool inImmediately)
{
    ComputerMove cmd;
    cmd.moveType = inImmediately ? ComputerMove::MT_SHIP_SET_TARGET_IMMEDIATELY : ComputerMove::MT_SHIP_SET_TARGET;
    cmd.sourceId = inSourceShipId;
    cmd.targetType = Target::T_SHIP;
    cmd.targetId = inTargetShipId;
    outMoves.append(cmd);
}


void ComputerPlayer::makeMoveShipSetTargetIsle(QList<ComputerMove> & outMoves, const uint inSourceShipId, const uint inTargetIsleId, bool inImmediately)
{
    ComputerMove cmd;
    cmd.moveType = inImmediately ? ComputerMove::MT_SHIP_SET_TARGET_IMMEDIATELY : ComputerMove::MT_SHIP_SET_TARGET;
    cmd.sourceId = inSourceShipId;
    cmd.targetType = Target::T_ISLE;
    cmd.targetId = inTargetIsleId;
    outMoves.append(cmd);
}


void ComputerPlayer::makeMoveShipSetTargetWater(QList<ComputerMove> & outMoves, const uint inSourceShipId, const QPointF inWaterPos, bool inImmediately)
{
    ComputerMove cmd;
    cmd.moveType = inImmediately ? ComputerMove::MT_SHIP_SET_TARGET_IMMEDIATELY : ComputerMove::MT_SHIP_SET_TARGET;
    cmd.sourceId = inSourceShipId;
    cmd.targetType = Target::T_WATER;
    cmd.pos = inWaterPos;
    outMoves.append(cmd);
}


void ComputerPlayer::makeMoveShipSetPatrol(QList<ComputerMove> & outMoves, const uint inSourceShipId, const uint inTargetIsleId)
{
    ComputerMove cmd;
    cmd.moveType = ComputerMove::MT_SHIP_SET_PATROL;
    cmd.sourceId = inSourceShipId;
    cmd.targetId = inTargetIsleId;
    outMoves.append(cmd);
}


QList<uint> ComputerPlayer::orderedUnsettledOrEnemyIsleFromCenter(const bool inSetUnsettled) const
{
    QList< QPair<qreal, uint> > uList;

    for(IsleInfo isleInfo : m_publicIsles)
    {
        if( (inSetUnsettled and isleInfo.owner == Player::PLAYER_UNSETTLED) or (!inSetUnsettled and isleInfo.owner  != owner()) )
        {
            qreal dx = isleInfo.pos.x() - m_centerOfMyIsles.x();
            qreal dy = isleInfo.pos.y() - m_centerOfMyIsles.y();
            qreal dist = dx * dx + dy * dy; // dist means square dist here, thats ok

            QPair<qreal, uint> p(dist, isleInfo.id);

            // insert p sorted
            if(uList.isEmpty() or uList.at(uList.count()-1).first < dist)
                uList.append( p );
            else
            {
                int index =  0;
                while( index < uList.count())
                {
                    if(uList.at(index).first > dist)
                    {
                        uList.insert(index, p);
                        break;
                    }
                    index++;
                }
            }
        }
    }

    // return the ordered list to caller, but without distance information
    QList<uint> unsettledList;
    for(QPair<qreal, uint> item : uList)
        unsettledList.append(item.second);
    return unsettledList;
}


void ComputerPlayer::closestHomeIsleFromEnemyIsle(const uint inEnemyIsleId, IsleInfo & outHomeIsleInfo)
{
    QPointF enemyPos(0, 0);
    for(IsleInfo ii : m_publicIsles)
    {
        if(ii.id == inEnemyIsleId)
        {
            enemyPos = ii.pos;
            break;
        }
    }
    float min_dist = -1;
    outHomeIsleInfo.id = 0;
    for(IsleInfo ii : m_privateIsles)
    {
        QPointF homePos = ii.pos;
        float dist = (homePos - enemyPos).manhattanLength();
        if(dist < min_dist or min_dist < 0)
        {
            min_dist = dist;
            outHomeIsleInfo = ii;
        }
    }
}
