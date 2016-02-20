#ifndef PLAYER_H
#define PLAYER_H

#include <QColor>
#include <QString>

class Player
{
public:
    Player(uint inOwner);

    uint owner() const { return m_owner; }

    static QColor colorForOwner(uint inOwnerId);
    static QString nameForOwner(uint inOwnerId);

private:
    uint m_owner;



};

#endif // PLAYER_H
