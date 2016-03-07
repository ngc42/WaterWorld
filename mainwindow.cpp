/* This File is part of WaterWorld. License is GNU GPL Version 3.
 * Please see https://github.com/ngc42/WaterWorld/blob/master/LICENSE for details.
 * WaterWorld is (C) 2016 by Eike Lange (eike@ngc42.de)
 */


#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "shiplistitem.h"
#include "pathlistitem.h"

#include <QPaintEvent>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QDebug>


MainWindow::MainWindow(QWidget *inParent) :
    QMainWindow(inParent), m_ui(new Ui::MainWindow)
{
    m_ui->setupUi(this);

    // the scene, where everything happens
    m_universeScene = new UniverseScene({0.0f, 0.0f, 1000.0f, 1000.0f}, this);

    // the big universe
    m_universeView = new UniverseView(this);
    m_ui->centralLayout->addWidget(m_universeView);
    m_universeView->setScene(m_universeScene);

    // info widget
    m_infoWidget = new QWidget(this);
    m_ui->centralLayout->addWidget(m_infoWidget);
    m_infoWidget->setMinimumSize(200, 200);
    m_infoWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);

    // Layout for info widget
    QVBoxLayout *infoLayout = new QVBoxLayout;
    infoLayout->setContentsMargins(0, 0, 0, 0);
    m_infoWidget->setLayout(infoLayout);

    // a small minimap
    m_minimapView = new MinimapView(this);
    infoLayout->addWidget(m_minimapView);
    m_minimapView->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_minimapView->setMaximumSize(200, 200);
    m_minimapView->setScene(m_universeScene);
    m_minimapView->scale(0.19f, 0.19f);

    m_waterObjectInfo = new WaterObjectInfo;
    infoLayout->addWidget(m_waterObjectInfo);

    // universe show isles
    m_universe = new Universe(this, m_universeScene, m_universeScene->width(), m_universeScene->height(), 20, 3);

    // overview dialog
    m_overviewDialog = new OverviewDialog(m_universe->numberOfEnemies() + 1, this);
    m_overviewDialog->hide();

    connect(m_minimapView, SIGNAL(sigMinimapClicked(QPointF)), m_universeView, SLOT(slotMinimapClicked(QPointF)));
    connect(m_ui->actionZoomIn, SIGNAL(triggered(bool)), m_universeView, SLOT(slotZoomIn()));
    connect(m_ui->actionZoomOut, SIGNAL(triggered(bool)), m_universeView, SLOT(slotZoomOut()));
    connect(m_ui->actionZoomNorm, SIGNAL(triggered(bool)), m_universeView, SLOT(slotZoomNorm()));
    connect(m_ui->actionNextRound, SIGNAL(triggered(bool)), this, SLOT(slotNextRound()));
    connect(m_ui->actionOverview, SIGNAL(triggered()), this, SLOT(slotToggleOverviewDialog()));

    connect(m_universeView, SIGNAL(sigUniverseViewClicked(QPointF)), m_universe, SLOT(slotUniverseViewClicked(QPointF)));
    connect(m_universeView, SIGNAL(sigUniverseViewClickedFinishShipTarget(QPointF,uint)),
            m_universe, SLOT(slotUniverseViewClickedFinishShipTarget(QPointF,uint)));
    connect(m_universeView, SIGNAL(sigUniverseViewClickedFinishIsleTarget(QPointF,uint)),
            m_universe, SLOT(slotUniverseViewClickedFinishIsleTarget(QPointF,uint)));

    connect(m_universe, SIGNAL(sigShowInfoWater()), this, SLOT(slotShowUniverseInfoWater()));
    connect(m_universe, SIGNAL(sigShowInfoIsle(IsleInfo)), this, SLOT(slotShowUniverseInfoIsle(IsleInfo)));
    connect(m_universe, SIGNAL(sigShowInfoHumanIsle(IsleInfo, QList<ShipInfo>)),
            this, SLOT(slotShowUniverseInfoHumanIsle(IsleInfo, QList<ShipInfo>)));
    connect(m_universe, SIGNAL(sigShowInfoShip(ShipInfo)), this, SLOT(slotShowUniverseInfoShip(ShipInfo)));
    connect(m_universe, SIGNAL(sigShowInfoHumanShip(ShipInfo, QVector<Target>)),
            this, SLOT(slotShowUniverseInfoHumanShip(ShipInfo, QVector<Target>)));
    connect(m_universe, SIGNAL(sigRecallInfoscreen()), this, SLOT(slotRecallInfoscreen()));


    // Infoscreen (WaterObjectInfo) -> human isle
    connect(m_waterObjectInfo, SIGNAL(signalDeleteShipById(uint)), this, SLOT(slotDeleteShip(uint)));
    connect(m_waterObjectInfo, SIGNAL(signalSetShipPatrolById(uint)), this, SLOT(slotSetShipPartrol(uint)));
    connect(m_waterObjectInfo, SIGNAL(signalCallInfoscreenById(uint)), this, SLOT(slotRecallInfoscreenById(uint)));
    connect(m_waterObjectInfo, SIGNAL(signalSetNewTargetForShip(uint)), this, SLOT(slotSetNewTargetForShip(uint)));
    connect(m_waterObjectInfo, SIGNAL(signalSetNewTargetForIsle(uint)), this, SLOT(slotSetNewTargetForIsle(uint)));
    connect(m_waterObjectInfo, SIGNAL(signalRemoveIsleTargetById(uint)), this, SLOT(slotRemoveIsleTarget(uint)));
    connect(m_waterObjectInfo, SIGNAL(signalSetBuildShipTypeById(uint,ShipTypeEnum)),
            this, SLOT(slotBuildNewShipType(uint,ShipTypeEnum)));

    // Infoscreen (WaterObjectInfo) -> human ship
    connect(m_waterObjectInfo, SIGNAL(signalSetRepeatTargetsById(uint,bool)),
            this, SLOT(slotRepeatShipTargets(uint,bool)));
    connect(m_waterObjectInfo, SIGNAL(signalDeleteTargetByIndex(uint,int)), this, SLOT(slotDeleteTarget(uint,int)));
}


MainWindow::~MainWindow()
{
    delete m_overviewDialog;
    delete m_ui;
}


void MainWindow::slotRecallInfoscreen()
{
    uint id = 0;

    switch(m_waterObjectInfo->lastCalledPage())
    {
        case PAGE_HUMAN_ISLE:
        case PAGE_ISLE:         // fall through
            id = m_waterObjectInfo->lastCalledIsleInfo().id;
            break;
        case PAGE_HUMAN_SHIP:
        case PAGE_SHIP:         // fall through
            id = m_waterObjectInfo->lastCalledShipInfo().id;
            break;
        default:
            id = 0;
    }
    slotRecallInfoscreenById(id);
}


void MainWindow::slotRecallInfoscreenById(uint objectId)
{
    m_universe->callInfoScreen(m_waterObjectInfo->lastCalledPage(), objectId);
}


void MainWindow::slotShowUniverseInfoWater()
{
    m_waterObjectInfo->showInfopageWater();
    m_universeView->hidePathItem();
}


void MainWindow::slotShowUniverseInfoIsle(IsleInfo isleInfo)
{
    m_waterObjectInfo->showInfopageIsle(isleInfo);
    m_universeView->hidePathItem();
}


void MainWindow::slotShowUniverseInfoHumanIsle(IsleInfo isleInfo, QList<ShipInfo> sList)
{
    m_waterObjectInfo->showInfopageHumanIsle(isleInfo, sList);
    m_universeView->hidePathItem();
    if(isleInfo.defaultTargetType != IsleInfo::T_NOTHING)
        m_universeView->showIslePath(isleInfo.pos, isleInfo.defaultTargetPos);
}


void MainWindow::slotShowUniverseInfoShip(ShipInfo shipInfo)
{
    m_waterObjectInfo->showInfopageShip(shipInfo);
    m_universeView->hidePathItem();
}


void MainWindow::slotShowUniverseInfoHumanShip(ShipInfo shipInfo, QVector<Target> shipTargets)
{
    m_universeView->hidePathItem();
    QVector<ExtendedTarget> extTargetList;
    for(Target t : shipTargets)
    {
        ExtendedTarget ext;
        ext.target = t;
        switch(t.tType)
        {
            case Target::T_SHIP:
            {
                ShipInfo tShipInfo;
                m_universe->shipForId(t.id, tShipInfo);
                ext.target_owner = tShipInfo.owner;
            }
                break;
            case Target::T_ISLE:
            {
                IsleInfo tIsleInfo;
                m_universe->isleForId(t.id, tIsleInfo);
                ext.target_owner = tIsleInfo.owner;
            }
                break;
            case Target::T_WATER:
                ext.target_owner = 0;
                break;
        }
        extTargetList.append(ext);
    }

    m_waterObjectInfo->showInfopageHumanShip(shipInfo, extTargetList);
    if(shipInfo.hasTarget)
    {
        m_universeView->showShipPath(shipInfo.pos, shipTargets, shipInfo.cycleTargetList);
    }
}


void MainWindow::slotDeleteShip(uint shipId)
{
    // this here instantly deletes a ship, because user wants this
    m_universe->deleteShipOnIsle(shipId);
}


void MainWindow::slotSetShipPartrol(uint shipId)
{
    m_universe->setShipPatrolsIsle(shipId);
}


void MainWindow::slotSetNewTargetForShip(uint shipId)
{
    InfoscreenPageEnum page = m_waterObjectInfo->lastCalledPage();
    if(page == PAGE_HUMAN_ISLE)
    {
        ShipInfo shipInfo;
        QVector<Target> shipTargets;
        m_universe->shipForId(shipId, shipInfo, shipTargets);
        if(shipTargets.count() > 0)
            m_universeView->showShipPath(shipInfo.pos, shipTargets, shipInfo.cycleTargetList);
        m_universeView->toggleShipWantsTarget(shipInfo.attachPos, shipId, shipInfo.technology);
    }
    else if(page == PAGE_HUMAN_SHIP)
    {
        ShipInfo shipInfo = m_waterObjectInfo->lastCalledShipInfo();
        m_universeView->toggleShipWantsTarget(shipInfo.attachPos, shipInfo.id, shipInfo.technology);
    }
    // now, its up to m_universeView to show visible feedback until target gets clicked.
    // -> then, m_universe sets up the target
    // -> then, m_universe shows up this isle again (if it was triggered by isle buttons)
}


void MainWindow::slotSetNewTargetForIsle(uint isleId)
{
    IsleInfo isleInfo;
    m_universe->isleForId(isleId, isleInfo);
    m_universeView->toggleIsleWantsTarget(isleInfo.pos, isleInfo.id);
}


void MainWindow::slotRemoveIsleTarget(uint isleId)
{
    m_universe->removeDefaultIsleTarget(isleId);
    slotRecallInfoscreen();
}


void MainWindow::slotBuildNewShipType(uint isleId, ShipTypeEnum newType)
{
    m_universe->isleSetShipToBuild(isleId, newType);
}


void MainWindow::slotRepeatShipTargets(uint shipId, bool repeat)
{
    m_universe->shipSetCycleTargets(shipId, repeat);
}


void MainWindow::slotDeleteTarget(uint shipId, int index)
{
    if(index < 0)
        m_universe->removeAllShipTargets(shipId);
    else
        m_universe->removeShipTargetByIndex(shipId, index);
    slotRecallInfoscreen();
}


void MainWindow::slotNextRound()
{
    m_universe->nextRound(m_universeScene);
    // call the infoscreen again. so there is a live update of ships and isles
    // during nextRound()
    slotRecallInfoscreen();
    // remove artefacts of moving ships
    m_universeScene->update(m_universeScene->sceneRect());
}


void MainWindow::slotToggleOverviewDialog()
{
    if(m_overviewDialog->isHidden())
    {
        QList<IsleInfo> isleInfoList;
        m_universe->getAllIsleInfos(isleInfoList);
        m_overviewDialog->updateIsleInfos(isleInfoList);

        QList<ShipInfo> shipInfoList;
        m_universe->getAllShipInfos(shipInfoList);
        m_overviewDialog->updateShipInfos(shipInfoList);
        m_overviewDialog->show();
    }
    else
        m_overviewDialog->hide();
}
