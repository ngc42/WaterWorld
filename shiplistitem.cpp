/* This File is part of WaterWorld. License is GNU GPL Version 3.
 * Please see https://github.com/ngc42/WaterWorld/blob/master/LICENSE for details.
 * WaterWorld is (C) 2016 by Eike Lange (eike@ngc42.de)
 */


#include <shiplistitem.h>
#include <player.h>

#include <QPixmap>
#include <QPainter>
#include <QHBoxLayout>
#include <QDebug>


ShipListItem::ShipListItem(const ShipListItemType inType, const ShipInfo inShipInfo, const QString inTitle)
    : QTableWidgetItem(QTableWidgetItem::UserType + 1000), m_myType(inType), m_shipId(inShipInfo.id)
{
    if(inType == ShipListItemType::SLIT_NAME)
    {
        setSizeHint(QSize(100, 20));
        QPixmap circle(16, 16);
        if(inShipInfo.hasTarget)
        {
            // ship has target. We have to check
            // this first, as a ship can have a target AND stay on isle
            circle.fill(Qt::black);
            QPainter p;
            p.begin(&circle);
            p.setBrush(QBrush(Player::colorForOwner(inShipInfo.owner)));
            p.setPen(Player::colorForOwner(inShipInfo.owner));
            p.drawEllipse(0, 0, 5, 5);
            p.drawEllipse(10, 10, 5, 5);
            p.end();
        }
        else if(inShipInfo.posType == ShipPositionEnum::SP_ONISLE)
        {
            // ships sits arround on isle
            circle.fill(Player::colorForOwner(inShipInfo.owner));
        }
        else if(inShipInfo.posType == ShipPositionEnum::SP_PATROL)
        {
            // ship on patruille
            circle.fill(Qt::black);
            QPainter p;
            p.begin(&circle);
            p.setBrush(QBrush(Qt::black));
            p.setPen(Player::colorForOwner(inShipInfo.owner));
            p.drawEllipse(0, 0, 15, 15);
            p.setBrush(Player::colorForOwner(inShipInfo.owner));
            p.drawEllipse(5, 5, 5, 5);
            p.end();
        }
        setIcon( circle );
        setText(inTitle);

    }
    else if(inType == ShipListItemType::SLIT_STATUS)
    {
        // give visual feedback over the damage of the ship.
        setSizeHint(QSize(40, 20));
        QPixmap damageBar(20, 10);
        damageBar.fill(Qt::green);
        QPainter p;
        p.begin(&damageBar);
        p.setBrush(QBrush(Qt::red));

        // damage should be visible too, if damage is small
        int width = 0;
        if(inShipInfo.damage > 0.0f)
            width = 5;
        if(inShipInfo.damage > 0.25f)
            width = 20 * inShipInfo.damage;

        p.fillRect(20 - width, 0, width, 10, Qt::red);

        p.end();
        setIcon(damageBar);  // show damage
        QString text = QString("Damage: %1%").arg(inShipInfo.damage * 100.0f, 3, 'F', 0);
        setToolTip(text);
    }
    else
    {
        setSizeHint(QSize(40, 20));
        QString text = QString("Tech: %1").arg(inShipInfo.technology, 1, 'F', 1);
        setText(text);
    }
    setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled|Qt::ItemNeverHasChildren);
}


