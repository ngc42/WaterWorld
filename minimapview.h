/* This File is part of WaterWorld. License is GNU GPL Version 3.
 * Please see https://github.com/ngc42/WaterWorld/blob/master/LICENSE for details.
 * WaterWorld is (C) 2016 by Eike Lange (eike@ngc42.de)
 */

#ifndef MINIMAPVIEW_H
#define MINIMAPVIEW_H


#include <QGraphicsView>


class MinimapView : public QGraphicsView
{
    Q_OBJECT

public:
    explicit MinimapView(QWidget *inParent = 0);

protected:
    /**
     * @brief MinimapView::mousePressEvent - react on mouse press events inside of minimap
     * @param inMouseEvent
     *
     * This is the only reason, why this class exists:
     */
    void mousePressEvent(QMouseEvent *inMouseEvent);

signals:
    // send Click event over to someone else (universe might be interested, because of repositioning)
    void sigMinimapClicked(QPointF screenPos);
};

#endif // MINIMAPVIEW_H
