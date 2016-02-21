/* This File is part of WaterWorld. License is GNU GPL Version 3.
 * Please see https://github.com/ngc42/WaterWorld/blob/master/LICENSE for details.
 * WaterWorld is (C) 2016 by Eike Lange (eike@ngc42.de)
 */


#ifndef UNIVERSE_H
#define UNIVERSE_H


#include <isle.h>
#include <ship.h>
#include <computerplayer.h>
#include <universescene.h>
#include <QVector>
#include <QList>
#include <QPointF>


enum InfoscreenPage {PAGE_NOTHING = 0, PAGE_WATER = 1, PAGE_ISLE = 2, PAGE_HUMAN_ISLE = 3, PAGE_SHIP = 4, PAGE_HUMAN_SHIP = 5};


class Universe : public QObject
{
    Q_OBJECT

public:
    explicit Universe(QObject *inParent, UniverseScene *& inOutUniverseScene, const qreal inUniverseWidth,
                      const qreal inUniverseHeight, const uint inNumIsles, const uint numEnemies);


    uint numberOfEnemies() const { return m_computerPlayers.count(); }

    // delete a ship and reshow the human isle
    void deleteShipOnIsle(const uint inShipId);

    // set ship to patrol the isle. Patrol means, this ship protects the isle
    // and gets attacked, whenever attacks the isle
    void setShipPatrolsIsle(const uint inShipId);

    QPointF shipPosById(const uint inShipId);

    float shipTechById(const uint inShipId);

    void shipForId(const uint inShipId, ShipInfo & outShipInfo);

    void shipForId(const uint inShipId, ShipInfo & outShipInfo, QVector<Target> & outShipTargets);

    void isleForId(const uint inIsleId, IsleInfo & outIsleInfo);

    // for communication with OverviewDialog (used in MainWindow::slotToggleOverviewDialog())
    void getAllIsleInfos(QList<IsleInfo> & outIsleInfos);
    void getAllShipInfos(QList<ShipInfo> & outShipInfos);

    void removeDefaultIsleTarget(const uint inIsleId);

    void removeAllShipTargets(const uint inShipId);

    void removeShipTargetByIndex(const uint inShipId, const int inIndex);

    void shipSetCycleTargets(const uint inShipId, const uint inCycle);

    void nextRound(UniverseScene *& inOutUniverseScene);

    // update InfoScreen after MainWindow::nextRound()
    void callInfoScreen(const InfoscreenPage inPage, const IsleInfo inIsleInfo, const ShipInfo inShipInfo);

private:
    void createIsles(const qreal inUniverseWidth, const qreal inUniverseHeight, const uint inNumIsles);

    void createShipOnIsle(UniverseScene *& inOutUniverseScene, const uint inIsleId);

    void shipFightShip(Ship *& inOutAttacker, Ship *& inOutDefender);

    void shipFightIslePatol(Ship *& inOutAttacker, const uint inIsleId);

    /**
     * @brief shipFightIsle - ship fights against isle. if needed, owner is changed, but ship not landed
     * @param inOutAttacker
     * @param inIsleId
     * @return true, if ship won
     */
    bool shipFightIsle(Ship *& inOutAttacker, const uint inIsleId);

    void shipLandOnIsle(Ship *& inOutShipToLand);

    /**
     * @brief isleForPoint - returns an isleInfo if we hit an isle
     * @param inScenePoint - position inside universe (scene coordinates)
     * @return isleInfo, if point is inside isle area, else isleInfo.id  == 0
     */
    void isleForPoint(const QPointF inScenePoint, IsleInfo & outIsleInfo);

    void setIsleOwnerById(const uint inIsleId, const uint inNewOwner, const QColor inNewColor);

    void setIslePopulationById(const uint inIsleId, const float inNewPopulation);

    int shipIndexForPoint(const QPointF inScenePoint) const;

    void shipForPoint(const QPointF inScenePoint, ShipInfo & outShipInfo, QVector<Target> & outShipTargets);

    void shipForPoint(const QPointF inScenePoint, ShipInfo & outShipInfo);

    int shipIndexForId(const uint inShipId) const;

    // really delete a ship
    void deleteShip(const uint inShipId);

    // show an isle, prepare all data
    void showHumanIsle(const IsleInfo inIsleInfo);

    // is of water objects, each id of every object is unique
    uint m_lastInsertedId;

    // Isles and Ships
    QVector<Isle*> m_isles;
    QList<Ship*> m_ships;

    // send isle and ship infos to strategy
    void prepareStrategies();
    void processStrategyCommands(const uint inOwner, const QList<ComputerMove> inComputerMoves);

    QVector<ComputerPlayer*> m_computerPlayers;


signals:
    /* These signals match the above enum InfoscreenPage.
     * How Infoscreen works:
     * 1. UniverseView::mousePressEvent() sends mouse position to Universe::slotUniverseViewClicked()
     * 2. it is selected, which object is hit by the userData
     * 3. The following signals are emitted, depends on object
     * 4. Mainwindow::slotShowUniverse* pops up the infoscreen
     *
     * on every MainWindow::nextRound(), the current InfoScreen is saved.
     * - callInfoScreen() is called, to display the page again.
     */
    void sigShowInfoWater();
    void sigShowInfoIsle(IsleInfo isleInfo);
    void sigShowInfoHumanIsle(IsleInfo isleInfo, QList<ShipInfo> sList);
    void sigShowInfoShip(ShipInfo shipInfo);
    void sigShowInfoHumanShip(ShipInfo shipInfo, QVector<Target> targets);
    void sigRecallInfoscreen(); // ask MainWindow to update infoscreen

public slots:
    void slotUniverseViewClicked(QPointF scenePos);
    void slotUniverseViewClickedFinishShipTarget(QPointF scenePos, uint shipId);
    void slotUniverseViewClickedFinishIsleTarget(QPointF scenePos, uint isleId);

};

#endif // UNIVERSE_H
