/* This File is part of WaterWorld. License is GNU GPL Version 3.
 * Please see https://github.com/ngc42/WaterWorld/blob/master/LICENSE for details.
 * WaterWorld is (C) 2016 by Eike Lange (eike@ngc42.de)
 */


#ifndef COMPUTERPLAYER_H
#define COMPUTERPLAYER_H


#include <player.h>
#include <isle.h>
#include <ship.h>
#include <QColor>
#include <QList>


struct ComputerMove
{
    uint owner;
    uint shipId;
    Target::TargetEnum targetType;
    uint targetId;      //  for ship or isle target
    QPointF pos;        // for water target
};


class ComputerPlayer : public Player
{
public:
    ComputerPlayer(uint inOwner);

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

    /**
     * @brief nextRound - processes the strategy and returns a list of moves to the caller
     * @param outMoves  - a list of moves
     */
    void nextRound(QList<ComputerMove> & outMoves);

private:

protected:
    /**
     * @brief orderedUnsettledOrEnemyIsleFromCenter
     * @param inSetUnsettled - true: return a list of unsettled isles, false: enemy isles
     * @return
     */
    QList<uint> orderedUnsettledOrEnemyIsleFromCenter(const bool inSetUnsettled) const;

    QList<IsleInfo> m_publicIsles;
    QList<IsleInfo> m_privateIsles;

    QList<ShipInfo> m_publicShips;
    QList<ShipInfo> m_privateShips;

    bool m_thereAreUnownedIsles;
    QPointF m_centerOfMyIsles;
};

#endif // COMPUTERPLAYER_H
