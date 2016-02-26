/* This File is part of WaterWorld. License is GNU GPL Version 3.
 * Please see https://github.com/ngc42/WaterWorld/blob/master/LICENSE for details.
 * WaterWorld is (C) 2016 by Eike Lange (eike@ngc42.de)
 */


#ifndef MAINWINDOW_H
#define MAINWINDOW_H


#include <universescene.h>
#include <universeview.h>
#include <minimapview.h>
#include <universe.h>
#include <overviewdialog.h>
#include <waterobjectinfo.h>

#include <QMainWindow>

namespace Ui {
class MainWindow;
}


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *inParent = 0);
    ~MainWindow();

private:
    // common ui parts of main window
    Ui::MainWindow *m_ui;
    UniverseScene *m_universeScene;
    QWidget *m_infoWidget;
    UniverseView *m_universeView;
    MinimapView *m_minimapView;

    // Player overview
    OverviewDialog *m_overviewDialog;

    // Info about water objects, the infoscreen
    WaterObjectInfo *m_waterObjectInfo;

    // Universe
    Universe *m_universe;

private slots:
    // unverse or infoscreen wants us call infoscreen again
    void slotRecallInfoscreen();
    void slotRecallInfoscreenById(uint objectId);

    // user clicked somewhere on the ocean
    void slotShowUniverseInfoWater();
    void slotShowUniverseInfoIsle(IsleInfo isleInfo);
    void slotShowUniverseInfoHumanIsle(IsleInfo isleInfo, QList<ShipInfo> sList);
    void slotShowUniverseInfoShip(ShipInfo shipInfo);
    void slotShowUniverseInfoHumanShip(ShipInfo shipInfo, QVector<Target> shipTargets);


    // user clicked a button on info view -> human isle
    void slotDeleteShip(uint shipId);
    void slotSetShipPartrol(uint shipId);
    void slotSetNewTargetForShip(uint shipId);
    void slotSetNewTargetForIsle(uint isleId);
    void slotRemoveIsleTarget(uint isleId);
    void slotBuildNewShipType(uint isleId, ShipTypeEnum newType);

    // user clicked a button on info view -> human ship
    void slotRepeatShipTargets(uint shipId, bool repeat);
    void slotDeleteTarget(uint shipId, int index);


    // nextround
    void slotNextRound();

    // overview dialog
    void slotToggleOverviewDialog();
};

#endif // MAINWINDOW_H
