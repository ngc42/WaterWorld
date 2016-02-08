/* This File is part of WaterWorld. License is GNU GPL Version 3.
 * Please see https://github.com/ngc42/WaterWorld/blob/master/LICENSE for details.
 * WaterWorld is (C) 2016 by Eike Lange (eike@ngc42.de)
 */


#ifndef OVERVIEWDIALOG_H
#define OVERVIEWDIALOG_H


#include "isle.h"
#include "ship.h"
#include <QDialog>
#include <QList>


namespace Ui {
class OverviewDialog;
}


class OverviewDialog : public QDialog
{
    Q_OBJECT

public:
    // inNumPlayer: human and computer player
    explicit OverviewDialog(const uint inNumPlayer, QWidget *inParent = 0);
    ~OverviewDialog();

    void updateIsleInfos(QList<IsleInfo> inIsleInfos);
    void updateShipInfos(QList<ShipInfo> inShipInfos);

private:
    Ui::OverviewDialog *m_ui;

    uint m_numPlayer;       // number of player (human and computer)
};


#endif // OVERVIEWDIALOG_H
