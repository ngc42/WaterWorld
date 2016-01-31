/* This File is part of WaterWorld. License is GNU GPL Version 3.
 * Please see https://github.com/ngc42/WaterWorld/blob/master/LICENSE for details.
 * WaterWorld is (C) 2016 by Eike Lange (eike@ngc42.de)
 */


#ifndef STRATEGY_H
#define STRATEGY_H


#include <isle.h>
#include <ship.h>
#include <QList>


struct StrategyCommand
{
    uint owner;
    uint shipId;
    Target::TargetEnum targetType;
    uint targetId;      //  for ship or isle target
    QPointF pos;        // for water target
};


class Strategy
{
public:
    Strategy(const uint inOwner);

    uint owner() const {return m_owner; }

    /**
     * @brief setIsles
     * @param inPublicIsleInfos - list with islesInfos, not owned by m_owner
     * @param inPrivateIsleInfos - list with islesInfos, owned by m_owner
     */
    void setIsles(const QList<IsleInfo> inPublicIsleInfos, const QList<IsleInfo> inPrivateIsleInfos);

    /**
     * @brief setShips
     * @param inPublicShipInfos - list of public visible enemy ships (ShipPositionEnum::S_OCEAN)
     * @param inPrivateShipInfos - list of own ships
     */
    void setShips(const QList<ShipInfo> inPublicShipInfos, const QList<ShipInfo> inPrivateShipInfos);

    void nextRound(QList<StrategyCommand> & outCommands);

protected:
    QList<uint> orderedUnsettledOrEnemyIsleFromCenter(const bool inSetUnsettled) const;


    uint m_owner;   // this strategy belongs to this owner

    QList<IsleInfo> m_publicIsles;
    QList<IsleInfo> m_privateIsles;

    QList<ShipInfo> m_publicShips;
    QList<ShipInfo> m_privateShips;

    bool m_thereAreUnownedIsles;
    QPointF m_centerOfMyIsles;
};

#endif // STRATEGY_H
