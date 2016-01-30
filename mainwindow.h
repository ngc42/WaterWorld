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
#include <QMainWindow>
#include <QStackedWidget>


namespace Ui {
class MainWindow;
class StackedWidget;
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

    // Info about water objects
    Ui::StackedWidget *m_uiWaterObjectInfo;
    QStackedWidget *m_waterObjectInfo;

    // Universe
    Universe *m_universe;

    /*
     *  last info screen issued with sigShow***
     * This is stored here, because after each nextRound(), we need to show the
     * infoscreen again.
     */
    InfoscreenPage m_lastCalledPage;
    ShipInfo m_lastCalledShipInfo;
    IsleInfo m_lastCalledIsleInfo;


private slots:
    // user clicked somewhere on the ocean
    void slotShowUniverseInfoWater();
    void slotShowUniverseInfoIsle(IsleInfo isleInfo);
    void slotShowUniverseInfoHumanIsle(IsleInfo isleInfo, QList<ShipInfo> sList);
    void slotShowUniverseInfoShip(ShipInfo shipInfo);

    // user clicked a button on info view -> human isle
    void slotDeleteShip();
    void slotSetShipPartrol();
    void slotSetNewTargetForShip();

    // nextround
    void slotNextRound();
};

#endif // MAINWINDOW_H
