/* This File is part of WaterWorld. License is GNU GPL Version 3.
 * Please see https://github.com/ngc42/WaterWorld/blob/master/LICENSE for details.
 * WaterWorld is (C) 2016 by Eike Lange (eike@ngc42.de)
 */


#include "computerplayer.h"
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


void ComputerPlayer::setShips(const QList<ShipInfo> inPublicShipInfos, const QList<ShipInfo> inPrivateShipInfos)
{
    m_publicShips = inPublicShipInfos;
    m_privateShips = inPrivateShipInfos;
}


void ComputerPlayer::nextRound(QList<ComputerMove> & outMoves)
{
    QSet<uint> shipsAlreadyProcessed;

    if(m_thereAreUnownedIsles)
    {
        QList<uint> unownedIds = orderedUnsettledOrEnemyIsleFromCenter(true);
        for(uint isleId : unownedIds)
        {
            bool shipsAvailable = false;
            for(ShipInfo shipInfo : m_privateShips)
            {
                if(shipInfo.posType == ShipPositionEnum::S_ONISLE and !shipsAlreadyProcessed.contains(shipInfo.id))
                {
                    shipsAlreadyProcessed.insert(shipInfo.id);
                    shipsAvailable = true;
                    ComputerMove cmd;
                    cmd.owner = owner();
                    cmd.shipId = shipInfo.id;
                    cmd.targetType = Target::TargetEnum::T_ISLE;
                    cmd.targetId = isleId;
                    outMoves.append(cmd);

                    break;
                }
            }
            if(! shipsAvailable)
                break;
        }
    }
    else
    {
        QList<uint> enemyIds = orderedUnsettledOrEnemyIsleFromCenter(false);
        for(uint isleId : enemyIds )
        {
            int sendNumShips = 0;
            for(ShipInfo shipInfo : m_privateShips)
            {
                if(shipInfo.posType == ShipPositionEnum::S_ONISLE and !shipsAlreadyProcessed.contains(shipInfo.id))
                {
                    shipsAlreadyProcessed.insert(shipInfo.id);
                    ComputerMove cmd;
                    cmd.owner = owner();
                    cmd.shipId = shipInfo.id;
                    cmd.targetType = Target::TargetEnum::T_ISLE;
                    cmd.targetId = isleId;
                    outMoves.append(cmd);

                    sendNumShips++;
                }
                if(sendNumShips > 10)
                    break;
            }
            if(sendNumShips > 10)
                break;
        }
    }
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
