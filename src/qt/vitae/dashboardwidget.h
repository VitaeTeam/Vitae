// Copyright (c) 2019 The PIVX developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef DASHBOARDWIDGET_H
#define DASHBOARDWIDGET_H

#include "qt/vitae/pwidget.h"
#include "qt/vitae/furabstractlistitemdelegate.h"
#include "qt/vitae/furlistrow.h"
#include "transactiontablemodel.h"
#include "qt/vitae/txviewholder.h"
#include "transactionfilterproxy.h"

#include <iostream>
#include <cstdlib>

#include <QWidget>
#include <QLineEdit>
#include <QMap>

#include <QtCharts/QChartView>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QBarSet>
#include <QtCharts/QChart>
#include <QtCharts/QValueAxis>

QT_CHARTS_USE_NAMESPACE

using namespace QtCharts;

class VITAEGUI;
class WalletModel;

namespace Ui {
class DashboardWidget;
}

class SortEdit : public QLineEdit{
    Q_OBJECT
public:
    explicit SortEdit(QWidget* parent = nullptr) : QLineEdit(parent){}

    inline void mousePressEvent(QMouseEvent *) override{
        emit Mouse_Pressed();
    }

    ~SortEdit() override{}

signals:
    void Mouse_Pressed();

};

enum SortTx {
    DATE_ASC = 0,
    DATE_DESC = 1,
    AMOUNT_ASC = 2,
    AMOUNT_DESC = 3
};

enum ChartShowType {
    ALL,
    YEAR,
    MONTH,
    DAY
};

QT_BEGIN_NAMESPACE
class QModelIndex;
QT_END_NAMESPACE

class DashboardWidget : public PWidget
{
    Q_OBJECT

public:
    explicit DashboardWidget(VITAEGUI* _window);
    ~DashboardWidget();

    void loadWalletModel() override;
    void loadChart();

public slots:
    void walletSynced(bool isSync);
    /**
     * Show incoming transaction notification for new transactions.
     * The new items are those between start and end inclusive, under the given parent item.
    */
    void processNewTransaction(const QModelIndex& parent, int start, int /*end*/);
signals:
    /** Notify that a new transaction appeared */
    void incomingTransaction(const QString& date, int unit, const CAmount& amount, const QString& type, const QString& address);
private slots:
    void windowResizeEvent(QResizeEvent *event);
    void handleTransactionClicked(const QModelIndex &index);
    void changeTheme(bool isLightTheme, QString &theme) override;
    void changeChartColors();
    void onSortChanged(const QString&);
    void onSortTypeChanged(const QString& value);
    void updateDisplayUnit();
    void showList();
    void onTxArrived(const QString& hash);
    void onChartYearChanged(const QString&);
    void onChartMonthChanged(const QString&);
    void onChartArrowClicked();

private:
    Ui::DashboardWidget *ui;
    FurAbstractListItemDelegate* txViewDelegate;
    TransactionFilterProxy* filter;
    TransactionFilterProxy* stakesFilter;
    TxViewHolder* txHolder;
    TransactionTableModel* txModel;
    int nDisplayUnit = -1;
    bool isSync = false;

    // Chart
    bool isChartInitialized = false;
    QChartView *chartView = nullptr;
    QBarSeries *series = nullptr;
    QBarSet *set0 = nullptr;
    QBarSet *set1 = nullptr;

    QBarCategoryAxis *axisX = nullptr;
    QValueAxis *axisY = nullptr;

    QChart *chart = nullptr;
    bool isChartMin = false;
    ChartShowType chartShow = YEAR;
    int yearFilter = 0;
    int monthFilter = 0;
    int dayStart = 1;
    bool hasZpivStakes = false;

    QMap<int, std::pair<qint64, qint64>> amountsByCache;

    void initChart();
    void refreshChart();
    QMap<int, std::pair<qint64, qint64>> getAmountBy();
    void updateAxisX(const QStringList *arg = nullptr);
    void setChartShow(ChartShowType type);
    std::pair<int, int> getChartRange(QMap<int, std::pair<qint64, qint64>> amountsBy);
    bool hasStakes();
};

#endif // DASHBOARDWIDGET_H
