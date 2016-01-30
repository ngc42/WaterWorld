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
