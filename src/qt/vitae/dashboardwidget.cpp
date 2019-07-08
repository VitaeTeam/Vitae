#include "qt/vitae/dashboardwidget.h"
#include "qt/vitae/forms/ui_dashboardwidget.h"
#include "qt/vitae/sendconfirmdialog.h"
#include "qt/vitae/txrow.h"
#include "qt/vitae/qtutils.h"
#include "guiutil.h"
#include "walletmodel.h"
#include "optionsmodel.h"
#include <QPainter>
#include <QModelIndex>
#include <QList>
#include <QGraphicsLayout>

#define DECORATION_SIZE 65
#define NUM_ITEMS 3

#include "moc_dashboardwidget.cpp"

DashboardWidget::DashboardWidget(VITAEGUI* parent) :
    PWidget(parent),
    ui(new Ui::DashboardWidget)
{
    ui->setupUi(this);

    txHolder = new TxViewHolder(isLightTheme());
    txViewDelegate = new FurAbstractListItemDelegate(
                DECORATION_SIZE,
                txHolder,
                this
    );

    this->setStyleSheet(parent->styleSheet());
    this->setContentsMargins(0,0,0,0);

    // Containers
    setCssProperty(this, "container");
    setCssProperty(ui->left, "container");
    ui->left->setContentsMargins(0,0,0,0);
    setCssProperty(ui->right, "container-right");
    ui->right->setContentsMargins(20,20,20,0);

    // Title
    ui->labelTitle2->setText(tr("Staking Rewards"));
    setCssTitleScreen(ui->labelTitle);
    setCssTitleScreen(ui->labelTitle2);

    /* Subtitle */
    ui->labelSubtitle->setText(tr("You can view your account's history"));
    setCssSubtitleScreen(ui->labelSubtitle);

    // Staking Information
    ui->labelMessage->setText(tr("Amount of PIV and zPIV staked."));
    setCssSubtitleScreen(ui->labelMessage);
    setCssProperty(ui->labelSquarePiv, "square-chart-piv");
    setCssProperty(ui->labelSquarezPiv, "square-chart-zpiv");
    setCssProperty(ui->labelPiv, "text-chart-piv");
    setCssProperty(ui->labelZpiv, "text-chart-zpiv");

    // Staking Amount
    QFont fontBold;
    fontBold.setWeight(QFont::Bold);

    setCssProperty(ui->labelChart, "legend-chart");

    ui->labelAmountZpiv->setText("0 zPIV");
    ui->labelAmountPiv->setText("0 PIV");
    setCssProperty(ui->labelAmountPiv, "text-stake-piv-disable");
    setCssProperty(ui->labelAmountZpiv, "text-stake-zpiv-disable");

    setCssProperty({ui->pushButtonAll,  ui->pushButtonMonth, ui->pushButtonYear}, "btn-check-time");
    setCssProperty({ui->comboBoxMonths,  ui->comboBoxYears}, "btn-combo-chart-selected");

    ui->comboBoxMonths->setView(new QListView());
    ui->comboBoxMonths->setStyleSheet("selection-background-color:transparent; selection-color:transparent;");
    ui->comboBoxYears->setView(new QListView());
    ui->comboBoxYears->setStyleSheet("selection-background-color:transparent; selection-color:transparent;");
    ui->pushButtonYear->setChecked(true);
    setChartShow(YEAR);

    setCssProperty(ui->pushButtonChartArrow, "btn-chart-arrow");

    connect(ui->comboBoxYears, SIGNAL(currentIndexChanged(const QString&)), this,SLOT(onChartYearChanged(const QString&)));

    // Sort Transactions
    setCssProperty(ui->comboBoxSort, "btn-combo");
    ui->comboBoxSort->setEditable(true);
    SortEdit* lineEdit = new SortEdit(ui->comboBoxSort);
    lineEdit->setReadOnly(true);
    lineEdit->setAlignment(Qt::AlignRight);
    ui->comboBoxSort->setLineEdit(lineEdit);
    ui->comboBoxSort->setStyleSheet("selection-background-color:transparent; selection-color:transparent;");
    connect(lineEdit, SIGNAL(Mouse_Pressed()), this, SLOT(onSortTxPressed()));
    ui->comboBoxSort->setView(new QListView());
    ui->comboBoxSort->addItem("Date");
    ui->comboBoxSort->addItem("Amount");
    //ui->comboBoxSort->addItem("Sent");
    //ui->comboBoxSort->addItem("Received");
    connect(ui->comboBoxSort, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(onSortChanged(const QString&)));

    // transactions
    setCssProperty(ui->listTransactions, "container");
    ui->listTransactions->setItemDelegate(txViewDelegate);
    ui->listTransactions->setIconSize(QSize(DECORATION_SIZE, DECORATION_SIZE));
    ui->listTransactions->setMinimumHeight(NUM_ITEMS * (DECORATION_SIZE + 2));
    ui->listTransactions->setAttribute(Qt::WA_MacShowFocusRect, false);
    ui->listTransactions->setSelectionBehavior(QAbstractItemView::SelectRows);

    // Sync Warning
    ui->layoutWarning->setVisible(true);
    ui->lblWarning->setText(tr("Please wait until the wallet is fully synced to see your correct balance"));
    setCssProperty(ui->lblWarning, "text-warning");
    setCssProperty(ui->imgWarning, "ic-warning");

    //Empty List
    ui->emptyContainer->setVisible(false);
    setCssProperty(ui->pushImgEmpty, "img-empty-transactions");

    ui->labelEmpty->setText(tr("No transactions yet"));
    setCssProperty(ui->labelEmpty, "text-empty");

    setCssProperty(ui->chartContainer, "container-chart");

    setCssProperty(ui->pushImgEmptyChart, "img-empty-staking-on");

    ui->btnHowTo->setText(tr("How to get PIV or zPIV"));
    setCssBtnSecondary(ui->btnHowTo);

    ui->labelEmptyChart->setText(tr("Staking off"));
    setCssProperty(ui->labelEmptyChart, "text-empty");

    ui->labelMessageEmpty->setText(tr("You can activate and deactivate the Staking mode in the status bar at the top right of the wallet"));
    setCssSubtitleScreen(ui->labelMessageEmpty);

    // Chart State
    ui->layoutChart->setVisible(false);
    ui->emptyContainerChart->setVisible(true);
    setShadow(ui->layoutShadow);

    connect(ui->listTransactions, SIGNAL(clicked(QModelIndex)), this, SLOT(handleTransactionClicked(QModelIndex)));
    if (window)
        connect(window, SIGNAL(windowResizeEvent(QResizeEvent*)), this, SLOT(windowResizeEvent(QResizeEvent*)));

    connect(ui->pushButtonYear, &QPushButton::clicked, [this](){setChartShow(YEAR);});
    connect(ui->pushButtonMonth, &QPushButton::clicked, [this](){setChartShow(MONTH);});
    connect(ui->pushButtonAll, &QPushButton::clicked, [this](){setChartShow(ALL);});
}

void DashboardWidget::setChartShow(ChartShowType type) {
    this->chartShow = type;
    if (chartShow == MONTH) {
        ui->containerChartArrow->setVisible(true);
    } else {
        ui->containerChartArrow->setVisible(false);
    }
    if (isChartInitialized) refreshChart();
}

void DashboardWidget::handleTransactionClicked(const QModelIndex &index){

    ui->listTransactions->setCurrentIndex(index);
    QModelIndex rIndex = filter->mapToSource(index);

    window->showHide(true);
    TxDetailDialog *dialog = new TxDetailDialog(window, false);
    dialog->setData(walletModel, rIndex);
    dialog->adjustSize();
    openDialogWithOpaqueBackgroundY(dialog, window, 3, 17);

    // Back to regular status
    ui->listTransactions->scrollTo(index);
    ui->listTransactions->clearSelection();
    ui->listTransactions->setFocus();
    dialog->deleteLater();
}

void DashboardWidget::loadWalletModel(){
    if (walletModel && walletModel->getOptionsModel()) {
        txModel = walletModel->getTransactionTableModel();
        // Set up transaction list
        filter = new TransactionFilterProxy();
        filter->setDynamicSortFilter(true);
        filter->setSortCaseSensitivity(Qt::CaseInsensitive);
        filter->setFilterCaseSensitivity(Qt::CaseInsensitive);
        filter->setSortRole(Qt::EditRole);
        filter->setSourceModel(txModel);
        filter->sort(TransactionTableModel::Date, Qt::DescendingOrder);
        txHolder->setFilter(filter);
        ui->listTransactions->setModel(filter);
        ui->listTransactions->setModelColumn(TransactionTableModel::ToAddress);

        if(txModel->size() == 0){
            ui->emptyContainer->setVisible(true);
            ui->listTransactions->setVisible(false);
            connect(ui->pushImgEmpty, SIGNAL(clicked()), window, SLOT(openFAQ()));
            connect(ui->btnHowTo, SIGNAL(clicked()), window, SLOT(openFAQ()));
        }
        connect(txModel, &TransactionTableModel::txArrived, this, &DashboardWidget::onTxArrived);

        // chart filter
        stakesFilter = new TransactionFilterProxy();
        stakesFilter->setSourceModel(txModel);
        stakesFilter->sort(TransactionTableModel::Date, Qt::AscendingOrder);
        stakesFilter->setOnlyStakes(true);
        loadChart();
    }
    // update the display unit, to not use the default ("PIV")
    updateDisplayUnit();
}

void DashboardWidget::onTxArrived(const QString& hash) {
    showList();
    if (hasStakes() && walletModel->isCoinStakeMine(hash)) {
        refreshChart();
    }
}

void DashboardWidget::showList(){
    if (ui->emptyContainer->isVisible()) {
        ui->emptyContainer->setVisible(false);
        ui->listTransactions->setVisible(true);
    }
}

void DashboardWidget::updateDisplayUnit() {
    if (walletModel && walletModel->getOptionsModel()) {
        nDisplayUnit = walletModel->getOptionsModel()->getDisplayUnit();
        txHolder->setDisplayUnit(nDisplayUnit);
        ui->listTransactions->update();
    }
}

void DashboardWidget::onSortTxPressed(){
    ui->comboBoxSort->showPopup();
}

void DashboardWidget::onSortChanged(const QString& value){
    if(!value.isNull()) {
        if (value == "Amount")
            filter->sort(TransactionTableModel::Amount, Qt::DescendingOrder);
        else if (value == "Date")
            filter->sort(TransactionTableModel::Date, Qt::DescendingOrder);
        ui->listTransactions->update();
    }
}

void DashboardWidget::walletSynced(bool sync){
    if (this->isSync != sync) {
        this->isSync = sync;
        ui->layoutWarning->setVisible(!this->isSync);
    }
}

void DashboardWidget::changeTheme(bool isLightTheme, QString& theme){
    static_cast<TxViewHolder*>(this->txViewDelegate->getRowFactory())->isLightTheme = isLightTheme;
    if (chart) this->changeChartColors();
}

const QStringList monthsNames = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

void DashboardWidget::loadChart(){
    if (hasStakes()) {
        if (!chart) {
            ui->layoutChart->setVisible(true);
            ui->emptyContainerChart->setVisible(false);
            initChart();
            QDate currentDate = QDate::currentDate();
            monthFilter = currentDate.month();
            yearFilter = currentDate.year();
            for (int i = 1; i < 13; ++i) ui->comboBoxMonths->addItem(QString(monthsNames[i-1]), QVariant(i));
            ui->comboBoxMonths->setCurrentIndex(monthFilter - 1);
            connect(ui->comboBoxMonths, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(onChartMonthChanged(const QString&)));
            connect(ui->pushButtonChartArrow, SIGNAL(clicked()), this, SLOT(onChartArrowClicked()));
        }
        refreshChart();
        changeChartColors();
        isChartInitialized = true;
    } else {
        ui->layoutChart->setVisible(false);
        ui->emptyContainerChart->setVisible(true);
    }
}

void DashboardWidget::initChart() {
    chart = new QChart();
    axisX = new QBarCategoryAxis();
    axisY = new QValueAxis();

    // Chart style
    chart->legend()->setVisible(false);
    chart->legend()->setAlignment(Qt::AlignTop);
    chart->layout()->setContentsMargins(0, 0, 0, 0);
    chart->setMargins({0, 0, 0, 0});
    chart->setBackgroundRoundness(0);
    // Axis
    chart->addAxis(axisX, Qt::AlignBottom);
    chart->addAxis(axisY, Qt::AlignRight);
    chart->setAnimationOptions(QChart::SeriesAnimations);

    chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setRubberBand( QChartView::HorizontalRubberBand );
    chartView->setContentsMargins(0,0,0,0);

    QHBoxLayout *baseScreensContainer = new QHBoxLayout(this);
    baseScreensContainer->setMargin(0);
    baseScreensContainer->addWidget(chartView);
    ui->chartContainer->setLayout(baseScreensContainer);
    ui->chartContainer->setContentsMargins(0,0,0,0);
    setCssProperty(ui->chartContainer, "container-chart");
}

void DashboardWidget::changeChartColors(){
    QColor gridLineColorX;
    QColor linePenColorY;
    QColor backgroundColor;
    QColor gridY;
    if(isLightTheme()){
        gridLineColorX = QColor(255,255,255);
        linePenColorY = gridLineColorX;
        backgroundColor = linePenColorY;
        axisY->setGridLineColor(QColor("#1a000000"));
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
}

// pair PIV, zPIV
QMap<int, std::pair<qint64, qint64>> DashboardWidget::getAmountBy() {
    if (chartShow != ALL) {
        bool filterByMonth = false;
        if (monthFilter != 0 && chartShow == MONTH) {
            filterByMonth = true;
        }
        if (yearFilter != 0) {
            if (filterByMonth) {
                QDate monthFirst = QDate(yearFilter, monthFilter, 1);
                stakesFilter->setDateRange(
                        QDateTime(monthFirst),
                        QDateTime(QDate(yearFilter, monthFilter, monthFirst.daysInMonth()))
                );
            } else {
                stakesFilter->setDateRange(
                        QDateTime(QDate(yearFilter, 1, 1)),
                        QDateTime(QDate(yearFilter, 12, 31))
                );
            }
        } else if (filterByMonth) {
            QDate currentDate = QDate::currentDate();
            QDate monthFirst = QDate(currentDate.year(), monthFilter, 1);
            stakesFilter->setDateRange(
                    QDateTime(monthFirst),
                    QDateTime(QDate(currentDate.year(), monthFilter, monthFirst.daysInMonth()))
            );
            ui->comboBoxYears->setCurrentText(QString::number(currentDate.year()));
        } else {
            stakesFilter->clearDateRange();
        }
    } else {
        stakesFilter->clearDateRange();
    }
    int size = stakesFilter->rowCount();
    QMap<int, std::pair<qint64, qint64>> amountBy;
    // Get all of the stakes
    for (int i = 0; i < size; ++i) {
        QModelIndex modelIndex = stakesFilter->index(i, TransactionTableModel::ToAddress);
        qint64 amount = llabs(modelIndex.data(TransactionTableModel::AmountRole).toLongLong());
        QDate date = modelIndex.data(TransactionTableModel::DateRole).toDateTime().date();
        bool isPiv = modelIndex.data(TransactionTableModel::TypeRole).toInt() != TransactionRecord::StakeZVIT;

        int time = 0;
        switch (chartShow) {
            case YEAR: {
                time = date.month();
                break;
            }
            case ALL: {
                time = date.year();
                break;
            }
            case MONTH: {
                time = date.day();
                break;
            }
            default:
                inform(tr("Error loading chart, invalid show option"));
                return amountBy;
        }
        if (amountBy.contains(time)) {
            if (isPiv) {
                amountBy[time].first += amount;
            } else
                amountBy[time].second += amount;
        } else {
            if (isPiv) {
                amountBy[time] = std::make_pair(amount, 0);
            } else {
                amountBy[time] = std::make_pair(0, amount);
                hasZpivStakes = true;
            }
        }
    }
    return amountBy;
}

void DashboardWidget::onChartYearChanged(const QString& yearStr) {
    if (isChartInitialized) {
        int newYear = yearStr.toInt();
        if (newYear != yearFilter) {
            yearFilter = newYear;
            refreshChart();
        }
    }
}

void DashboardWidget::onChartMonthChanged(const QString& monthStr) {
    if (isChartInitialized) {
        int newMonth = ui->comboBoxMonths->currentData().toInt();
        if (newMonth != monthFilter) {
            monthFilter = newMonth;
            refreshChart();
        }
    }
}

void DashboardWidget::refreshChart(){
    if (chart) {
        if (chart->series().size() > 0)
            chart->removeAllSeries();
        axisX->clear();
    }
    // init sets
    set0 = new QBarSet("PIV");
    set1 = new QBarSet("zPIV");
    set0->setColor(QColor(92,75,125));
    set1->setColor(QColor(176,136,255));

    series = new QBarSeries();
    series->attachAxis(axisX);
    series->attachAxis(axisY);

    // pair PIV, zPIV
    amountsByCache = getAmountBy();

    QStringList xLabels;
    isChartMin = width() < 1300;
    bool withMonthNames = !isChartMin && (chartShow == YEAR);

    qreal maxValue = 0;
    qint64 totalPiv = 0;
    qint64 totalZpiv = 0;

    QList<qreal> valuesPiv;
    QList<qreal> valueszPiv;

    std::pair<int,int> range = getChartRange(amountsByCache);
    bool isOrderedByMonth = chartShow == MONTH;
    int daysInMonth = QDate(yearFilter, monthFilter, 1).daysInMonth();

    for (int j = range.first; j < range.second; j++) {
        int num = (isOrderedByMonth && j > daysInMonth) ? (j % daysInMonth) : j;
        qreal piv = 0;
        qreal zpiv = 0;
        if (amountsByCache.contains(num)) {
            std::pair <qint64, qint64> pair = amountsByCache[num];
            piv = (pair.first != 0) ? pair.first / 100000000 : 0;
            zpiv = (pair.second != 0) ? pair.second / 100000000 : 0;
            totalPiv += pair.first;
            totalZpiv += pair.second;
        }

        xLabels << ((withMonthNames) ? monthsNames[num - 1] : QString::number(num));
        valuesPiv.append(piv);
        valueszPiv.append(zpiv);

        int max = std::max(piv, zpiv);
        if (max > maxValue) {
            maxValue = max;
        }
    }

    set0->append(valuesPiv);
    set1->append(valueszPiv);

    // Total
    nDisplayUnit = walletModel->getOptionsModel()->getDisplayUnit();
    if (totalPiv > 0 || totalZpiv > 0) {
        setCssProperty(ui->labelAmountPiv, "text-stake-piv");
        setCssProperty(ui->labelAmountZpiv, "text-stake-zpiv");
    } else {
        setCssProperty(ui->labelAmountPiv, "text-stake-piv-disable");
        setCssProperty(ui->labelAmountZpiv, "text-stake-zpiv-disable");
    }
    forceUpdateStyle({ui->labelAmountPiv, ui->labelAmountZpiv});
    ui->labelAmountPiv->setText(GUIUtil::formatBalance(totalPiv, nDisplayUnit));
    ui->labelAmountZpiv->setText(GUIUtil::formatBalance(totalZpiv, nDisplayUnit, true));

    series->append(set0);
    if(hasZpivStakes)
        series->append(set1);
    chart->addSeries(series);

    // bar width
    if (chartShow == YEAR)
        series->setBarWidth(0.8);
    else {
        series->setBarWidth(0.3);
    }
    axisX->append(xLabels);
    axisY->setRange(0,maxValue);

    // Controllers
    switch (chartShow) {
        case ALL: {
            ui->container_chart_dropboxes->setVisible(false);
            break;
        }
        case YEAR: {
            ui->container_chart_dropboxes->setVisible(true);
            ui->containerBoxMonths->setVisible(false);
            break;
        }
        case MONTH: {
            ui->container_chart_dropboxes->setVisible(true);
            ui->containerBoxMonths->setVisible(true);
            break;
        }
        default: break;
    }

    // Refresh years filter, first address created is the start
    int yearStart = QDateTime::fromTime_t(static_cast<uint>(walletModel->getCreationTime())).date().year();
    int currentYear = QDateTime::currentDateTime().date().year();

    QString selection;
    if (ui->comboBoxYears->count() > 0) {
        selection = ui->comboBoxYears->currentText();
        isChartInitialized = false;
    }
    ui->comboBoxYears->clear();
    if (yearStart == currentYear) {
        ui->comboBoxYears->addItem(QString::number(currentYear));
    } else {
        for (int i = yearStart; i < (currentYear + 1); ++i)ui->comboBoxYears->addItem(QString::number(i));
    }

    if (!selection.isEmpty()) {
        ui->comboBoxYears->setCurrentText(selection);
        isChartInitialized = true;
    } else {
        ui->comboBoxYears->setCurrentText(QString::number(currentYear));
    }
}

std::pair<int, int> DashboardWidget::getChartRange(QMap<int, std::pair<qint64, qint64>> amountsBy) {
    switch (chartShow) {
        case YEAR:
            return std::make_pair(1, 13);
        case ALL: {
            QList<int> keys = amountsBy.uniqueKeys();
            qSort(keys);
            return std::make_pair(keys.first(), keys.last() + 1);
        }
        case MONTH:
            return std::make_pair(dayStart, dayStart + 9);
        default:
            inform(tr("Error loading chart, invalid show option"));
            return std::make_pair(0, 0);
    }
}

void DashboardWidget::updateAxisX(const QStringList* args) {
    axisX->clear();
    QStringList months;
    std::pair<int,int> range = getChartRange(amountsByCache);
    if (args) {
        months = *args;
    } else {
        for (int i = range.first; i < range.second; i++) months << QString::number(i);
    }
    axisX->append(months);
}

void DashboardWidget::onChartArrowClicked() {
    dayStart--;
    if (dayStart == 0) {
        dayStart = QDate(yearFilter, monthFilter, 1).daysInMonth();
    }
    refreshChart();
}

void DashboardWidget::windowResizeEvent(QResizeEvent *event){
    if (hasStakes() > 0 && axisX) {
        if (width() > 1300) {
            if (isChartMin) {
                isChartMin = false;
                switch (chartShow) {
                    case YEAR: {
                        updateAxisX(&monthsNames);
                        break;
                    }
                    case ALL: break;
                    case MONTH: {
                        updateAxisX();
                        break;
                    }
                    default:
                        inform(tr("Error loading chart, invalid show option"));
                        return;
                }
                chartView->repaint();
            }
        } else {
            if (!isChartMin) {
                updateAxisX();
                isChartMin = true;
            }
        }
    }
}

bool DashboardWidget::hasStakes() {
    return stakesFilter->rowCount() > 0;
}

DashboardWidget::~DashboardWidget(){
    delete ui;
    delete chart;
}
