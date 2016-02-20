/* This File is part of WaterWorld. License is GNU GPL Version 3.
 * Please see https://github.com/ngc42/WaterWorld/blob/master/LICENSE for details.
 * WaterWorld is (C) 2016 by Eike Lange (eike@ngc42.de)
 */


#include "player.h"


Player::Player(const uint inOwner)
    : m_owner(inOwner)
{
}


QColor Player::colorForOwner(uint inOwnerId)
{
    switch(inOwnerId)
    {
        case 0: return Qt::lightGray;
        case 1: return Qt::green;
        case 2: return Qt::yellow;
        case 3: return Qt::magenta;
        case 4: return Qt::cyan;
        case 5: return Qt::red;
        case 6: return Qt::darkYellow;
        case 7: return Qt::darkMagenta;
        case 8: return Qt::darkCyan;
        case 9: return Qt::darkRed;
        default: return Qt::red;
    }
}


QString Player::nameForOwner(uint inOwnerId)
{
    switch(inOwnerId)
    {
        case 0: return QString("unsettled");
        case 1: return QString("Human Player");
        default: return QString("Enemy %1").arg(inOwnerId);
    }
}
