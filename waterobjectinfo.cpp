/* This File is part of WaterWorld. License is GNU GPL Version 3.
 * Please see https://github.com/ngc42/WaterWorld/blob/master/LICENSE for details.
 * WaterWorld is (C) 2016 by Eike Lange (eike@ngc42.de)
 */


#include <waterobjectinfo.h>
#include <pathlistitem.h>
#include <shiplistitem.h>
#include <player.h>

#include <QPaintEvent>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QDebug>


WaterObjectInfo::WaterObjectInfo(QWidget *inParent)
    : QStackedWidget(inParent), m_ui(new Ui::StackedWidget), m_lastCalledPage(PAGE_NOTHING)
{
    m_ui->setupUi(this);
    m_ui->cbHumanIsleShiptype->insertItem(ShipTypeEnum::ST_BATTLESHIP, Ship::shipTypeName(ShipTypeEnum::ST_BATTLESHIP));
    m_ui->cbHumanIsleShiptype->insertItem(ShipTypeEnum::ST_COURIER, Ship::shipTypeName(ShipTypeEnum::ST_COURIER));
    m_ui->cbHumanIsleShiptype->insertItem(ShipTypeEnum::ST_COLONY, Ship::shipTypeName(ShipTypeEnum::ST_COLONY));
    m_ui->cbHumanIsleShiptype->insertItem(ShipTypeEnum::ST_FLEET, Ship::shipTypeName(ShipTypeEnum::ST_FLEET));

    // Push buttons and more on Info -> human isle
    connect(m_ui->pbDeleteShip, SIGNAL(clicked()), this, SLOT(slotDeleteShip()));
    connect(m_ui->pbSetShipPatrolIsle, SIGNAL(clicked()), this, SLOT(slotSetShipPartrol()));
    connect(m_ui->tblwHShipList, SIGNAL(itemDoubleClicked(QTableWidgetItem*)),
            this, SLOT(slotSelectShipFromShipList(QTableWidgetItem*)));
    connect(m_ui->pbSetShipTarget, SIGNAL(clicked()), this, SLOT(slotSetNewTargetForShip()));
    connect(m_ui->pbHumanShipSetTarget, SIGNAL(clicked()), this, SLOT(slotSetNewTargetForShip()));
    connect(m_ui->pbSetDefaultTarget, SIGNAL(clicked()), this, SLOT(slotSetNewTargetForIsle()));
    connect(m_ui->pbRemoveDefaultTarget, SIGNAL(clicked()), this, SLOT(slotRemoveIsleTarget()));
    connect(m_ui->cbHumanIsleShiptype, SIGNAL(currentIndexChanged(int)),
            this, SLOT(slotBuildNewShipType(int)));

    // Push buttons and more on Info -> human ship
    connect(m_ui->pbHumanShipRepeatTargets, SIGNAL(toggled(bool)),
            this, SLOT(slotRepeatShipTargets(bool)));
    connect(m_ui->pbHumanShipDelTarget, SIGNAL(clicked()), this, SLOT(slotDeleteTarget()));


    setCurrentIndex(PAGE_NOTHING);
}


WaterObjectInfo::~WaterObjectInfo()
{
    delete m_ui;
}


void WaterObjectInfo::showInfopageWater()
{
    m_lastCalledPage = PAGE_WATER;
    setCurrentIndex(PAGE_WATER);
}


void WaterObjectInfo::showInfopageIsle(const IsleInfo inIsleInfo)
{
    // save state
    m_lastCalledPage = PAGE_ISLE;
    m_lastCalledIsleInfo = inIsleInfo;

    // prepare page
    QPixmap pix(30, 20);
    pix.fill(inIsleInfo.color);
    m_ui->labelIsleId->setText(QString("%1").arg(inIsleInfo.id));
    m_ui->labelIsleColor->setPixmap(pix);

    QString s = QString("Population: %1").arg(inIsleInfo.population, 0, 'F', 0);
    m_ui->labelIslePopulation->setText(s);
    s = QString("Technology: %1").arg(inIsleInfo.technology, 0, 'F', 0);
    m_ui->labelIsleTechnology->setText(s);

    // show page
    setCurrentIndex(PAGE_ISLE);
}


void WaterObjectInfo::showInfopageHumanIsle(const IsleInfo inIsleInfo, const QList<ShipInfo> inShipInfoList)
{
    // save state
    m_lastCalledPage = PAGE_HUMAN_ISLE;
    m_lastCalledIsleInfo = inIsleInfo;

    // prepare page
    QPixmap pix(30, 20);
    pix.fill(inIsleInfo.color);
    m_ui->labelHumanIsleId->setText(QString("%1").arg(inIsleInfo.id));
    m_ui->labelHumanIsleId->setProperty("ISLEID", QVariant(inIsleInfo.id));
    m_ui->labelHumanIsleColor->setPixmap(pix);

    m_ui->tblwHShipList->setColumnCount(3);
    m_ui->tblwHShipList->clearContents();
    m_ui->tblwHShipList->setRowCount(0);

    QString s = QString("Population: %1").arg(inIsleInfo.population, 0, 'F', 0);
    m_ui->labelHumanIslePopulation->setText(s);

    s = QString("Technology: %1").arg((int) inIsleInfo.technology);
    m_ui->labelHumanIsleTechnology->setText(s);

    for(ShipInfo info : inShipInfoList)
    {
        m_ui->tblwHShipList->insertRow(0);
        QString shipName = QString("%1 %2").arg(Ship::typeName(info.shipType)).arg(info.id);
        ShipListItem *item1 = new ShipListItem(ShipListItem::SLIT_NAME, info, shipName);
        m_ui->tblwHShipList->setItem(0, 0, item1);

        ShipListItem *item2 = new ShipListItem(ShipListItem::SLIT_STATUS, info, "");
        m_ui->tblwHShipList->setItem(0, 1, item2);

        ShipListItem *item3 = new ShipListItem(ShipListItem::SLIT_TECHNOLOGY, info, "");
        m_ui->tblwHShipList->setItem(0, 2, item3);
    }

    m_ui->tblwHShipList->resizeColumnsToContents();

    // default target
    switch(inIsleInfo.defaultTargetType)
    {
    case IsleInfo::T_ISLE:
        s = QString("T: Isle with id %1").arg(inIsleInfo.defaultTargetIsle);
        break;
    case IsleInfo::T_WATER:
        s = QString("Water at (%1 ; %2)").arg(inIsleInfo.defaultTargetPos.x()).arg(inIsleInfo.defaultTargetPos.y());
        break;
    default:    // nothing
        s = QString("No default target set");
        break;
    }
    m_ui->labelHumanIsleDefaultTarget->setText(s);

    // Buildlevel
    switch(inIsleInfo.shipToBuild)
    {
        case ShipTypeEnum::ST_BATTLESHIP:
            m_ui->cbHumanIsleShiptype->setCurrentIndex(ShipTypeEnum::ST_BATTLESHIP);
            break;
        case ShipTypeEnum::ST_COURIER:
            m_ui->cbHumanIsleShiptype->setCurrentIndex(ShipTypeEnum::ST_COURIER);
            break;
        case ShipTypeEnum::ST_COLONY:
            m_ui->cbHumanIsleShiptype->setCurrentIndex(ShipTypeEnum::ST_COLONY);
            break;
        case ShipTypeEnum::ST_FLEET:
            m_ui->cbHumanIsleShiptype->setCurrentIndex(ShipTypeEnum::ST_FLEET);
            break;
    }
    s = QString("Build: %1%").arg(inIsleInfo.buildlevel * 100.0f, 0, 'F', 0);
    m_ui->labelHumanIsleBuildlevel->setText(s);

    // show page
    setCurrentIndex(PAGE_HUMAN_ISLE);
}


void WaterObjectInfo::showInfopageShip(const ShipInfo inShipInfo)
{
    // save state
    m_lastCalledPage = PAGE_SHIP;
    m_lastCalledShipInfo = inShipInfo;

    // prepare page
    QPixmap pix(30, 20);
    pix.fill(inShipInfo.color);

    m_ui->labelShipId->setText(QString("%1").arg(inShipInfo.id));
    m_ui->labelShipColor->setPixmap(pix);

    QString s = QString("Damage: %1%").arg(inShipInfo.damage * 100.0f, 0, 'F', 0);
    m_ui->labelShipDamage->setText(s);

    s = QString("Tech: %1").arg(inShipInfo.technology, 3, 'F', 0);
    m_ui->labelShipTechnology->setText(s);

    // ship type
    m_ui->labelShiptype->setText(Ship::typeName(inShipInfo.shipType));

    // show page
    setCurrentIndex(PAGE_SHIP);
}


void WaterObjectInfo::showInfopageHumanShip(const ShipInfo inShipInfo, const QVector<ExtendedTarget> inShipTargets)
{
    // save state
    m_lastCalledPage = PAGE_HUMAN_SHIP;
    m_lastCalledShipInfo = inShipInfo;

    // prepare page
    QPixmap pix(30, 20);
    pix.fill(inShipInfo.color);
    m_ui->labelHumanShipId->setText(QString("%1").arg(inShipInfo.id));
    m_ui->labelHumanShipColor->setPixmap(pix);
    QString s = QString("Damage: %1%").arg(inShipInfo.damage * 100.0f, 3, 'F', 0);
    m_ui->labelHumanShipDamage->setText(s);
    s = QString("Technology: %1").arg(inShipInfo.technology, 2, 'F', 1);
    m_ui->labelHumanShipTechnology->setText(s);

    // ship type
    m_ui->labelHumanShipShiptype->setText(Ship::typeName(inShipInfo.shipType));

    m_ui->tableHTargets->clearContents();
    m_ui->tableHTargets->setRowCount(inShipTargets.count());

    m_ui->pbHumanShipRepeatTargets->setEnabled(inShipTargets.count() != 0);
    m_ui->pbHumanShipRepeatTargets->setChecked(inShipInfo.cycleTargetList);

    m_ui->tableHTargets->setColumnCount(3);
    m_ui->tableHTargets->clearContents();
    m_ui->tableHTargets->setRowCount(0);

    // calculation of the time to spent
    uint uTimeToTarget = 0;
    bool firstUnvisited = false;
    QPointF lastPos, delta;
    for(int i = 0; i < inShipTargets.count(); i++)
    {
        ExtendedTarget t = inShipTargets.at(i);

        if(! t.target.visited)
        {
            if(! firstUnvisited)
            {
                firstUnvisited = true;
                lastPos = inShipInfo.pos;
            }
            delta = t.target.pos - lastPos;
            lastPos = t.target.pos;
            uTimeToTarget = uTimeToTarget +
                    (uint) (std::sqrt( delta.x() * delta.x() + delta.y() * delta.y() ) / inShipInfo.technology) + 1;
        }

        m_ui->tableHTargets->insertRow(i);

        PathListItem *item1 = new PathListItem(PathListItem::PLIT_TYPE, i, uTimeToTarget, t.target, 0, Qt::black);
        m_ui->tableHTargets->setItem(i, 0, item1);

        // owner of target
        uint owner = t.target_owner;
        QColor color = Player::colorForOwner(owner);

        PathListItem *item2 = new PathListItem(PathListItem::PLIT_OWNER, i, uTimeToTarget, t.target, owner, color);
        m_ui->tableHTargets->setItem(i, 1, item2);

        PathListItem *item3 = new PathListItem(PathListItem::PLIT_TIME, i, uTimeToTarget, t.target, 0, Qt::black);
        m_ui->tableHTargets->setItem(i, 2, item3);
    }

    m_ui->tableHTargets->resizeColumnsToContents();

    // show page
    setCurrentIndex(PAGE_HUMAN_SHIP);
}


void WaterObjectInfo::slotDeleteShip()
{
    if(m_ui->tblwHShipList->currentRow() < 0)
        return;
    ShipListItem *item = (ShipListItem* ) m_ui->tblwHShipList->currentItem();
    emit signalDeleteShipById(item->id());
}


void WaterObjectInfo::slotSetShipPartrol()
{
    if(m_ui->tblwHShipList->currentRow() < 0)
        return;
    ShipListItem *item = (ShipListItem* ) m_ui->tblwHShipList->currentItem();
    emit signalSetShipPatrolById(item->id());
}


void WaterObjectInfo::slotSelectShipFromShipList(QTableWidgetItem *item)
{
    Q_ASSERT(m_lastCalledPage == PAGE_HUMAN_ISLE);
    ShipListItem *slItem = dynamic_cast<ShipListItem*>(item);
    Q_ASSERT(slItem != 0);
    uint shipId = slItem->id();
    m_lastCalledPage = PAGE_HUMAN_SHIP;
    emit signalCallInfoscreenById(shipId);
}


void WaterObjectInfo::slotSetNewTargetForShip()
{
    if(m_lastCalledPage == PAGE_HUMAN_ISLE)
    {
        if(m_ui->tblwHShipList->currentRow() < 0)
            return;
        ShipListItem *item = (ShipListItem* ) m_ui->tblwHShipList->currentItem();
        uint id = item->id();
        emit signalSetNewTargetForShip(id);
        return;
    }
    if(m_lastCalledPage == PAGE_HUMAN_SHIP)
    {
        emit signalSetNewTargetForShip(0);
        return;
    }
    Q_ASSERT(false);    // anything else should crash
}


void WaterObjectInfo::slotSetNewTargetForIsle()
{
    Q_ASSERT(m_lastCalledPage == PAGE_HUMAN_ISLE);
    // id is stored in a property field
    QVariant id_v = m_ui->labelHumanIsleId->property("ISLEID");
    bool ok;
    uint id = id_v.toUInt(&ok);
    Q_ASSERT(ok);
    emit signalSetNewTargetForIsle(id);
}


void WaterObjectInfo::slotRemoveIsleTarget()
{
    Q_ASSERT(m_lastCalledPage == PAGE_HUMAN_ISLE);
    // id is stored in a property field
    QVariant id_v = m_ui->labelHumanIsleId->property("ISLEID");
    bool ok;
    uint id = id_v.toUInt(&ok);
    Q_ASSERT(ok);
    emit signalRemoveIsleTargetById(id);
}


void WaterObjectInfo::slotBuildNewShipType(int newType)
{
    Q_ASSERT(m_lastCalledPage == PAGE_HUMAN_ISLE);
    emit signalSetBuildShipTypeById(m_lastCalledIsleInfo.id, (ShipTypeEnum) newType);
}


void WaterObjectInfo::slotRepeatShipTargets(bool repeat)
{
    Q_ASSERT(m_lastCalledPage == PAGE_HUMAN_SHIP);
    emit signalSetRepeatTargetsById(m_lastCalledShipInfo.id, repeat);
}


void WaterObjectInfo::slotDeleteTarget()
{
    Q_ASSERT(m_lastCalledPage == PAGE_HUMAN_SHIP);
    int selectedRow = m_ui->tableHTargets->currentRow();
    emit signalDeleteTargetByIndex(m_lastCalledShipInfo.id, selectedRow);
}
