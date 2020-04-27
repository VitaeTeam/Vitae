//
// Created by furszy on 3/21/19.
//

#include "VITAEGUI.h"

#ifdef Q_OS_MAC
#include "macdockiconhandler.h"
#endif

#include <qt/guiutil.h>
#include "networkstyle.h"
#include "notificator.h"

#include "qt/vitae/qtutils.h"


#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QApplication>
#include <QColor>


#include "util.h"


const QString VITAEGUI::DEFAULT_WALLET = "~Default";

VITAEGUI::VITAEGUI(const NetworkStyle* networkStyle, QWidget* parent) :
        QMainWindow(parent),
        clientModel(0){

    /* Open CSS when configured */
    this->setStyleSheet(GUIUtil::loadStyleSheet());

    GUIUtil::restoreWindowGeometry("nWindow", QSize(1200, 700), this);

    QString windowTitle = tr("PIVX Core") + " - ";
#ifdef ENABLE_WALLET
    /* if compiled with wallet support, -disablewallet can still disable the wallet */
    enableWallet = !GetBoolArg("-disablewallet", false);
#else
    enableWallet = false;
#endif // ENABLE_WALLET

    if (enableWallet) {
        windowTitle += tr("Wallet");
    } else {
        windowTitle += tr("Node");
    }

    setWindowTitle(windowTitle);

#ifndef Q_OS_MAC
    QApplication::setWindowIcon(networkStyle->getAppIcon());
    setWindowIcon(networkStyle->getAppIcon());
#else
    MacDockIconHandler::instance()->setIcon(networkStyle->getAppIcon());
#endif




#ifdef ENABLE_WALLET
    // Create wallet frame
    if(enableWallet){

        QFrame* centralWidget = new QFrame(this);
        this->setMinimumWidth(1200);
        QHBoxLayout* centralWidgetLayouot = new QHBoxLayout();
        centralWidget->setLayout(centralWidgetLayouot);
        centralWidgetLayouot->setContentsMargins(0,0,0,0);
        centralWidgetLayouot->setSpacing(0);

        centralWidget->setProperty("cssClass", "container");
        centralWidget->setStyleSheet("padding:0px; border:none; margin:0px;");

        // First the nav
        navMenu = new NavMenuWidget(this);
        centralWidgetLayouot->addWidget(navMenu);

        this->setCentralWidget(centralWidget);
        this->setContentsMargins(0,0,0,0);

        QFrame *container = new QFrame(centralWidget);
        container->setContentsMargins(0,0,0,0);
        centralWidgetLayouot->addWidget(container);

        // Then topbar + the stackedWidget
        QVBoxLayout *baseScreensContainer = new QVBoxLayout(this);
        baseScreensContainer->setMargin(0);
        container->setLayout(baseScreensContainer);

        // Insert the topbar
        topBar = new TopBar(this);
        topBar->setContentsMargins(0,0,0,0);
        baseScreensContainer->addWidget(topBar);

        // Now stacked widget
        stackedContainer = new QStackedWidget(this);
        QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        stackedContainer->setSizePolicy(sizePolicy);
        stackedContainer->setContentsMargins(0,0,0,0);
        baseScreensContainer->addWidget(stackedContainer);

        // Init
        dashboard = new DashboardWidget(this, this);
        sendWidget = new SendWidget(this, this);
        receiveWidget = new ReceiveWidget(this,this);
        addressesWidget = new AddressesWidget(this,this);
        privacyWidget = new PrivacyWidget(this,this);
        settingsWidget = new SettingsWidget(this,this);

        // Add to parent
        stackedContainer->addWidget(dashboard);
        stackedContainer->addWidget(sendWidget);
        stackedContainer->addWidget(receiveWidget);
        stackedContainer->addWidget(addressesWidget);
        stackedContainer->addWidget(privacyWidget);
        stackedContainer->addWidget(settingsWidget);
        stackedContainer->setCurrentWidget(dashboard);

    } else
#endif
    {
        // Add the rpc console instead.
    }


    // Create system tray icon and notification
    createTrayIcon(networkStyle);

}


void VITAEGUI::createTrayIcon(const NetworkStyle* networkStyle)
{
#ifndef Q_OS_MAC
    trayIcon = new QSystemTrayIcon(this);
    QString toolTip = tr("PIVX Core client") + " " + networkStyle->getTitleAddText();
    trayIcon->setToolTip(toolTip);
    trayIcon->setIcon(networkStyle->getAppIcon());
    trayIcon->hide();
#endif

    notificator = new Notificator(QApplication::applicationName(), trayIcon, this);
}

//
VITAEGUI::~VITAEGUI() {

}







void VITAEGUI::setClientModel(ClientModel* clientModel) {
    this->clientModel = clientModel;
    // TODO: Complete me..
}

void VITAEGUI::goToDashboard() {
    stackedContainer->setCurrentWidget(dashboard);
}

void VITAEGUI::goToSend() {
    stackedContainer->setCurrentWidget(sendWidget);
}

void VITAEGUI::goToReceive() {
    stackedContainer->setCurrentWidget(receiveWidget);
}

void VITAEGUI::goToAddresses() {
    stackedContainer->setCurrentWidget(addressesWidget);
}

void VITAEGUI::goToPrivacy() {
    stackedContainer->setCurrentWidget(privacyWidget);
}

void VITAEGUI::goToSettings() {
    stackedContainer->setCurrentWidget(settingsWidget);
}


void VITAEGUI::changeTheme(bool isLightTheme){
    // Change theme in all of the childs here..

    QString css = isLightTheme ? getLightTheme() : getDarkTheme();
    this->setStyleSheet(css);

    // Notify
    emit themeChanged(isLightTheme, css);

    // Update style
    updateStyle(this);
}

void VITAEGUI::resizeEvent(QResizeEvent* event){
    // Parent..
    QMainWindow::resizeEvent(event);
    // background
    showHide(opEnabled);
    // Notify
    emit windowResizeEvent(event);
}

void VITAEGUI::showHide(bool show){
    if(!op) op = new QLabel(this);
    if(!show){
        op->setVisible(false);
        opEnabled = false;
    }else{
        QColor bg("#000000");
        bg.setAlpha(200);
        if(!isLightTheme()){
            bg = QColor("#00000000");
            bg.setAlpha(150);
        }

        QPalette palette;
        palette.setColor(QPalette::Window, bg);
        op->setAutoFillBackground(true);
        op->setPalette(palette);
        op->setWindowFlags(Qt::CustomizeWindowHint);
        op->move(0,0);
        op->show();
        op->activateWindow();
        op->resize(width(), height());
        op->setVisible(true);
        opEnabled = true;
    }
}

int VITAEGUI::getNavWidth(){
    return this->navMenu->width();
}



#ifdef ENABLE_WALLET
bool VITAEGUI::addWallet(const QString& name, WalletModel* walletModel)
{
    // Single wallet supported for now..
    if(!stackedContainer || !clientModel || !walletModel)
        return false;

    // todo: show out of sync warning..
    // todo: complete this next method
    //connect(walletView, SIGNAL(showNormalIfMinimized()), gui, SLOT(showNormalIfMinimized()));

    // set the model for every view
    dashboard->setWalletModel(walletModel);


    return true;
}

bool VITAEGUI::setCurrentWallet(const QString& name)
{
    //if (!walletFrame)
    //    return false;
    return true;//walletFrame->setCurrentWallet(name);
}

void VITAEGUI::removeAllWallets()
{
    //if (!walletFrame)
    //    return;
    //setWalletActionsEnabled(false);
    //walletFrame->removeAllWallets();
}
#endif // ENABLE_WALLET
