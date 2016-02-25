/* This File is part of WaterWorld. License is GNU GPL Version 3.
 * Please see https://github.com/ngc42/WaterWorld/blob/master/LICENSE for details.
 * WaterWorld is (C) 2016 by Eike Lange (eike@ngc42.de)
 */


#ifndef WATEROBJECTINFO_H
#define WATEROBJECTINFO_H


#include <isle.h>
#include <ship.h>
#include "ui_waterobjectinfo.h"

#include <QStackedWidget>
#include <QTableWidgetItem>


namespace Ui {
class StackedWidget;
}


enum InfoscreenPageEnum {PAGE_NOTHING = 0, PAGE_WATER = 1, PAGE_ISLE = 2, PAGE_HUMAN_ISLE = 3, PAGE_SHIP = 4, PAGE_HUMAN_SHIP = 5};


struct ExtendedTarget
{
    Target target;      // target of a ship
    uint target_owner;  // owner of this target
};


class WaterObjectInfo : public QStackedWidget
{
    Q_OBJECT

public:
    WaterObjectInfo(QWidget * inParent = 0);

    ~WaterObjectInfo();

    // show the different pages
    void showInfopageWater();
    void showInfopageIsle(const IsleInfo inIsleInfo);
    void showInfopageHumanIsle(const IsleInfo inIsleInfo, const QList<ShipInfo> inShipInfoList);
    void showInfopageShip(const ShipInfo inShipInfo);
    void showInfopageHumanShip(const ShipInfo inShipInfo, const QVector<ExtendedTarget> inShipTargets);


    // @fixme: remove this method
    Ui::StackedWidget *ui() { return m_ui; }

    // getter for internal state
    InfoscreenPageEnum lastCalledPage() const { return m_lastCalledPage; }
    ShipInfo lastCalledShipInfo() const { return m_lastCalledShipInfo; }
    IsleInfo lastCalledIsleInfo() const { return m_lastCalledIsleInfo; }

private:
    Ui::StackedWidget *m_ui;

    InfoscreenPageEnum m_lastCalledPage;
    ShipInfo m_lastCalledShipInfo;
    IsleInfo m_lastCalledIsleInfo;

signals:
    void signalDeleteShipById(uint shipId);
    void signalSetShipPatrolById(uint shipId);
    void signalCallInfoscreenById(uint objectId);
    void signalSetNewTargetForShip(uint shipId);
    void signalSetNewTargetForIsle(uint isleId);
    void signalRemoveIsleTargetById(uint isleId);
    void signalSetBuildShipTypeById(uint isleId, ShipTypeEnum shipType);
    void signalSetRepeatTargetsById(uint shipId, bool repeat);
    void signalDeleteTargetByIndex(uint ShipId, int index); // index == -1 means ALL targets



private slots:
    // user clicked a button on info view -> human isle
    void slotDeleteShip();
    void slotSetShipPartrol();
    void slotSelectShipFromShipList(QTableWidgetItem *item);    // double click on ship list on human isle
    void slotSetNewTargetForShip();
    void slotSetNewTargetForIsle();
    void slotRemoveIsleTarget();
    void slotBuildNewShipType(int newType);

    // user clicked a button on info view -> human ship
    void slotRepeatShipTargets(bool cycle);
    void slotDeleteTarget();

};

#endif // WATEROBJECTINFO_H
