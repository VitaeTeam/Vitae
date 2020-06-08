// Copyright (c) 2019-2020 The PIVX developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "qt/vitae/privacywidget.h"
#include "qt/vitae/forms/ui_privacywidget.h"
#include "qt/vitae/qtutils.h"
#include "guiutil.h"
#include "qt/vitae/denomgenerationdialog.h"
#include "qt/vitae/txviewholder.h"
#include "walletmodel.h"
#include "optionsmodel.h"
#include "coincontroldialog.h"
#include "coincontrol.h"
#include "zvit/accumulators.h"

#define DECORATION_SIZE 65
#define NUM_ITEMS 3

PrivacyWidget::PrivacyWidget(VITAEGUI* parent) :
    PWidget(parent),
    ui(new Ui::PrivacyWidget)
{
    ui->setupUi(this);
    this->setStyleSheet(parent->styleSheet());

    /* Containers */
    setCssProperty(ui->left, "container");
    ui->left->setContentsMargins(0,20,0,0);
    setCssProperty(ui->right, "container-right");
    ui->right->setContentsMargins(20,10,20,20);

    /* Light Font */
    QFont fontLight;
    fontLight.setWeight(QFont::Light);

    /* Title */
    ui->labelTitle->setText(tr("Privacy"));
    setCssTitleScreen(ui->labelTitle);
    ui->labelTitle->setFont(fontLight);

    /* Button Group */
    ui->pushLeft->setText(tr("Convert"));
    setCssProperty(ui->pushLeft, "btn-check-left");
    ui->pushRight->setText(tr("Mint"));
    setCssProperty(ui->pushRight, "btn-check-right");

    /* Subtitle */
    ui->labelSubtitle1->setText(tr("Minting zVIT anonymizes your VIT by removing any\ntransaction history, making transactions untraceable "));
    setCssSubtitleScreen(ui->labelSubtitle1);

    ui->labelSubtitle2->setText(tr("Mint new zVIT or convert back to VIT"));
    setCssSubtitleScreen(ui->labelSubtitle2);
    ui->labelSubtitle2->setContentsMargins(0,2,0,0);
    setCssProperty(ui->labelSubtitleAmount, "text-title");

    ui->lineEditAmount->setPlaceholderText("0.00 VIT ");
    ui->lineEditAmount->setValidator(new QRegExpValidator(QRegExp("[0-9]+")));
    initCssEditLine(ui->lineEditAmount);

    /* Denom */
    ui->labelTitleDenom1->setText("Denom. with value 1:");
    setCssProperty(ui->labelTitleDenom1, "text-subtitle");
    ui->labelValueDenom1->setText("0x1 = 0 zVIT");
    setCssProperty(ui->labelValueDenom1, "text-body2");

    ui->labelTitleDenom5->setText("Denom. with value 5:");
    setCssProperty(ui->labelTitleDenom5, "text-subtitle");
    ui->labelValueDenom5->setText("0x5 = 0 zVIT");
    setCssProperty(ui->labelValueDenom5, "text-body2");

    ui->labelTitleDenom10->setText("Denom. with value 10:");
    setCssProperty(ui->labelTitleDenom10, "text-subtitle");
    ui->labelValueDenom10->setText("0x10 = 0 zVIT");
    setCssProperty(ui->labelValueDenom10, "text-body2");

    ui->labelTitleDenom50->setText("Denom. with value 50:");
    setCssProperty(ui->labelTitleDenom50, "text-subtitle");
    ui->labelValueDenom50->setText("0x50 = 0 zVIT");
    setCssProperty(ui->labelValueDenom50, "text-body2");

    ui->labelTitleDenom100->setText("Denom. with value 100:");
    setCssProperty(ui->labelTitleDenom100, "text-subtitle");
    ui->labelValueDenom100->setText("0x100 = 0 zVIT");
    setCssProperty(ui->labelValueDenom100, "text-body2");

    ui->labelTitleDenom500->setText("Denom. with value 500:");
    setCssProperty(ui->labelTitleDenom500, "text-subtitle");
    ui->labelValueDenom500->setText("0x500 = 0 zVIT");
    setCssProperty(ui->labelValueDenom500, "text-body2");

    ui->labelTitleDenom1000->setText("Denom. with value 1000:");
    setCssProperty(ui->labelTitleDenom1000, "text-subtitle");
    ui->labelValueDenom1000->setText("0x1000 = 0 zVIT");
    setCssProperty(ui->labelValueDenom1000, "text-body2");

    ui->labelTitleDenom5000->setText("Denom. with value 5000:");
    setCssProperty(ui->labelTitleDenom5000, "text-subtitle");
    ui->labelValueDenom5000->setText("0x5000 = 0 zVIT");
    setCssProperty(ui->labelValueDenom5000, "text-body2");

    ui->layoutDenom->setVisible(false);

    // List
    ui->labelListHistory->setText(tr("Last zVIT Movements"));
    setCssProperty(ui->labelListHistory, "text-title");

    //ui->emptyContainer->setVisible(false);
    setCssProperty(ui->pushImgEmpty, "img-empty-privacy");
    ui->labelEmpty->setText(tr("No transactions yet"));
    setCssProperty(ui->labelEmpty, "text-empty");

    // Buttons
    setCssBtnPrimary(ui->pushButtonSave);

    // Only Convert to VIT enabled.
    ui->containerViewPrivacyChecks->setVisible(false);
    onMintSelected(false);

    ui->btnTotalzVIT->setTitleClassAndText("btn-title-grey", "Total 0 zVIT");
    ui->btnTotalzVIT->setSubTitleClassAndText("text-subtitle", "Show denominations of zVIT owned.");
    ui->btnTotalzVIT->setRightIconClass("ic-arrow");

    ui->btnCoinControl->setTitleClassAndText("btn-title-grey", "Coin Control");
    ui->btnCoinControl->setSubTitleClassAndText("text-subtitle", "Select VIT outputs to mint into zVIT.");

    ui->btnDenomGeneration->setTitleClassAndText("btn-title-grey", "Denom Generation");
    ui->btnDenomGeneration->setSubTitleClassAndText("text-subtitle", "Select the denomination of the coins.");
    ui->btnDenomGeneration->setVisible(false);

    ui->btnRescanMints->setTitleClassAndText("btn-title-grey", "Rescan Mints");
    ui->btnRescanMints->setSubTitleClassAndText("text-subtitle", "Find mints in the blockchain.");

    ui->btnResetZerocoin->setTitleClassAndText("btn-title-grey", "Reset Spent zVIT");
    ui->btnResetZerocoin->setSubTitleClassAndText("text-subtitle", "Reset zerocoin database.");

    connect(ui->btnTotalzVIT, SIGNAL(clicked()), this, SLOT(onTotalZpivClicked()));
    connect(ui->btnCoinControl, SIGNAL(clicked()), this, SLOT(onCoinControlClicked()));
    connect(ui->btnDenomGeneration, SIGNAL(clicked()), this, SLOT(onDenomClicked()));
    connect(ui->btnRescanMints, SIGNAL(clicked()), this, SLOT(onRescanMintsClicked()));
    connect(ui->btnResetZerocoin, SIGNAL(clicked()), this, SLOT(onResetZeroClicked()));

    ui->pushRight->setChecked(true);
    connect(ui->pushLeft, &QPushButton::clicked, [this](){onMintSelected(false);});
    connect(ui->pushRight,  &QPushButton::clicked, [this](){onMintSelected(true);});

    // List
    setCssProperty(ui->listView, "container");
    txHolder = new TxViewHolder(isLightTheme());
    delegate = new FurAbstractListItemDelegate(
                DECORATION_SIZE,
                txHolder,
                this
    );

    ui->listView->setItemDelegate(delegate);
    ui->listView->setIconSize(QSize(DECORATION_SIZE, DECORATION_SIZE));
    ui->listView->setMinimumHeight(NUM_ITEMS * (DECORATION_SIZE + 2));
    ui->listView->setAttribute(Qt::WA_MacShowFocusRect, false);
    ui->listView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->listView->setLayoutMode(QListView::LayoutMode::Batched);
    ui->listView->setBatchSize(30);
    ui->listView->setUniformItemSizes(true);
}

void PrivacyWidget::loadWalletModel(){
    if(walletModel) {
        txModel = walletModel->getTransactionTableModel();
        // Set up transaction list
        filter = new TransactionFilterProxy();
        filter->setDynamicSortFilter(true);
        filter->setSortCaseSensitivity(Qt::CaseInsensitive);
        filter->setFilterCaseSensitivity(Qt::CaseInsensitive);
        filter->setSortRole(Qt::EditRole);
        filter->setShowZcTxes(true);
        filter->setSourceModel(txModel);
        filter->sort(TransactionTableModel::Date, Qt::DescendingOrder);
        txHolder->setDisplayUnit(walletModel->getOptionsModel()->getDisplayUnit());
        txHolder->setFilter(filter);
        ui->listView->setModel(filter);

        updateDisplayUnit();
        updateDenomsSupply();

        if (!txModel->hasZcTxes()) {
            ui->emptyContainer->setVisible(true);
            ui->listView->setVisible(false);
        }else{
            showList();
        }

        connect(ui->pushButtonSave, SIGNAL(clicked()), this, SLOT(onSendClicked()));
    }

}

void PrivacyWidget::onMintSelected(bool isMint){
    QString btnText;
    if(isMint){
        btnText = tr("Mint zVIT");
        ui->btnCoinControl->setVisible(true);
        ui->labelSubtitleAmount->setText(tr("Enter amount of VIT to mint into zVIT"));
    }else{
        btnText = tr("Convert back to VIT");
        ui->btnCoinControl->setVisible(false);
        ui->labelSubtitleAmount->setText(tr("Enter amount of zVIT to convert back into VIT"));
    }
    ui->pushButtonSave->setText(btnText);
}

void PrivacyWidget::updateDisplayUnit() {
    if (walletModel && walletModel->getOptionsModel()) {
        nDisplayUnit = walletModel->getOptionsModel()->getDisplayUnit();

        txHolder->setDisplayUnit(nDisplayUnit);
        ui->listView->update();
    }
}

void PrivacyWidget::showList(){
    ui->emptyContainer->setVisible(false);
    ui->listView->setVisible(true);
}

void PrivacyWidget::onTotalZpivClicked(){
    bool isVisible = ui->layoutDenom->isVisible();
    if(!isVisible){
        ui->layoutDenom->setVisible(true);
        ui->btnTotalzVIT->setRightIconClass("btn-dropdown", true);
    }else{
        ui->layoutDenom->setVisible(false);
        ui->btnTotalzVIT->setRightIconClass("ic-arrow", true);
    }
}

void PrivacyWidget::onSendClicked(){
    if (!walletModel || !walletModel->getOptionsModel())
        return;

    if(sporkManager.IsSporkActive(SPORK_20_ZEROCOIN_MAINTENANCE_MODE)) {
        warn(tr("Zerocoin"), tr("zVIT is currently undergoing maintenance"));
        return;
    }

    // Only convert enabled.
    bool isConvert = true;// ui->pushLeft->isChecked();

    if(!GUIUtil::requestUnlock(walletModel, AskPassphraseDialog::Context::Mint_zVIT, true)){
        inform(tr("You need to unlock the wallet to be able to %1 zVIT").arg(isConvert ? tr("convert") : tr("mint")));
        return;
    }

    bool isValid = true;
    CAmount value = GUIUtil::parseValue(
            ui->lineEditAmount->text(),
            walletModel->getOptionsModel()->getDisplayUnit(),
            &isValid
    );

    if (!isValid || value <= 0) {
        setCssEditLine(ui->lineEditAmount, false, true);
        inform(tr("Invalid value"));
        return;
    }

    setCssEditLine(ui->lineEditAmount, true, true);
    if(isConvert){
        spend(value);
    }else{
        mint(value);
    }
}

void PrivacyWidget::mint(CAmount value){
    std::string strError;
    if(!walletModel->mintCoins(value, CoinControlDialog::coinControl, strError)){
        inform(tr(strError.data()));
    }else{
        // Mint succeed
        inform(tr("zVIT minted successfully"));
        // clear
        ui->lineEditAmount->clear();
    }
}

void PrivacyWidget::spend(CAmount value){
    CZerocoinSpendReceipt receipt;
    std::vector<CZerocoinMint> selectedMints;
    bool mintChange = false;
    bool minimizeChange = false;

    if(!walletModel->convertBackZpiv(
            value,
            selectedMints,
            mintChange,
            minimizeChange,
            receipt
    )){
        inform(receipt.GetStatusMessage().data());
    }else{
        // Spend succeed
        inform(tr("zVIT converted back to VIT"));
        // clear
        ui->lineEditAmount->clear();
    }
}


void PrivacyWidget::onCoinControlClicked(){
    if(ui->pushRight->isChecked()) {
        if (walletModel->getBalance() > 0) {
            if (!coinControlDialog) {
                coinControlDialog = new CoinControlDialog();
                coinControlDialog->setModel(walletModel);
            } else {
                coinControlDialog->refreshDialog();
            }
            coinControlDialog->exec();
            ui->btnCoinControl->setActive(CoinControlDialog::coinControl->HasSelected());
        } else {
            inform(tr("You don't have any VIT to select."));
        }
    }
}

void PrivacyWidget::onDenomClicked(){
    showHideOp(true);
    DenomGenerationDialog* dialog = new DenomGenerationDialog(window);
    openDialogWithOpaqueBackgroundY(dialog, window, 4.5, 5);
}

void PrivacyWidget::onRescanMintsClicked(){
    if (ask(tr("Rescan Mints"),
        tr("Your zerocoin mints are going to be scanned from the blockchain from scratch"))
    ){
        std::string strResetMintResult = walletModel->resetMintZerocoin();
        inform(QString::fromStdString(strResetMintResult));
    }
}

void PrivacyWidget::onResetZeroClicked(){
    if (ask(tr("Reset Spent zVIT"),
        tr("Your zerocoin spends are going to be scanned from the blockchain from scratch"))
    ){
        std::string strResetMintResult = walletModel->resetSpentZerocoin();
        inform(QString::fromStdString(strResetMintResult));
    }
}

void PrivacyWidget::updateDenomsSupply(){
    std::map<libzerocoin::CoinDenomination, CAmount> mapDenomBalances;
    std::map<libzerocoin::CoinDenomination, int> mapUnconfirmed;
    std::map<libzerocoin::CoinDenomination, int> mapImmature;
    for (const auto& denom : libzerocoin::zerocoinDenomList){
        mapDenomBalances.insert(std::make_pair(denom, 0));
        mapUnconfirmed.insert(std::make_pair(denom, 0));
        mapImmature.insert(std::make_pair(denom, 0));
    }

    std::set<CMintMeta> vMints;
    walletModel->listZerocoinMints(vMints, true, false, true, true);

    std::map<libzerocoin::CoinDenomination, int> mapMaturityHeights = GetMintMaturityHeight();
    for (auto& meta : vMints){
        // All denominations
        mapDenomBalances.at(meta.denom)++;

        if (!meta.nHeight || chainActive.Height() - meta.nHeight <= Params().Zerocoin_MintRequiredConfirmations()) {
            // All unconfirmed denominations
            mapUnconfirmed.at(meta.denom)++;
        } else {
            if (meta.denom == libzerocoin::CoinDenomination::ZQ_ERROR) {
                mapImmature.at(meta.denom)++;
            } else if (meta.nHeight >= mapMaturityHeights.at(meta.denom)) {
                mapImmature.at(meta.denom)++;
            }
        }
    }

    int64_t nCoins = 0;
    int64_t nSumPerCoin = 0;
    int64_t nUnconfirmed = 0;
    int64_t nImmature = 0;
    QString strDenomStats, strUnconfirmed = "";

    for (const auto& denom : libzerocoin::zerocoinDenomList) {
        nCoins = libzerocoin::ZerocoinDenominationToInt(denom);
        nSumPerCoin = nCoins * mapDenomBalances.at(denom);
        nUnconfirmed = mapUnconfirmed.at(denom);
        nImmature = mapImmature.at(denom);

        strUnconfirmed = "";
        if (nUnconfirmed) {
            strUnconfirmed += QString::number(nUnconfirmed) + QString(" unconf. ");
        }
        if(nImmature) {
            strUnconfirmed += QString::number(nImmature) + QString(" immature ");
        }
        if(nImmature || nUnconfirmed) {
            strUnconfirmed = QString("( ") + strUnconfirmed + QString(") ");
        }

        strDenomStats = strUnconfirmed + QString::number(mapDenomBalances.at(denom)) + " x " +
                        QString::number(nCoins) + " = <b>" +
                        QString::number(nSumPerCoin) + " zVIT </b>";

        switch (nCoins) {
            case libzerocoin::CoinDenomination::ZQ_ONE:
                ui->labelValueDenom1->setText(strDenomStats);
                break;
            case libzerocoin::CoinDenomination::ZQ_FIVE:
                ui->labelValueDenom5->setText(strDenomStats);
                break;
            case libzerocoin::CoinDenomination::ZQ_TEN:
                ui->labelValueDenom10->setText(strDenomStats);
                break;
            case libzerocoin::CoinDenomination::ZQ_FIFTY:
                ui->labelValueDenom50->setText(strDenomStats);
                break;
            case libzerocoin::CoinDenomination::ZQ_ONE_HUNDRED:
                ui->labelValueDenom100->setText(strDenomStats);
                break;
            case libzerocoin::CoinDenomination::ZQ_FIVE_HUNDRED:
                ui->labelValueDenom500->setText(strDenomStats);
                break;
            case libzerocoin::CoinDenomination::ZQ_ONE_THOUSAND:
                ui->labelValueDenom1000->setText(strDenomStats);
                break;
            case libzerocoin::CoinDenomination::ZQ_FIVE_THOUSAND:
                ui->labelValueDenom5000->setText(strDenomStats);
                break;
            default:
                // Error Case: don't update display
                break;
        }
    }

    CAmount matureZerocoinBalance = walletModel->getZerocoinBalance() - walletModel->getUnconfirmedZerocoinBalance() - walletModel->getImmatureZerocoinBalance();
    ui->btnTotalzVIT->setTitleText(tr("Total %1").arg(GUIUtil::formatBalance(matureZerocoinBalance, nDisplayUnit, true)));
}

void PrivacyWidget::changeTheme(bool isLightTheme, QString& theme){
    static_cast<TxViewHolder*>(this->delegate->getRowFactory())->isLightTheme = isLightTheme;
    ui->listView->update();
}

PrivacyWidget::~PrivacyWidget(){
    delete ui;
}
