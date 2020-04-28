#include "qt/vitae/topbar.h"
#include "qt/vitae/forms/ui_topbar.h"
#include <QPixmap>
#include <QFile>
#include "qt/vitae/lockunlock.h"
#include "qt/vitae/qtutils.h"
#include "qt/vitae/receivedialog.h"
#include "qt/vitae/walletpassworddialog.h"

#include "qt/vitae/qtutils.h"
#include "qt/vitae/receivedialog.h"
#include "qt/vitae/walletpassworddialog.h"

#include "bitcoinunits.h"
#include "clientmodel.h"
#include "qt/guiconstants.h"
#include "qt/guiutil.h"
#include "optionsmodel.h"
#include "qt/platformstyle.h"
#include "wallet.h"
#include "walletmodel.h"


TopBar::TopBar(VITAEGUI* _mainWindow, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TopBar),
    mainWindow(_mainWindow)
{
    ui->setupUi(this);

    // Set parent stylesheet
    this->setStyleSheet(_mainWindow->styleSheet());

    /* Containers */
    ui->containerTop->setProperty("cssClass", "container-top");
    ui->containerTop->setContentsMargins(10,4,10,10);

    QFont font;
    font.setWeight(QFont::Light);


    ui->labelTitle1->setProperty("cssClass", "text-title-topbar");
    ui->labelTitle1->setFont(font);
    ui->labelTitle2->setProperty("cssClass", "text-title-topbar");
    ui->labelTitle1->setFont(font);
    ui->labelTitle3->setProperty("cssClass", "text-title-topbar");
    ui->labelTitle1->setFont(font);
    ui->labelTitle4->setProperty("cssClass", "text-title-topbar");
    ui->labelTitle1->setFont(font);
    ui->labelTitle5->setProperty("cssClass", "text-title-topbar");
    ui->labelTitle1->setFont(font);
    ui->labelTitle6->setProperty("cssClass", "text-title-topbar");
    ui->labelTitle1->setFont(font);

    // Amount information top

    ui->widgetTopAmount->setVisible(false);
    ui->labelAmountTopPiv->setProperty("cssClass", "amount-small-topbar");
    ui->labelAmountTopzPiv->setProperty("cssClass", "amount-small-topbar");

    ui->labelAmountPiv->setProperty("cssClass", "amount-topbar");
    ui->labelAmountzPiv->setProperty("cssClass", "amount-topbar");

    ui->labelPendingPiv->setProperty("cssClass", "amount-small-topbar");
    ui->labelPendingzPiv->setProperty("cssClass", "amount-small-topbar");

    ui->labelImmaturePiv->setProperty("cssClass", "amount-small-topbar");
    ui->labelImmaturezPiv->setProperty("cssClass", "amount-small-topbar");


    //ui->pushButtonSync->setProperty("cssClass", "sync-status");

    // Buttons


    // New button

    ui->pushButtonPeers->setButtonClassStyle("cssClass", "btn-check-peers");
    ui->pushButtonPeers->setButtonText("No Online Peers");

    ui->pushButtonConnection->setButtonClassStyle("cssClass", "btn-check-connect");
    ui->pushButtonConnection->setButtonText("No Connection");

    ui->pushButtonStack->setButtonClassStyle("cssClass", "btn-check-stack");
    ui->pushButtonStack->setButtonText("Staking Disabled");

    ui->pushButtonMint->setButtonClassStyle("cssClass", "btn-check-mint");
    ui->pushButtonMint->setButtonText("Automint Enabled");

    ui->pushButtonSync->setButtonClassStyle("cssClass", "btn-check-sync");
    ui->pushButtonSync->setButtonText(" %54 Synchronizing..");

    ui->pushButtonLock->setButtonClassStyle("cssClass", "btn-check-lock");

    ui->pushButtonTheme->setButtonClassStyle("cssClass", "btn-check-theme");

    if(isLightTheme()){
        ui->pushButtonTheme->setButtonText("Light Theme");
    }else{
        ui->pushButtonTheme->setButtonText("Dark Theme");
    }


    ui->qrContainer->setProperty("cssClass", "container-qr");
    ui->pushButtonQR->setProperty("cssClass", "btn-qr");

    // QR image

    QPixmap pixmap(":/res/img/img-qr-test.png");
    ui->btnQr->setIcon(
                QIcon(pixmap.scaled(
                         70,
                         70,
                         Qt::KeepAspectRatio))
                );

    ui->pushButtonLock->setButtonText("Wallet Locked  ");
    ui->pushButtonLock->setButtonClassStyle("cssClass", "btn-check-status-lock");


    connect(ui->pushButtonQR, SIGNAL(clicked()), this, SLOT(onBtnReceiveClicked()));
    connect(ui->btnQr, SIGNAL(clicked()), this, SLOT(onBtnReceiveClicked()));


    connect(ui->pushButtonLock, SIGNAL(Mouse_Pressed()), this, SLOT(onBtnLockClicked()));
    //connect(ui->pushButtonLock, SIGNAL(Mouse_Hover()), this, SLOT(onBtnLockHover()));
    //connect(ui->pushButtonLock, SIGNAL(Mouse_HoverLeave()), this, SLOT(onBtnLockHoverLeave()));

    connect(ui->pushButtonTheme, SIGNAL(Mouse_Pressed()), this, SLOT(onThemeClicked()));
}

void TopBar::onThemeClicked(){
    // Store theme
    bool lightTheme = !ui->pushButtonTheme->isChecked();
    setTheme(lightTheme);
    // TODO: Complete me..
    //mainWindow->changeTheme(lightTheme);

    if(lightTheme){
        ui->pushButtonTheme->setText2("Light Theme");
    }else{
        ui->pushButtonTheme->setText2("Dark Theme");
    }
}


void TopBar::onBtnLockClicked(){

    if(!lockUnlockWidget){
        lockUnlockWidget = new LockUnlock(this->mainWindow);
        connect(lockUnlockWidget, SIGNAL(Mouse_Leave()), this, SLOT(lockDropdownMouseLeave()));
        connect(lockUnlockWidget, SIGNAL(lockClicked(const StateClicked&)), this, SLOT(lockDropdownClicked(const StateClicked&)));
    }

    lockUnlockWidget->setFixedWidth(ui->pushButtonLock->width());
    lockUnlockWidget->adjustSize();

    lockUnlockWidget->move(
                ui->pushButtonLock->pos().rx() + this->mainWindow->getNavWidth() + 10,
                ui->pushButtonLock->y() + 36
                );

    lockUnlockWidget->setStyleSheet("margin:0px; padding:0px;");

    lockUnlockWidget->raise();
    lockUnlockWidget->activateWindow();
    lockUnlockWidget->show();

    // Keep open
    ui->pushButtonLock->setKeepExpanded(true);
}

void TopBar::lockDropdownClicked(const StateClicked& state){
    switch (state) {
        case LOCK:
            lockUnlockWidget->close();
            this->showPasswordDialog();
        break;
        case UNLOCK_FOR_STAKING:
            // Do something
        break;
        case UNLOCK:
            // Nothing
        break;
    }
}

void TopBar::showPasswordDialog() {
    mainWindow->showHide(true);
    WalletPasswordDialog* dialog = new WalletPasswordDialog(mainWindow);
    openDialogWithOpaqueBackgroundY(dialog, mainWindow, 3, 5);
}

void TopBar::lockDropdownMouseLeave(){
    lockUnlockWidget->close();

    switch(lockUnlockWidget->lock){
        case 0:{
            ui->pushButtonLock->setButtonText("Wallet Locked");
            ui->pushButtonLock->setButtonClassStyle("cssClass", "btn-check-status-lock", true);
            break;
        }
        case 1:{
            ui->pushButtonLock->setButtonText("Wallet Unlocked");
            ui->pushButtonLock->setButtonClassStyle("cssClass", "btn-check-status-unlock", true);

            break;
        }
        case 2:{
            ui->pushButtonLock->setButtonText("Wallet Unlocked for staking");
            ui->pushButtonLock->setButtonClassStyle("cssClass", "btn-check-status-staking", true);
            break;
        }
    }

    ui->pushButtonLock->setKeepExpanded(false);
    ui->pushButtonLock->setSmall();

    ui->pushButtonLock->update();

}

void TopBar::onBtnReceiveClicked(){
    mainWindow->showHide(true);
    ReceiveDialog* receiveDialog = new ReceiveDialog(mainWindow);
    if(openDialogWithOpaqueBackground(receiveDialog, mainWindow)){
        // TODO: Complete me..
        //mainWindow->openSnackbar("Address Copied");
    }

}

void TopBar::showTop(){
    if(ui->bottom_container->isVisible()){
        ui->bottom_container->setVisible(false);
        ui->widgetTopAmount->setVisible(true);
        this->setFixedHeight(75);
    }
}

void TopBar::showBottom()
{
    ui->widgetTopAmount->setVisible(false);
    ui->bottom_container->setVisible(true);
    this->setFixedHeight(200);
    this->adjustSize();
}

TopBar::~TopBar()
{
    delete ui;
}

void TopBar::setClientModel(ClientModel *model){
    this->clientModel = model;
    if(clientModel){
        // Keep up to date with client
        setNumConnections(clientModel->getNumConnections());
        connect(clientModel, SIGNAL(numConnectionsChanged(int)), this, SLOT(setNumConnections(int)));


    }
}

void TopBar::setNumConnections(int count)
{
    ui->pushButtonConnection->setChecked(count > 0);
    // TODO: Check if really want to put the number of connections here
    ui->pushButtonConnection->setButtonText(tr("%n active connection(s)", "", count));
}

void TopBar::setWalletModel(WalletModel *model){
    this->walletModel = model;

    connect(model, SIGNAL(balanceChanged(CAmount, CAmount, CAmount, CAmount, CAmount, CAmount, CAmount, CAmount, CAmount)), this,
            SLOT(updateBalances(CAmount, CAmount, CAmount, CAmount, CAmount, CAmount, CAmount, CAmount, CAmount)));
    connect(model->getOptionsModel(), SIGNAL(displayUnitChanged(int)), this, SLOT(updateDisplayUnit()));

    // update the display unit, to not use the default ("PIVX")
    updateDisplayUnit();

    // TODO: Complete me..
    //refreshWalletStatus();
}

void TopBar::updateDisplayUnit()
{
    if (walletModel && walletModel->getOptionsModel()) {
        int displayUnitPrev = nDisplayUnit;
        nDisplayUnit = walletModel->getOptionsModel()->getDisplayUnit();
        if (displayUnitPrev != nDisplayUnit)
            updateBalances(walletModel->getBalance(), walletModel->getUnconfirmedBalance(), walletModel->getImmatureBalance(),
                           walletModel->getZerocoinBalance(), walletModel->getUnconfirmedZerocoinBalance(), walletModel->getImmatureZerocoinBalance(),
                           walletModel->getWatchBalance(), walletModel->getWatchUnconfirmedBalance(), walletModel->getWatchImmatureBalance());

        // TODO: Update txdelegate->unit with the current unit
        //ui->listTransactions->update();
    }
}

void TopBar::updateBalances(const CAmount& balance, const CAmount& unconfirmedBalance, const CAmount& immatureBalance,
                            const CAmount& zerocoinBalance, const CAmount& unconfirmedZerocoinBalance, const CAmount& immatureZerocoinBalance,
                            const CAmount& watchOnlyBalance, const CAmount& watchUnconfBalance, const CAmount& watchImmatureBalance){

    CAmount nLockedBalance = 0;
    CAmount nWatchOnlyLockedBalance = 0;
    if (!walletModel) {
        nLockedBalance = walletModel->getLockedBalance();
    }

    // PIV Balance
    CAmount nTotalBalance = balance + unconfirmedBalance;
    CAmount pivAvailableBalance = balance - immatureBalance - nLockedBalance;
    CAmount nUnlockedBalance = nTotalBalance - nLockedBalance;

    // zPIV Balance
    CAmount matureZerocoinBalance = zerocoinBalance - unconfirmedZerocoinBalance - immatureZerocoinBalance;

    // Set
    QString totalPiv = formatBalance(pivAvailableBalance);
    QString totalzPiv = formatBalance(matureZerocoinBalance);
    // Top
    ui->labelAmountTopPiv->setText(totalPiv);
    ui->labelAmountTopzPiv->setText(totalzPiv);

    // Expanded
    ui->labelAmountPiv->setText(totalPiv);
    ui->labelAmountzPiv->setText(totalzPiv);

    ui->labelPendingPiv->setText(formatBalance(unconfirmedBalance));
    ui->labelPendingzPiv->setText(formatBalance(unconfirmedZerocoinBalance));

    ui->labelImmaturePiv->setText(formatBalance(immatureBalance));
    ui->labelImmaturezPiv->setText(formatBalance(immatureZerocoinBalance));
}

QString TopBar::formatBalance(CAmount amount){
    return (amount == 0) ? ("0.00 " + BitcoinUnits::name(nDisplayUnit)) : BitcoinUnits::floorHtmlWithUnit(nDisplayUnit, amount, false, BitcoinUnits::separatorAlways, true);
}
