#include "minimapview.h"
#include <QMouseEvent>
#include <QDebug>

MinimapView::MinimapView(QWidget *inParent) :
    QGraphicsView(inParent)
{
}


/**
 * @brief MinimapView::mousePressEvent - react on mouse press events inside of minimap
 * @param inMouseEvent
 *
 * This is the only reason, why this class exists:
 */
void MinimapView::mousePressEvent(QMouseEvent *inMouseEvent)
{
    qDebug()  << "Minimap sends Click: " << mapToScene( inMouseEvent->pos() );
    emit sigMinimapClicked( mapToScene(inMouseEvent->pos()) );
}
