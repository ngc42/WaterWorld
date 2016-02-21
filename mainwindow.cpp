/* This File is part of WaterWorld. License is GNU GPL Version 3.
 * Please see https://github.com/ngc42/WaterWorld/blob/master/LICENSE for details.
 * WaterWorld is (C) 2016 by Eike Lange (eike@ngc42.de)
 */


#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ui_waterobjectinfo.h"
#include "shiplistitem.h"
#include "pathlistitem.h"

#include <QPaintEvent>
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

    // Push buttons on Info -> human isle
    connect(m_uiWaterObjectInfo->pbDelete, SIGNAL(clicked()), this, SLOT(slotDeleteShip()));
    connect(m_uiWaterObjectInfo->pbPatrol, SIGNAL(clicked()), this, SLOT(slotSetShipPartrol()));
    connect(m_uiWaterObjectInfo->pbTarget, SIGNAL(clicked()), this, SLOT(slotSetNewTargetForShip()));
    connect(m_uiWaterObjectInfo->pbHumanShipSetTarget, SIGNAL(clicked()), this, SLOT(slotSetNewTargetForShip()));
    connect(m_uiWaterObjectInfo->pbSetDefaultTarget, SIGNAL(clicked()), this, SLOT(slotSetNewTargetForIsle()));
    connect(m_uiWaterObjectInfo->pbClearDefaultTarget, SIGNAL(clicked()), this, SLOT(slotClearIsleTarget()));
    connect(m_uiWaterObjectInfo->tableWidget, SIGNAL(itemDoubleClicked(QTableWidgetItem*)),
            this, SLOT(slotSelectShipFromShipList(QTableWidgetItem*)));

    // Push buttons on Info -> human ship
    connect(m_uiWaterObjectInfo->pbHumanShipRepeatTargets, SIGNAL(toggled(bool)),
            this, SLOT(slotCycleShipTargets(bool)));
    connect(m_uiWaterObjectInfo->pbHumanShipDelTarget, SIGNAL(clicked()), this, SLOT(slotDeleteTarget()));

    // show nothing at start
    m_waterObjectInfo->setCurrentIndex(PAGE_NOTHING);
    m_lastCalledPage = PAGE_NOTHING;
}


MainWindow::~MainWindow()
{
    delete m_overviewDialog;
    delete m_uiWaterObjectInfo;
    delete m_ui;
}


void MainWindow::paintEvent(QPaintEvent *inEvent)
{
     QMainWindow::paintEvent(inEvent);
     QPainter painter(this);
     painter.fillRect(inEvent->rect(), Qt::lightGray);
}


void MainWindow::slotRecallInfoscreen()
{
    m_universe->callInfoScreen(m_lastCalledPage, m_lastCalledIsleInfo, m_lastCalledShipInfo);
}


void MainWindow::slotShowUniverseInfoWater()
{
    m_waterObjectInfo->setCurrentIndex(1);
    m_universeView->hidePathItem();
}


void MainWindow::slotShowUniverseInfoIsle(IsleInfo isleInfo)
{
    QPixmap pix(30, 20);
    pix.fill(isleInfo.color);
    m_uiWaterObjectInfo->labelIsleId->setText(QString("%1").arg(isleInfo.id));
    m_uiWaterObjectInfo->labelIsleColor->setPixmap(pix);

    QString s = QString("Population: %1").arg(isleInfo.population, 0, 'F', 0);
    m_uiWaterObjectInfo->labelIslePopulation->setText(s);
    s = QString("Technology: %1").arg(isleInfo.technology, 0, 'F', 0);
    m_uiWaterObjectInfo->labelIsleTechnology->setText(s);

    // set page and save last state
    m_waterObjectInfo->setCurrentIndex(PAGE_ISLE);
    m_lastCalledPage = PAGE_ISLE;
    m_lastCalledIsleInfo = isleInfo;
    m_universeView->hidePathItem();
}


void MainWindow::slotShowUniverseInfoHumanIsle(IsleInfo isleInfo, QList<ShipInfo> sList)
{
    m_universeView->hidePathItem();
    QPixmap pix(30, 20);
    pix.fill(isleInfo.color);
    m_uiWaterObjectInfo->labelHumanIsleId->setText(QString("%1").arg(isleInfo.id));
    m_uiWaterObjectInfo->labelHumanIsleId->setProperty("ISLEID", QVariant(isleInfo.id));
    m_uiWaterObjectInfo->labelHumanIsleColor->setPixmap(pix);

    m_uiWaterObjectInfo->tableWidget->setColumnCount(3);
    m_uiWaterObjectInfo->tableWidget->clearContents();
    m_uiWaterObjectInfo->tableWidget->setRowCount(0);

    QString s = QString("Population: %1").arg(isleInfo.population, 0, 'F', 0);
    m_uiWaterObjectInfo->labelHumanIslePopulation->setText(s);

    s = QString("Technology: %1").arg(isleInfo.technology, 0, 'F', 0);
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

    // default target
    switch(isleInfo.defaultTargetType)
    {
    case IsleInfo::T_ISLE:
        s = QString("T: Isle with id %1").arg(isleInfo.defaultTargetIsle);
        break;
    case IsleInfo::T_WATER:
        s = QString("Water at (%1 ; %2)").arg(isleInfo.defaultTargetPos.x()).arg(isleInfo.defaultTargetPos.y());
        break;
    default:    // nothing
        s = QString("No default target set");
        break;
    }
    m_uiWaterObjectInfo->labelHumanIsleDefaultTarget->setText(s);
    // set page and save last state
    m_waterObjectInfo->setCurrentIndex(PAGE_HUMAN_ISLE);
    m_lastCalledPage = PAGE_HUMAN_ISLE;
    m_lastCalledIsleInfo = isleInfo;
    if(isleInfo.defaultTargetType != IsleInfo::T_NOTHING)
        m_universeView->showIslePath(isleInfo.pos, isleInfo.defaultTargetPos);
}


void MainWindow::slotShowUniverseInfoShip(ShipInfo shipInfo)
{
    QPixmap pix(30, 20);
    pix.fill(shipInfo.color);

    m_uiWaterObjectInfo->labelShipId->setText(QString("%1").arg(shipInfo.id));
    m_uiWaterObjectInfo->labelShipColor->setPixmap(pix);

    QString s = QString("Damage: %1%").arg(shipInfo.damage * 100.0f, 0, 'F', 0);
    m_uiWaterObjectInfo->labelShipDamage->setText(s);

    s = QString("Tech: %1").arg(shipInfo.technology, 3, 'F', 0);
    m_uiWaterObjectInfo->labelShipTechnology->setText(s);

    // set page and save last state
    m_waterObjectInfo->setCurrentIndex(PAGE_SHIP);
    m_lastCalledPage = PAGE_SHIP;
    m_lastCalledShipInfo = shipInfo;
    m_universeView->hidePathItem();
}


void MainWindow::slotShowUniverseInfoHumanShip(ShipInfo shipInfo, QVector<Target> shipTargets)
{
    m_universeView->hidePathItem();
    QPixmap pix(30, 20);
    pix.fill(shipInfo.color);
    m_uiWaterObjectInfo->labelHumanShipId->setText(QString("%1").arg(shipInfo.id));
    m_uiWaterObjectInfo->labelHumanShipColor->setPixmap(pix);
    QString s = QString("Damage: %1%").arg(shipInfo.damage * 100.0f, 3, 'F', 0);
    m_uiWaterObjectInfo->labelHumanShipDamage->setText(s);
    s = QString("Technology: %1").arg(shipInfo.technology, 2, 'F', 1);
    m_uiWaterObjectInfo->labelHumanShipTechnology->setText(s);

    m_uiWaterObjectInfo->tableHTargets->clearContents();
    m_uiWaterObjectInfo->tableHTargets->setRowCount(shipTargets.count());

    m_uiWaterObjectInfo->pbHumanShipRepeatTargets->setEnabled(shipTargets.count() != 0);
    m_uiWaterObjectInfo->pbHumanShipRepeatTargets->setChecked(shipInfo.cycleTargetList);

    m_uiWaterObjectInfo->tableHTargets->setColumnCount(3);
    m_uiWaterObjectInfo->tableHTargets->clearContents();
    m_uiWaterObjectInfo->tableHTargets->setRowCount(0);

    // calculation of the time to spent
    uint uTimeToTarget = 0;
    bool firstUnvisited = false;
    QPointF lastPos, delta;
    for(int i = 0; i < shipTargets.count(); i++)
    {
        Target t = shipTargets.at(i);

        if(! t.visited)
        {
            if(! firstUnvisited)
            {
                firstUnvisited = true;
                lastPos = shipInfo.pos;
            }
            delta = t.pos - lastPos;
            lastPos = t.pos;
            uTimeToTarget = uTimeToTarget +
                    (uint) (std::sqrt( delta.x() * delta.x() + delta.y() * delta.y() ) / shipInfo.technology) + 1;
        }

        m_uiWaterObjectInfo->tableHTargets->insertRow(i);

        PathListItem *item1 = new PathListItem(PathListItem::PLIT_TYPE, i, uTimeToTarget, t, 0, Qt::black);
        m_uiWaterObjectInfo->tableHTargets->setItem(i, 0, item1);

        // owner of target
        uint owner = 0;
        QColor color = Qt::black;
        switch (t.tType)
        {
            case Target::T_ISLE:
            {
                IsleInfo isleInfo;
                m_universe->isleForId(t.id, isleInfo);
                owner = isleInfo.owner;
                color = isleInfo.color;
                break;
            }
            case Target::T_SHIP:
            {
                ShipInfo shipInfo;
                m_universe->shipForId(t.id, shipInfo);
                owner = shipInfo.owner;
                color = shipInfo.color;
                break;
            }
            default:    // T_WATER
                break;
        }

        PathListItem *item2 = new PathListItem(PathListItem::PLIT_OWNER, i, uTimeToTarget, t, owner, color);
        m_uiWaterObjectInfo->tableHTargets->setItem(i, 1, item2);

        PathListItem *item3 = new PathListItem(PathListItem::PLIT_TIME, i, uTimeToTarget, t, 0, Qt::black);
        m_uiWaterObjectInfo->tableHTargets->setItem(i, 2, item3);
    }

    m_uiWaterObjectInfo->tableHTargets->resizeColumnsToContents();

    // set page and save last state
    m_waterObjectInfo->setCurrentIndex(PAGE_HUMAN_SHIP);
    m_lastCalledPage = PAGE_HUMAN_SHIP;
    m_lastCalledShipInfo = shipInfo;

    if(shipInfo.hasTarget)
    {
        // @fixme: the repeat-parameter must be used some time
        m_universeView->showShipPath(shipInfo.pos, shipTargets, shipInfo.cycleTargetList);
    }
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


void MainWindow::slotSelectShipFromShipList(QTableWidgetItem *item)
{
    if(m_lastCalledPage != PAGE_HUMAN_ISLE)
        return;
    ShipListItem *slItem = dynamic_cast<ShipListItem*>(item);
    if(slItem == 0)
        return;
    uint shipId = slItem->id();

    qInfo() << "MainWindow::slotSelectShipFromShipList(), id = " << shipId;
    ShipInfo shipInfo;
    m_universe->shipForId(shipId, shipInfo);
    m_lastCalledPage = PAGE_HUMAN_SHIP;
    m_lastCalledShipInfo = shipInfo;
    m_universe->callInfoScreen(m_lastCalledPage, m_lastCalledIsleInfo, m_lastCalledShipInfo);
}


void MainWindow::slotSetNewTargetForShip()
{
    if(m_lastCalledPage == PAGE_HUMAN_ISLE)
    {
        if(m_uiWaterObjectInfo->tableWidget->currentRow() < 0)
            return;
        ShipListItem *item = (ShipListItem* ) m_uiWaterObjectInfo->tableWidget->currentItem();
        uint id = item->id();
        ShipInfo shipInfo;
        QVector<Target> shipTargets;
        m_universe->shipForId(id, shipInfo, shipTargets);
        if(shipTargets.count() > 0)
            // @fixme: the repeat-parameter must be used some time
            m_universeView->showShipPath(shipInfo.pos, shipTargets, false);
        m_universeView->toggleShipWantsTarget(shipInfo.attachPos, id, shipInfo.technology);
    }
    else if(m_lastCalledPage == PAGE_HUMAN_SHIP)
    {
        m_universeView->toggleShipWantsTarget(m_lastCalledShipInfo.attachPos, m_lastCalledShipInfo.id,
                                              m_lastCalledShipInfo.technology);
    }
    // now, its up to m_universeView to show visible feedback until target gets clicked.
    // -> then, m_universe sets up the target
    // -> then, m_universe shows up this isle again (if it was triggered by isle buttons)
}


void MainWindow::slotSetNewTargetForIsle()
{
    if(m_lastCalledPage != PAGE_HUMAN_ISLE)
    {
        qInfo() << "BUG in MainWindow::slotSetNewTargetForIsle() : wrong page saved";
        return;
    }
    // id is stored in a property field
    QVariant id_v = m_uiWaterObjectInfo->labelHumanIsleId->property("ISLEID");
    bool ok;
    IsleInfo isleInfo;
    m_universe->isleForId(id_v.toUInt(&ok), isleInfo);
    m_universeView->toggleIsleWantsTarget(isleInfo.pos, isleInfo.id);
}


void MainWindow::slotClearIsleTarget()
{
    if(m_lastCalledPage != PAGE_HUMAN_ISLE)
    {
        qInfo() << "BUG in MainWindow::slotClearIsleTarget() : wrong page saved";
        return;
    }
    // id is stored in a property field
    QVariant id_v = m_uiWaterObjectInfo->labelHumanIsleId->property("ISLEID");
    bool ok;
    uint isleId = id_v.toUInt(&ok);
    m_universe->removeDefaultIsleTarget(isleId);
    m_universe->callInfoScreen(m_lastCalledPage, m_lastCalledIsleInfo, m_lastCalledShipInfo);
}


void MainWindow::slotCycleShipTargets(bool cycle)
{
    qInfo() << "MainWindow::slotCycleShipTargets( " << cycle << " )";
    m_universe->shipSetCycleTargets(m_lastCalledShipInfo.id, cycle);
}


void MainWindow::slotDeleteTarget()
{
    int selectedRow = m_uiWaterObjectInfo->tableHTargets->currentRow();
    qInfo() << "I delete all the targets or current " << selectedRow;
    if(selectedRow < 0)
        m_universe->removeAllShipTargets(m_lastCalledShipInfo.id);
    else
        m_universe->removeShipTargetByIndex(m_lastCalledShipInfo.id, selectedRow);
    m_universe->callInfoScreen(m_lastCalledPage, m_lastCalledIsleInfo, m_lastCalledShipInfo);
}


void MainWindow::slotNextRound()
{
    m_universe->nextRound(m_universeScene);
    // call the infoscreen again. so there is a live update of ships and isles
    // during nextRound()
    m_universe->callInfoScreen(m_lastCalledPage, m_lastCalledIsleInfo, m_lastCalledShipInfo);
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
