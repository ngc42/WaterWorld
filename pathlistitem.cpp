/* This File is part of WaterWorld. License is GNU GPL Version 3.
 * Please see https://github.com/ngc42/WaterWorld/blob/master/LICENSE for details.
 * WaterWorld is (C) 2016 by Eike Lange (eike@ngc42.de)
 */


#include "pathlistitem.h"
#include <QPixmap>
#include <QIcon>

PathListItem::PathListItem(const PathListItemType inType, const int inIndex, uint inTimeToTarget,
                           const Target inTarget, const uint inTargetOwner, const QColor inTargetColor)
    : QTableWidgetItem(QTableWidgetItem::UserType + 2000), m_myType(inType), m_targetId(inIndex)
{
    if(inType == PathListItemType::PLIT_TYPE)
    {
        setSizeHint(QSize(80, 20));

        switch(inTarget.tType)
        {
            case Target::T_ISLE:
                setText("Isle");
                break;
            case Target::T_SHIP:
                setText("Ship");
                break;
            default:    // T_WATER
                setText("Water");
                break;
        }
    }
    else if(inType == PathListItemType::PLIT_OWNER)
    {
        setSizeHint(QSize(50, 20));

        QString ownerText = QString("%1").arg(inTargetOwner);
        switch(inTarget.tType)
        {
            case Target::T_ISLE:
            {
                QPixmap pixmap(10, 10);
                pixmap.fill(inTargetColor);
                QIcon icon(pixmap);
                setText(ownerText);
                setIcon(icon);
            }
                break;
            case Target::T_SHIP:
            {
                QPixmap pixmap(10, 10);
                pixmap.fill(inTargetColor);
                QIcon icon(pixmap);
                setText(ownerText);
                setIcon(icon);
            }
                break;
            default:    // T_WATER
            {
                QPixmap pixmap(36, 10);
                pixmap.fill(Qt::darkBlue);
                QIcon icon(pixmap);
                setIcon(icon);
            }
                break;
        }
    }
    else if(inType == PathListItemType::PLIT_TIME)
    {
        QString s = QString("%1 days").arg(inTimeToTarget);
        setText(s);
    }
    setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled|Qt::ItemNeverHasChildren);
}
