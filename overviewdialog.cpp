/* This File is part of WaterWorld. License is GNU GPL Version 3.
 * Please see https://github.com/ngc42/WaterWorld/blob/master/LICENSE for details.
 * WaterWorld is (C) 2016 by Eike Lange (eike@ngc42.de)
 */


#include "overviewdialog.h"
#include "ui_overviewdialog.h"
#include <QStringList>
#include <QVector>
#include <QDebug>
#include <QTableWidgetItem>


OverviewDialog::OverviewDialog(const uint inNumPlayer, QWidget *inParent) :
    QDialog(inParent, Qt::Dialog| Qt::CustomizeWindowHint|Qt::WindowTitleHint ),
    m_ui(new Ui::OverviewDialog), m_numPlayer(inNumPlayer)
{
    m_ui->setupUi(this);

    // Player + "ALL" + "0" (No Player)
    m_ui->tableWidget->setRowCount(m_numPlayer + 2);
    m_ui->tableWidget->setColumnCount(3);

    // Vertical Table Header
    QStringList vertLabels;
    for(uint i = 0; i <= m_numPlayer; i++)
        vertLabels << QString("P %1").arg(i);
    vertLabels << "ALL";
    m_ui->tableWidget->setVerticalHeaderLabels(vertLabels);

    // Horizontal Table Header
    QStringList horzLabels;
    horzLabels << "Isle" << "Population" << "Ships";
    m_ui->tableWidget->setHorizontalHeaderLabels(horzLabels);

    // set table widget items for all columns
    for(uint i = 0; i < m_numPlayer + 2; i++)
    {
        // column isles
        QTableWidgetItem *it_0 = new QTableWidgetItem;
        m_ui->tableWidget->setItem(i, TC_ISLES, it_0);

        // column population
        QTableWidgetItem *it_1 = new QTableWidgetItem;
        m_ui->tableWidget->setItem(i, TC_POPULATION, it_1);

        // column ships
        QTableWidgetItem *it_2 = new QTableWidgetItem;
        m_ui->tableWidget->setItem(i, TC_SHIPS, it_2);
    }
}


OverviewDialog::~OverviewDialog()
{
    delete m_ui;
}


void OverviewDialog::updateIsleInfos(QList<IsleInfo> inIsleInfos)
{
    QVector<int> isleCount(m_numPlayer + 2, 0);
    QVector<int> populationCount(m_numPlayer + 2, 0);

    for(IsleInfo iInfo : inIsleInfos)
    {
        // counting isles for player iInfo.owner
        isleCount[iInfo.owner]++;
        // counting isles for all player in total
        isleCount[m_numPlayer + 1]++;

        populationCount[iInfo.owner] = populationCount.value(iInfo.owner) + iInfo.population;
        populationCount[m_numPlayer + 1] = populationCount.value(m_numPlayer + 1) + iInfo.population;
    }

    // display the infos in the table
    for(uint i = 0; i < m_numPlayer + 2; i++)
    {
        QString ic = QString("%1").arg( isleCount.value(i, -1) );
        QString pc = QString("%1").arg( populationCount.value(i, -1));

        m_ui->tableWidget->item(i, TC_ISLES)->setText(ic);
        m_ui->tableWidget->item(i, TC_POPULATION)->setText(pc);
    }
}


void OverviewDialog::updateShipInfos(QList<ShipInfo> inShipInfos)
{
    QVector<int> shipCount(m_numPlayer + 2, 0);

    // count
    for(ShipInfo sInfo : inShipInfos)
    {
        shipCount[sInfo.owner]++;
        shipCount[m_numPlayer + 1]++;
    }
    // display
    for(uint i = 0; i < m_numPlayer + 2; i++)
    {
        QString sc = QString("%1").arg( shipCount.value(i, -1) );
        m_ui->tableWidget->item(i, TC_SHIPS)->setText(sc);
    }
}
