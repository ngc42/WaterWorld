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
#include <QPair>


struct ExtendedShipInfo
{
    ShipInfo shipInfo;
    QVector<Target> targets;
};


struct ComputerMove
{
    // @fixme: "add to fleet" and "multitargets" (repeatable) are missing
    enum MoveTypeEnum { MT_ISLE_BUILD_SHIPTYPE,
                        MT_ISLE_ALL_SHIPS_TO_PATROL,
                        MT_SHIP_SET_TARGET_IMMEDIATELY, // clear targets, then set target
                        MT_SHIP_SET_TARGET,             // set target
                        MT_SHIP_SET_PATROL};
    MoveTypeEnum moveType;
    ShipTypeEnum shipTypeToBuild;   // sourceId must build this shiptype
    uint sourceId;  // ship or isle, the waterobject we talk about
    Target::TargetEnum targetType;
    uint targetId;      //  for ship or isle target, target isle if ship goes patrol
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
    void setShips(const QList<ShipInfo> inPublicShipInfos, const QList<ExtendedShipInfo> inPrivateShipInfos);

    /**
     * @brief nextRound - processes the strategy and returns a list of moves to the caller
     * @param outMoves  - a list of moves
     */
    void nextRound(QList<ComputerMove> & outMoves);


private:

    // isle (1) cares for another isle (2)
    QVector< QPair<uint, uint> > m_careList;

protected:

    // make a move and append it to the list
    void makeMoveIsleBuildShiptype(QList<ComputerMove> & outMoves, const uint inIsleId, const ShipTypeEnum inShipType);
    void makeMoveIsleAllShipsToPatrol(QList<ComputerMove> & outMoves, const uint inIsleId);
    void makeMoveShipSetTargetShip(QList<ComputerMove> & outMoves, const uint inSourceShipId, const uint inTargetShipId, bool inImmediately);
    void makeMoveShipSetTargetIsle(QList<ComputerMove> & outMoves, const uint inSourceShipId, const uint inTargetIsleId, bool inImmediately);
    void makeMoveShipSetTargetWater(QList<ComputerMove> & outMoves, const uint inSourceShipId, const QPointF inWaterPos, bool inImmediately);
    void makeMoveShipSetPatrol(QList<ComputerMove> & outMoves, const uint inSourceShipId, const uint inTargetIsleId);



    /**
     * @brief orderedUnsettledOrEnemyIsleFromCenter
     * @param inSetUnsettled - true: return a list of unsettled isles, false: enemy isles
     * @return
     */
    QList<uint> orderedUnsettledOrEnemyIsleFromCenter(const bool inSetUnsettled) const;

    void closestHomeIsleFromEnemyIsle(const uint inEnemyIsleId, IsleInfo & outHomeIsleInfo);

    QList<IsleInfo> m_publicIsles;
    QList<IsleInfo> m_privateIsles;

    uint m_homeIsleId;

    QList<ShipInfo> m_publicShips;
    QList<ExtendedShipInfo> m_privateShips;

    bool m_thereAreUnownedIsles;
    QPointF m_centerOfMyIsles;
};

#endif // COMPUTERPLAYER_H
