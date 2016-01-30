/* This File is part of WaterWorld. License is GNU GPL Version 3.
 * Please see https://github.com/ngc42/WaterWorld/blob/master/LICENSE for details.
 * WaterWorld is (C) 2016 by Eike Lange (eike@ngc42.de)
 */


#include "strategy.h"
#include <QPair>
#include <QSet>
#include <QDebug>


Strategy::Strategy(const uint inOwner)
    : m_owner(inOwner), m_thereAreUnownedIsles(false)
{

}


void Strategy::setIsles(const QList<IsleInfo> inPublicIsleInfos, const QList<IsleInfo> inPrivateIsleInfos)
{
    m_publicIsles = inPublicIsleInfos;
    m_privateIsles = inPrivateIsleInfos;

    // test for unowned isles
    m_thereAreUnownedIsles = false;
    for(IsleInfo isleInfo : m_publicIsles)
        if(isleInfo.owner == 0)
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


void Strategy::setShips(const QList<ShipInfo> inPublicShipInfos, const QList<ShipInfo> inPrivateShipInfos)
{
    m_publicShips = inPublicShipInfos;
    m_privateShips = inPrivateShipInfos;
}


void Strategy::nextRound(QList<StrategyCommand> & outCommands)
{
    if(m_thereAreUnownedIsles)
    {
        QSet<uint> shipsAlreadyProcessed;
        QList<uint> unownedIds = orderedUnsettledIsleFromCenter();
        for(uint isleId : unownedIds)
        {
            bool shipsAvailable = false;
            for(ShipInfo shipInfo : m_privateShips)
            {
                if(shipInfo.posType == ShipPositionEnum::S_ONISLE and !shipsAlreadyProcessed.contains(shipInfo.id))
                {
                    shipsAlreadyProcessed.insert(shipInfo.id);
                    shipsAvailable = true;
                    StrategyCommand cmd;
                    cmd.owner = owner();
                    cmd.shipId = shipInfo.id;
                    cmd.targetType = Target::TargetEnum::T_ISLE;
                    cmd.targetId = isleId;
                    outCommands.append(cmd);

                    break;
                }
            }
            if(! shipsAvailable)
                break;
        }
    }
}


QList<uint> Strategy::orderedUnsettledIsleFromCenter() const
{
    QList< QPair<qreal, uint> > uList;

    for(IsleInfo isleInfo : m_publicIsles)
    {
        if(isleInfo.owner == 0)
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
