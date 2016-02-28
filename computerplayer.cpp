/* This File is part of WaterWorld. License is GNU GPL Version 3.
 * Please see https://github.com/ngc42/WaterWorld/blob/master/LICENSE for details.
 * WaterWorld is (C) 2016 by Eike Lange (eike@ngc42.de)
 */


#include <computerplayer.h>

#include <QPair>
#include <QSet>
#include <QDebug>


ComputerPlayer::ComputerPlayer(const uint inOwner)
    : Player(inOwner)
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

        return;
    }
    if(numMyIsles == 1)
    {   // stage 1: try to get a second isle

        return;
    }
    if(m_thereAreUnownedIsles and numMyIsles < 10)
    {   // stage 2: try to get 10 or more isles

        return;
    }

    // stage 3, shoot them down

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


void ComputerPlayer::makeMoveShipSetTargetShip(QList<ComputerMove> & outMoves, const uint inSourceShipId, const uint inTargetShipId)
{
    ComputerMove cmd;
    cmd.moveType = ComputerMove::MT_SHIP_SET_TARGET;
    cmd.sourceId = inSourceShipId;
    cmd.targetType = Target::T_SHIP;
    cmd.targetId = inTargetShipId;
    outMoves.append(cmd);
}


void ComputerPlayer::makeMoveShipSetTargetIsle(QList<ComputerMove> & outMoves, const uint inSourceShipId, const uint inTargetIsleId)
{
    ComputerMove cmd;
    cmd.moveType = ComputerMove::MT_SHIP_SET_TARGET;
    cmd.sourceId = inSourceShipId;
    cmd.targetType = Target::T_ISLE;
    cmd.targetId = inTargetIsleId;
    outMoves.append(cmd);
}


void ComputerPlayer::makeMoveShipSetTargetWater(QList<ComputerMove> & outMoves, const uint inSourceShipId, const QPointF inWaterPos)
{
    ComputerMove cmd;
    cmd.moveType = ComputerMove::MT_SHIP_SET_TARGET;
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
