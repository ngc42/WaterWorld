/* This File is part of WaterWorld. License is GNU GPL Version 3.
 * Please see https://github.com/ngc42/WaterWorld/blob/master/LICENSE for details.
 * WaterWorld is (C) 2016 by Eike Lange (eike@ngc42.de)
 */

#include "shiplistitem.h"
#include <QPixmap>
#include <QPainter>
#include <QHBoxLayout>
#include <QDebug>

ShipListItem::ShipListItem(const ShipListItemType inType, const ShipInfo inShipInfo, const QString inTitle)
    : QTableWidgetItem(), m_myType(inType), m_shipId(inShipInfo.id)
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
            p.setBrush(QBrush(Qt::green));
            p.setPen(Qt::green);
            p.drawEllipse(0, 0, 5, 5);
            p.drawEllipse(10, 10, 5, 5);
            p.end();
        }
        else if(inShipInfo.posType == ShipPositionEnum::S_ONISLE)
        {
            // ships sits arround on isle
            circle.fill(Qt::green);
        }
        else if(inShipInfo.posType == ShipPositionEnum::S_PATROL)
        {
            // ship on patruille
            circle.fill(Qt::black);
            QPainter p;
            p.begin(&circle);
            p.setBrush(QBrush(Qt::black));
            p.setPen(Qt::green);
            p.drawEllipse(0, 0, 15, 15);
            p.setBrush(Qt::green);
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
        QPixmap circle(20, 10);
        circle.fill(Qt::green);
        QPainter p;
        p.begin(&circle);
        p.setBrush(QBrush(Qt::red));
        int width = 20 * inShipInfo.damage;
        p.fillRect(20 - width, 0, width, 10, Qt::red);
        p.end();
        setIcon(circle);  // show damage
        QString text = QString("Dmg: %1").arg(inShipInfo.damage * 100.0f, 3);
        qDebug() << " SLIT_STATUS: id: " << inShipInfo.id << " dmg: " << inShipInfo.damage;
        setText(text);
    }
    else
    {
        setSizeHint(QSize(40, 20));
        QString text = QString("T: %1").arg(inShipInfo.technology, 1);
        setText(text);
    }
}


