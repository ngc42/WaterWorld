/* This File is part of WaterWorld. License is GNU GPL Version 3.
 * Please see https://github.com/ngc42/WaterWorld/blob/master/LICENSE for details.
 * WaterWorld is (C) 2016 by Eike Lange (eike@ngc42.de)
 */


#include "pathlistitem.h"


PathListItem::PathListItem(const PathListItemType inType, const int inIndex, uint inTimeToTarget, const Target inTarget)
    : QTableWidgetItem(QTableWidgetItem::UserType + 2000), m_myType(inType), m_targetId(inIndex)
{
    if(inType == PathListItemType::PLIT_TYPE)
    {
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
        switch(inTarget.tType)
        {
            case Target::T_ISLE:
                setText("unknown");
                break;
            case Target::T_SHIP:
                setText("unknown");
                break;
            default:    // T_WATER
                setText("-");
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
