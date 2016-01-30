/* This File is part of WaterWorld. License is GNU GPL Version 3.
 * Please see https://github.com/ngc42/WaterWorld/blob/master/LICENSE for details.
 * WaterWorld is (C) 2016 by Eike Lange (eike@ngc42.de)
 */


#ifndef UNIVERSESCENE_H
#define UNIVERSESCENE_H

#include <QGraphicsScene>

class UniverseScene : public QGraphicsScene
{
    Q_OBJECT
public:
    explicit UniverseScene(const QRectF inSceneRect, QObject *inParent = 0);

signals:

public slots:
};

#endif // UNIVERSESCENE_H
