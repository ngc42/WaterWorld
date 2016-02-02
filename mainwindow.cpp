/* This File is part of WaterWorld. License is GNU GPL Version 3.
 * Please see https://github.com/ngc42/WaterWorld/blob/master/LICENSE for details.
 * WaterWorld is (C) 2016 by Eike Lange (eike@ngc42.de)
 */


#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ui_waterobjectinfo.h"
#include "shiplistitem.h"
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QDebug>


MainWindow::MainWindow(QWidget *inParent) :
    QMainWindow(inParent), m_ui(new Ui::MainWindow), m_uiWaterObjectInfo(new Ui::StackedWidget)
{
    m_ui->setupUi(this);

    m_ui->centralLayout->setContentsMargins(5, 5, 5, 5);
    m_ui->centralLayout->setSpacing(5);

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

    m_waterObjectInfo = new QStackedWidget;
    m_uiWaterObjectInfo->setupUi(m_waterObjectInfo);
    infoLayout->addWidget(m_waterObjectInfo);

    // universe show isles
    m_universe = new Universe(this, m_universeScene, 100.0, 100.0, 3);

    connect(m_minimapView, SIGNAL(sigMinimapClicked(QPointF)), m_universeView, SLOT(slotMinimapClicked(QPointF)));
    connect(m_ui->actionZoomIn, SIGNAL(triggered(bool)), m_universeView, SLOT(slotZoomIn()));
    connect(m_ui->actionZoomOut, SIGNAL(triggered(bool)), m_universeView, SLOT(slotZoomOut()));
    connect(m_ui->actionZoomNorm, SIGNAL(triggered(bool)), m_universeView, SLOT(slotZoomNorm()));
    connect(m_ui->actionNextRound, SIGNAL(triggered(bool)), this, SLOT(slotNextRound()));

    connect(m_universeView, SIGNAL(sigUniverseViewClicked(QPointF)), m_universe, SLOT(slotUniverseViewClicked(QPointF)));
    connect(m_universeView, SIGNAL(sigUniverseViewClickedFinishTarget(QPointF,uint)),
            m_universe, SLOT(slotUniverseViewClickedFinishTarget(QPointF,uint)));


    connect(m_universe, SIGNAL(sigShowInfoWater()), this, SLOT(slotShowUniverseInfoWater()));
    connect(m_universe, SIGNAL(sigShowInfoIsle(IsleInfo)), this, SLOT(slotShowUniverseInfoIsle(IsleInfo)));
    connect(m_universe, SIGNAL(sigShowInfoHumanIsle(IsleInfo, QList<ShipInfo>)), this, SLOT(slotShowUniverseInfoHumanIsle(IsleInfo, QList<ShipInfo>)));
    connect(m_universe, SIGNAL(sigShowInfoShip(ShipInfo)), this, SLOT(slotShowUniverseInfoShip(ShipInfo)));
    connect(m_universe, SIGNAL(sigShowInfoHumanShip(ShipInfo, Target)), this, SLOT(slotShowUniverseInfoHumanShip(ShipInfo, Target)));


    // Push buttons on Info -> human isle
    connect(m_uiWaterObjectInfo->pbDelete, SIGNAL(clicked()), this, SLOT(slotDeleteShip()));
    connect(m_uiWaterObjectInfo->pbPatrol, SIGNAL(clicked()), this, SLOT(slotSetShipPartrol()));
    connect(m_uiWaterObjectInfo->pbTarget, SIGNAL(clicked()), this, SLOT(slotSetNewTargetForShip()));
    connect(m_uiWaterObjectInfo->pbHumanShipSetTarget, SIGNAL(clicked()), this, SLOT(slotSetNewTargetForShip()));


    // show nothing at start
    m_waterObjectInfo->setCurrentIndex(PAGE_NOTHING);
    m_lastCalledPage = PAGE_NOTHING;
}


MainWindow::~MainWindow()
{
    delete m_uiWaterObjectInfo;
    delete m_ui;
}


void MainWindow::slotShowUniverseInfoWater()
{
    m_waterObjectInfo->setCurrentIndex(1);
}


void MainWindow::slotShowUniverseInfoIsle(IsleInfo isleInfo)
{
    QPixmap pix(30, 20);
    pix.fill(isleInfo.color);
    m_uiWaterObjectInfo->labelIsleId->setText(QString("%1").arg(isleInfo.id));
    m_uiWaterObjectInfo->labelIsleColor->setPixmap(pix);

    QString s = QString("P: %1").arg(isleInfo.population);
    m_uiWaterObjectInfo->labelIslePopulation->setText(s);
    s = QString("T: %1").arg(isleInfo.technology, 0);
    m_uiWaterObjectInfo->labelIsleTechnology->setText(s);

    // set page and save last state
    m_waterObjectInfo->setCurrentIndex(PAGE_ISLE);
    m_lastCalledPage = PAGE_ISLE;
    m_lastCalledIsleInfo = isleInfo;
}


void MainWindow::slotShowUniverseInfoHumanIsle(IsleInfo isleInfo, QList<ShipInfo> sList)
{
    QPixmap pix(30, 20);
    pix.fill(isleInfo.color);
    m_uiWaterObjectInfo->labelHumanIsleId->setText(QString("%1").arg(isleInfo.id));
    m_uiWaterObjectInfo->labelHumanIsleColor->setPixmap(pix);

    m_uiWaterObjectInfo->tableWidget->setColumnCount(3);
    m_uiWaterObjectInfo->tableWidget->clearContents();
    m_uiWaterObjectInfo->tableWidget->setRowCount(0);

    QString s = QString("H-P: %1").arg(isleInfo.population);
    m_uiWaterObjectInfo->labelHumanIslePopulation->setText(s);

    s = QString("H-T: %1").arg(isleInfo.technology, 1);
    m_uiWaterObjectInfo->labelHumanIsleTechnology->setText(s);

    for(ShipInfo info : sList)
    {
        m_uiWaterObjectInfo->tableWidget->insertRow(0);
        QString shipName = QString("Sailor %1").arg(info.id);
        ShipListItem *item1 = new ShipListItem(ShipListItem::SLIT_NAME, info, shipName);
        m_uiWaterObjectInfo->tableWidget->setItem(0, 0, item1);

        ShipListItem *item2 = new ShipListItem(ShipListItem::SLIT_STATUS, info, "");
        m_uiWaterObjectInfo->tableWidget->setItem(0, 1, item2);

        ShipListItem *item3 = new ShipListItem(ShipListItem::SLIT_TECHNOLOGY, info, "");
        m_uiWaterObjectInfo->tableWidget->setItem(0, 2, item3);
    }

    m_uiWaterObjectInfo->tableWidget->resizeColumnsToContents();

    // set page and save last state
    m_waterObjectInfo->setCurrentIndex(PAGE_HUMAN_ISLE);
    m_lastCalledPage = PAGE_HUMAN_ISLE;
    m_lastCalledIsleInfo = isleInfo;
}


void MainWindow::slotShowUniverseInfoShip(ShipInfo shipInfo)
{

    QPixmap pix(30, 20);
    pix.fill(shipInfo.color);

    m_uiWaterObjectInfo->labelShipId->setText(QString("%1").arg(shipInfo.id));
    m_uiWaterObjectInfo->labelShipColor->setPixmap(pix);

    QString s = QString("Dmg: %1").arg(shipInfo.damage, 2);
    m_uiWaterObjectInfo->labelShipDamage->setText(s);

    s = QString("Tech: %1").arg(shipInfo.technology, 2);
    m_uiWaterObjectInfo->labelShipTechnology->setText(s);

    // set page and save last state
    m_waterObjectInfo->setCurrentIndex(PAGE_SHIP);
    m_lastCalledPage = PAGE_SHIP;
    m_lastCalledShipInfo = shipInfo;
}


void MainWindow::slotShowUniverseInfoHumanShip(ShipInfo shipInfo, Target shipTarget)
{
    QPixmap pix(30, 20);
    pix.fill(shipInfo.color);
    m_uiWaterObjectInfo->labelHumanShipId->setText(QString("%1").arg(shipInfo.id));
    m_uiWaterObjectInfo->labelHumanShipColor->setPixmap(pix);
    QString s = QString("Dmg: %1").arg(shipInfo.damage, 2);
    m_uiWaterObjectInfo->labelHumanShipDamage->setText(s);
    s = QString("Tech: %1").arg(shipInfo.technology, 2);
    m_uiWaterObjectInfo->labelHumanShipTechnology->setText(s);

    if(shipTarget.validTarget)
    {
        s = QString(shipTarget.tType == Target::TargetEnum::T_ISLE ? "isle" : Target::TargetEnum::T_SHIP ? "ship" : "water");
    }
    else
        s = {"no"};
    m_uiWaterObjectInfo->labelHumanShipHasTarget->setText(s);

    // set page and save last state
    m_waterObjectInfo->setCurrentIndex(PAGE_HUMAN_SHIP);
    m_lastCalledPage = PAGE_HUMAN_SHIP;
    m_lastCalledShipInfo = shipInfo;

}

void MainWindow::slotDeleteShip()
{
    // @fixme: we have ShipPositionEnum::S_TRASH, but this here really deletes the ship in place
    // correct?

    if(m_uiWaterObjectInfo->tableWidget->currentRow() < 0)
        return;
    ShipListItem *item = (ShipListItem* ) m_uiWaterObjectInfo->tableWidget->currentItem();
    m_universe->deleteShipOnIsle(item->id());
}


void MainWindow::slotSetShipPartrol()
{
    if(m_uiWaterObjectInfo->tableWidget->currentRow() < 0)
        return;
    ShipListItem *item = (ShipListItem* ) m_uiWaterObjectInfo->tableWidget->currentItem();
    m_universe->setShipPatrolsIsle(item->id());
}


void MainWindow::slotSetNewTargetForShip()
{
    if(m_lastCalledPage == PAGE_HUMAN_ISLE)
    {
        if(m_uiWaterObjectInfo->tableWidget->currentRow() < 0)
            return;
        ShipListItem *item = (ShipListItem* ) m_uiWaterObjectInfo->tableWidget->currentItem();
        m_universeView->toggleShipWantsTarget(m_universe->shipPosById(item->id()), item->id());
    }
    else if(m_lastCalledPage == PAGE_HUMAN_SHIP)
    {
        m_universeView->toggleShipWantsTarget(m_lastCalledShipInfo.pos, m_lastCalledShipInfo.id);
    }
    // now, its up to m_universeView to show visible feedback until target gets clicked.
    // -> then, m_universe sets up the target
    // -> then, m_universe shows up this isle again (if it was triggered by isle buttons)
}


void MainWindow::slotNextRound()
{
    m_universe->nextRound(m_universeScene);
    // call the infoscreen again. so there is a live update of ships and isles
    // during nextRound()
    m_universe->callInfoScreen(m_lastCalledPage, m_lastCalledIsleInfo, m_lastCalledShipInfo);
}
