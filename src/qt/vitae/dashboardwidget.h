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
/*
#include <QBarCategoryAxis>
#include <QBarSet>
#include <QChart>
#include <QValueAxis>

 QT_CHARTS_USE_NAMESPACE
 */

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

    void loadWalletModel() override ;
    void loadChart();
private slots:
    void handleTransactionClicked(const QModelIndex &index);

    void changeTheme(bool isLightTheme, QString &theme);
    void changeChartColors();
    void onSortTxPressed();
    void onSortChanged(const QString&);
    void updateDisplayUnit();
    void showList();
    void openFAQ();
private:
    Ui::DashboardWidget *ui;
    // Painter delegate
    FurAbstractListItemDelegate* txViewDelegate;
    TransactionFilterProxy* filter;
    TxViewHolder* txHolder;
    TransactionTableModel* txModel;
    int nDisplayUnit = -1;

    /*
    // Chart
    QBarSet *set0;
    QBarSet *set1;

    QBarCategoryAxis *axisX;
    QValueAxis *axisY;

    QChart *chart;
     */
};

#endif // DASHBOARDWIDGET_H
