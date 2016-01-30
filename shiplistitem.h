#ifndef SHIPLISTITEM_H
#define SHIPLISTITEM_H

#include <QTableWidgetItem>
#include <QLabel>
#include <QPixmap>
#include <QIcon>

#include <ship.h>


/**
 * @brief The ShipListItem class
 *
 * This class represents an item in a QTableWidget within the infoscreen.
 *
 * This infoscreen is part of information we show about an isle. For human isles, we
 * show the ships on this isle.
 * See mainwindow -> m_uiWaterObjectInfo for related object information
 *
 * For every ship, we show 3 parts of information,
 * -- the name together with a symbol showing, if the ship has a target or patrols the isle
 * -- the damage (status) of the ship
 * -- the technology level, for some ships, this is equivalent to the "force".
 *
 */

class ShipListItem : public QTableWidgetItem
{
public:

    /**
     * @brief The ShipListItemType enum describes different type of data to set
     */
    enum ShipListItemType {SLIT_NAME, SLIT_STATUS, SLIT_TECHNOLOGY};

    /**
     * @brief ShipListItem
     * @param inType - a selector for different parts of data to show
     * @param inShipInfo
     * @param inTitle
     */
    ShipListItem(const ShipListItemType inType, const ShipInfo inShipInfo, const QString inTitle);


    /**
     * @brief id returns the ship, we show info about
     * @return ShipId
     */
    uint id() const { return m_shipId; }


private:
    ShipListItemType m_myType;
    uint m_shipId;
};

#endif // SHIPLISTITEM_H
