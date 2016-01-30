/* This File is part of WaterWorld. License is GNU GPL Version 3.
 * Please see https://github.com/ngc42/WaterWorld/blob/master/LICENSE for details.
 * WaterWorld is (C) 2016 by Eike Lange (eike@ngc42.de)
 */


#include "minimapview.h"
#include <QMouseEvent>
#include <QDebug>


MinimapView::MinimapView(QWidget *inParent) :
    QGraphicsView(inParent)
{
}


void MinimapView::mousePressEvent(QMouseEvent *inMouseEvent)
{
    qDebug()  << "Minimap sends Click: " << mapToScene( inMouseEvent->pos() );
    emit sigMinimapClicked( mapToScene(inMouseEvent->pos()) );
}
