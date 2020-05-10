#ifndef DASHBOARDWIDGET_H
#define DASHBOARDWIDGET_H

#include "qt/vitae/pwidget.h"
#include "qt/vitae/furabstractlistitemdelegate.h"
#include "qt/vitae/furlistrow.h"
#include "transactiontablemodel.h"
#include "qt/vitae/txviewholder.h"
#include "transactionfilterproxy.h"

#include <QWidget>
#include <QLineEdit>

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

QT_BEGIN_NAMESPACE
class QModelIndex;
QT_END_NAMESPACE

class DashboardWidget : public PWidget
{
    Q_OBJECT

public:
    explicit DashboardWidget(VITAEGUI* _window, QWidget *parent = nullptr);
    ~DashboardWidget();

    void loadWalletModel() override;
    void loadChart();

public slots:
    void walletSynced(bool isSync);
private slots:
    void windowResizeEvent(QResizeEvent *event);
    void handleTransactionClicked(const QModelIndex &index);
    void changeTheme(bool isLightTheme, QString &theme) override;
    void changeChartColors();
    void onSortTxPressed();
    void onSortChanged(const QString&);
    void updateDisplayUnit();
    void showList();
    void onTxArrived();
    void openFAQ();
private:
    Ui::DashboardWidget *ui;
    // Painter delegate
    FurAbstractListItemDelegate* txViewDelegate;
    TransactionFilterProxy* filter;
    TransactionFilterProxy* stakesFilter;
    TxViewHolder* txHolder;
    TransactionTableModel* txModel;
    int nDisplayUnit = -1;

    // Chart
    QChartView *chartView = nullptr;
    QBarSeries *series = nullptr;
    QBarSet *set0 = nullptr;
    QBarSet *set1 = nullptr;

    QBarCategoryAxis *axisX = nullptr;
    QValueAxis *axisY = nullptr;

    QChart *chart = nullptr;
    bool isChartMin = false;
    bool isSync = false;

    void initChart();
    void refreshChart();
};

#endif // DASHBOARDWIDGET_H
