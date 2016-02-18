/* This File is part of WaterWorld. License is GNU GPL Version 3.
 * Please see https://github.com/ngc42/WaterWorld/blob/master/LICENSE for details.
 * WaterWorld is (C) 2016 by Eike Lange (eike@ngc42.de)
 */


#ifndef PATHLISTITEM_H
#define PATHLISTITEM_H


#include <ship.h>
#include <QTableWidgetItem>
#include <QLabel>
#include <QPixmap>
#include <QIcon>


class PathListItem : public QTableWidgetItem
{
public:

    /**
     * @brief The PathListItemType enum describes different type of data to set
     */
    enum PathListItemType {PLIT_TYPE, PLIT_OWNER, PLIT_TIME};

    /**
     * @brief PathListItem - creates an item used for Infoscreen->Human ship, target list
     * @param inType - the type of this item
     * @param inShipInfo - info about the ship
     * @param inTarget - info about the target we display
     */
    PathListItem(const PathListItemType inType, const int inIndex, uint inTimeToTarget, const Target inTarget);

    /**
     * @brief id returns the target id, we show info about
     * @return TargetId
     */
    uint id() const { return m_targetId; }

private:
    PathListItemType m_myType;
    uint m_targetId;
};

#endif // PATHLISTITEM_H
