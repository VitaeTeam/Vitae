#include "qt/pivx/dashboardwidget.h"
#include "qt/pivx/forms/ui_dashboardwidget.h"
#include "qt/pivx/txdetaildialog.h"
#include "qt/pivx/txrow.h"
#include "qt/pivx/qtutils.h"
#include "qt/pivx/txviewholder.h"
#include "walletmodel.h"
#include <QFile>
#include <QAbstractItemDelegate>
#include <QPainter>
#include <QSettings>
#include <QModelIndex>
#include <QObject>
#include <QPaintEngine>

#include "QGraphicsDropShadowEffect"
#include <iostream>

/*
// Chart
#include <QtCharts/QChartView>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QLegend>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QValueAxis>
 */
#include <QGraphicsLayout>

#define DECORATION_SIZE 60
#define NUM_ITEMS 3

#include "moc_dashboardwidget.cpp"

DashboardWidget::DashboardWidget(PIVXGUI* _window, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DashboardWidget),
    window(_window)
{
    ui->setupUi(this);

    txViewDelegate = new FurAbstractListItemDelegate(
                DECORATION_SIZE,
                new TxViewHolder(isLightTheme()),
                this
                );

    // Load css.
    this->setStyleSheet(parent->styleSheet());
    this->setContentsMargins(0,0,0,0);

    setProperty("cssClass", "container");

    // Containers
    ui->left->setProperty("cssClass", "container");
    ui->left->setContentsMargins(0,0,0,0);
    ui->right->setProperty("cssClass", "container-right");
    ui->right->setContentsMargins(20,20,20,20);

    // Title
    ui->labelTitle->setProperty("cssClass", "text-title-screen");

    ui->labelTitle2->setText("Staking Rewards");
    ui->labelTitle2->setProperty("cssClass", "text-title-screen");

    /* Subtitle */

    ui->labelSubtitle->setText("You can view your account's history");
    ui->labelSubtitle->setProperty("cssClass", "text-subtitle");


    // Staking Information
    ui->labelMessage->setText("Amount of PIV and zPIV staked.");
    ui->labelMessage->setProperty("cssClass", "text-subtitle");
    ui->labelSquarePiv->setProperty("cssClass", "square-chart-piv");
    ui->labelSquarezPiv->setProperty("cssClass", "square-chart-zpiv");
    ui->labelPiv->setProperty("cssClass", "text-chart-piv");
    ui->labelZpiv->setProperty("cssClass", "text-chart-zpiv");

    // Staking Amount
    QFont fontBold;
    fontBold.setWeight(QFont::Bold);

    ui->labelChart->setProperty("cssClass", "legend-chart");

    ui->labelAmountPiv->setProperty("cssClass", "text-stake-piv");
    ui->labelAmountPiv->setText("9 PIV");

    ui->labelAmountZpiv->setProperty("cssClass", "text-stake-zpiv");
    ui->labelAmountZpiv->setText("9 zPIV");


    // Chart
    //ui->verticalWidgetChart->setProperty("cssClass", "container-chart");

    ui->pushButtonHour->setProperty("cssClass", "btn-check-time");
    ui->pushButtonDay->setProperty("cssClass", "btn-check-time");
    ui->pushButtonWeek->setProperty("cssClass", "btn-check-time");
    ui->pushButtonMonth->setProperty("cssClass", "btn-check-time");
    ui->pushButtonYear->setProperty("cssClass", "btn-check-time");


    // Sort Transactions

    ui->comboBoxSort->setProperty("cssClass", "btn-combo");

    QListView * listView = new QListView();

    ui->comboBoxSort->addItem("Amount");
    ui->comboBoxSort->addItem("Date");
    ui->comboBoxSort->addItem("Sent");

    ui->comboBoxSort->setView(listView);

    // transactions
    ui->listTransactions->setProperty("cssClass", "container");
    ui->listTransactions->setItemDelegate(txViewDelegate);
    ui->listTransactions->setIconSize(QSize(DECORATION_SIZE, DECORATION_SIZE));
    ui->listTransactions->setMinimumHeight(NUM_ITEMS * (DECORATION_SIZE + 2));
    ui->listTransactions->setAttribute(Qt::WA_MacShowFocusRect, false);
    ui->listTransactions->setSelectionBehavior(QAbstractItemView::SelectRows);

    //ui->listTransactions->setVisible(false);

    //Empty List

    ui->emptyContainer->setVisible(false);
    ui->pushImgEmpty->setProperty("cssClass", "img-empty-transactions");

    ui->labelEmpty->setText("No transactions yet");
    ui->labelEmpty->setProperty("cssClass", "text-empty");


    ui->chartContainer->setProperty("cssClass", "container-chart");
    // load chart
    loadChart();

    connect(ui->listTransactions, SIGNAL(clicked(QModelIndex)), this, SLOT(handleTransactionClicked(QModelIndex)));

    // Style
    connect(window, SIGNAL(themeChanged(bool, QString&)), this, SLOT(changeTheme(bool, QString&)));

}

void DashboardWidget::handleTransactionClicked(const QModelIndex &index){

    ui->listTransactions->setCurrentIndex(index);

    window->showHide(true);
    TxDetailDialog *dialog = new TxDetailDialog(window);
    openDialogWithOpaqueBackgroundY(dialog, window, 3, 6);

    // Back to regular status
    ui->listTransactions->scrollTo(index);
    ui->listTransactions->clearSelection();
    ui->listTransactions->setFocus();

}

void DashboardWidget::loadChart(){
    /*
    set0 = new QBarSet("PIV");
    set0->setColor(QColor(176,136,255));

    set1 = new QBarSet("zPIV");
    set1->setColor(QColor(92,75,125));

    *set0 << 4 << 2 << 4 << 6;
    *set1 << 6 << 9 << 3 << 6;

    QBarSeries *series = new QBarSeries();
    series->append(set0);
    series->append(set1);

    // bar width
    series->setBarWidth(0.8);

    chart = new QChart();
    chart->addSeries(series);
    // title
    //chart->setTitle("Simple barchart example");
    chart->setAnimationOptions(QChart::SeriesAnimations);

    QStringList categories;
    categories << "Jan" << "Feb" << "Mar" << "Apr";
    axisX = new QBarCategoryAxis();
    axisX->append(categories);
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);

    axisY = new QValueAxis();
    axisY->setRange(0,12);
    chart->addAxis(axisY, Qt::AlignRight);
    series->attachAxis(axisY);

    // Legend
    chart->legend()->setVisible(false);
    chart->legend()->setAlignment(Qt::AlignTop);

    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);

    QVBoxLayout *baseScreensContainer = new QVBoxLayout(this);
    baseScreensContainer->setMargin(0);
    ui->chartContainer->setLayout(baseScreensContainer);

    ui->chartContainer->layout()->addWidget(chartView);
    ui->chartContainer->setProperty("cssClass", "container-chart");

    // Set colors
    changeChartColors();

    // Chart margin removed.
    chart->layout()->setContentsMargins(0, 0, 0, 0);
    chart->setBackgroundRoundness(0);
     */
}

void DashboardWidget::changeChartColors(){
    /*
    // Colors
    QColor gridLineColorX;
    QColor linePenColorY;
    QColor backgroundColor;
    QColor gridY;
    if(isLightTheme()){
        gridLineColorX = QColor(255,255,255);
        linePenColorY = gridLineColorX;
        backgroundColor = linePenColorY;
    }else{
        gridY = QColor("#40ffffff");
        axisY->setGridLineColor(gridY);
        gridLineColorX = QColor(15,11,22);
        linePenColorY =  gridLineColorX;
        backgroundColor = linePenColorY;
    }

    axisX->setGridLineColor(gridLineColorX);
    axisY->setLinePenColor(linePenColorY);
    chart->setBackgroundBrush(QBrush(backgroundColor));
    set0->setBorderColor(gridLineColorX);
    set1->setBorderColor(gridLineColorX);
     */
}

void DashboardWidget::setWalletModel(WalletModel* model){
    txModel = model->getTransactionTableModel();
    ui->listTransactions->setModel(this->txModel);
}

void DashboardWidget::changeTheme(bool isLightTheme, QString& theme){
    // Change theme in all of the childs here..
    this->setStyleSheet(theme);
    static_cast<TxViewHolder*>(this->txViewDelegate->getRowFactory())->isLightTheme = isLightTheme;
    this->changeChartColors();
    updateStyle(this);
}

DashboardWidget::~DashboardWidget()
{
    delete ui;
}
