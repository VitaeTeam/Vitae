#ifndef DASHBOARDWIDGET_H
#define DASHBOARDWIDGET_H

#include "qt/vitae/furabstractlistitemdelegate.h"
#include "qt/vitae/furlistrow.h"
#include "transactiontablemodel.h"
#include "qt/vitae/txviewholder.h"

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

class DashboardWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DashboardWidget(VITAEGUI* _window, QWidget *parent = nullptr);
    ~DashboardWidget();

    void setWalletModel(WalletModel *model);
    void loadChart();
private slots:
    void handleTransactionClicked(const QModelIndex &index);

    void changeTheme(bool isLightTheme, QString &theme);
    void changeChartColors();
    void onSortTxPressed();
    void updateDisplayUnit();
    void showList();
private:
    Ui::DashboardWidget *ui;
    VITAEGUI* window;
    // Painter delegate
    FurAbstractListItemDelegate* txViewDelegate;
    TxViewHolder* txHolder;
    // Model
    WalletModel* walletModel;
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
