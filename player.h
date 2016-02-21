/* This File is part of WaterWorld. License is GNU GPL Version 3.
 * Please see https://github.com/ngc42/WaterWorld/blob/master/LICENSE for details.
 * WaterWorld is (C) 2016 by Eike Lange (eike@ngc42.de)
 */


#ifndef PLAYER_H
#define PLAYER_H


#include <QColor>
#include <QString>


class Player
{
public:

    /**
     * @brief The PlayerEnum enum
     *
     * instead of using hardcoded number, this enum describes
     * constants for owner-id. Enemies starting from PLAYER_ENEMY_BASE.
     */
    enum PlayerEnum {PLAYER_UNSETTLED = 0, PLAYER_HUMAN = 1, PLAYER_ENEMY_BASE = 2};

    Player(uint inOwner);

    uint owner() const { return m_owner; }

    static QColor colorForOwner(uint inOwnerId);

    static QString nameForOwner(uint inOwnerId);

private:
    uint m_owner;
};

#endif // PLAYER_H
