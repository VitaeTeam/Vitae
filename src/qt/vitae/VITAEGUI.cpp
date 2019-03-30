//
// Created by furszy on 3/21/19.
//

#include "VITAEGUI.h"

#ifdef Q_OS_MAC
#include "macdockiconhandler.h"
#endif

#include <qt/guiutil.h>
#include "clientmodel.h"
#include "optionsmodel.h"
#include "networkstyle.h"
#include "notificator.h"
#include "ui_interface.h"
#include "qt/vitae/qtutils.h"


#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QApplication>
#include <QColor>
#include <QShortcut>
#include <QKeySequence>

// TODO: Remove this..
#include <QMessageBox>


#include "util.h"


const QString VITAEGUI::DEFAULT_WALLET = "~Default";

VITAEGUI::VITAEGUI(const NetworkStyle* networkStyle, QWidget* parent) :
        QMainWindow(parent),
        clientModel(0){

    /* Open CSS when configured */
    this->setStyleSheet(GUIUtil::loadStyleSheet());

    GUIUtil::restoreWindowGeometry("nWindow", QSize(1200, 600), this);

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
        this->setMinimumHeight(600);
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
        // When compiled without wallet or -disablewallet is provided,
        // the central widget is the rpc console.
        rpcConsole = new RPCConsole(enableWallet ? this : 0);
        setCentralWidget(rpcConsole);
    }


    // Create system tray icon and notification
    createTrayIcon(networkStyle);

    // Connect events
    connectActions();

}

static Qt::Modifier shortKey
#ifdef Q_OS_MAC
     = Qt::CTRL;
#else
     = Qt::ALT;
#endif


/**
 * Here add every event connection
 */
void VITAEGUI::connectActions() {

    QShortcut *homeShort = new QShortcut(this);
    QShortcut *sendShort = new QShortcut(this);
    QShortcut *receiveShort = new QShortcut(this);
    QShortcut *addressesShort = new QShortcut(this);
    QShortcut *privacyShort = new QShortcut(this);
    QShortcut *settingsShort = new QShortcut(this);

    homeShort->setKey(QKeySequence(shortKey + Qt::Key_1));
    sendShort->setKey(QKeySequence(shortKey + Qt::Key_2));
    receiveShort->setKey(QKeySequence(shortKey + Qt::Key_3));
    addressesShort->setKey(QKeySequence(shortKey + Qt::Key_4));
    privacyShort->setKey(QKeySequence(shortKey + Qt::Key_5));
    settingsShort->setKey(QKeySequence(shortKey + Qt::Key_6));

    connect(homeShort, SIGNAL(activated()), this, SLOT(goToDashboard()));
    connect(sendShort, SIGNAL(activated()), this, SLOT(goToSend()));
    connect(receiveShort, SIGNAL(activated()), this, SLOT(goToReceive()));
    connect(addressesShort, SIGNAL(activated()), this, SLOT(goToAddresses()));
    connect(privacyShort, SIGNAL(activated()), this, SLOT(goToPrivacy()));
    connect(settingsShort, SIGNAL(activated()), this, SLOT(goToSettings()));
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
    if(this->clientModel) {
        // TODO: Complete me..
        topBar->setClientModel(clientModel);

        // Receive and report messages from client model
        connect(clientModel, SIGNAL(message(QString, QString, unsigned int)), this, SLOT(message(QString, QString, unsigned int)));

        if (rpcConsole) {
            rpcConsole->setClientModel(clientModel);
        }

        if (trayIcon) {
            trayIcon->show();
        }
    }
}


/**
 * TODO remove QMessageBox for the snackbar..
 */
void VITAEGUI::message(const QString& title, const QString& message, unsigned int style, bool* ret)
{
    QString strTitle = tr("PIVX Core"); // default title
    // Default to information icon
    int nMBoxIcon = QMessageBox::Information;
    int nNotifyIcon = Notificator::Information;

    QString msgType;

    // Prefer supplied title over style based title
    if (!title.isEmpty()) {
        msgType = title;
    } else {
        switch (style) {
            case CClientUIInterface::MSG_ERROR:
                msgType = tr("Error");
                break;
            case CClientUIInterface::MSG_WARNING:
                msgType = tr("Warning");
                break;
            case CClientUIInterface::MSG_INFORMATION:
                msgType = tr("Information");
                break;
            default:
                break;
        }
    }
    // Append title to "PIVX - "
    if (!msgType.isEmpty())
        strTitle += " - " + msgType;

    // Check for error/warning icon
    if (style & CClientUIInterface::ICON_ERROR) {
        nMBoxIcon = QMessageBox::Critical;
        nNotifyIcon = Notificator::Critical;
    } else if (style & CClientUIInterface::ICON_WARNING) {
        nMBoxIcon = QMessageBox::Warning;
        nNotifyIcon = Notificator::Warning;
    }

    // Display message
    if (style & CClientUIInterface::MODAL) {
        // Check for buttons, use OK as default, if none was supplied
        QMessageBox::StandardButton buttons;
        if (!(buttons = (QMessageBox::StandardButton)(style & CClientUIInterface::BTN_MASK)))
            buttons = QMessageBox::Ok;

        showNormalIfMinimized();
        QMessageBox mBox((QMessageBox::Icon)nMBoxIcon, strTitle, message, buttons, this);
        int r = mBox.exec();
        if (ret != NULL)
            *ret = r == QMessageBox::Ok;
    } else
        notificator->notify((Notificator::Class)nNotifyIcon, strTitle, message);
}


void VITAEGUI::showNormalIfMinimized(bool fToggleHidden)
{
    if (!clientModel)
        return;

    // activateWindow() (sometimes) helps with keyboard focus on Windows
    if (isHidden()) {
        show();
        activateWindow();
    } else if (isMinimized()) {
        showNormal();
        activateWindow();
    } else if (GUIUtil::isObscured(this)) {
        raise();
        activateWindow();
    } else if (fToggleHidden)
        hide();
}


void VITAEGUI::goToDashboard(){
    if(stackedContainer->currentWidget() != dashboard){
        stackedContainer->setCurrentWidget(dashboard);
        topBar->showBottom();
    }
}

void VITAEGUI::goToSend(){
    showTop(sendWidget);
}

void VITAEGUI::goToAddresses(){
    showTop(addressesWidget);
}

void VITAEGUI::goToPrivacy(){
    showTop(privacyWidget);
}

void VITAEGUI::goToMasterNodes(){
    //showTop(masterNodesWidget);
}

void VITAEGUI::goToSettings(){
    showTop(settingsWidget);
}

void VITAEGUI::goToReceive(){
    showTop(receiveWidget);
}

void VITAEGUI::showTop(QWidget* view){
    if(stackedContainer->currentWidget() != view){
        stackedContainer->setCurrentWidget(view);
        topBar->showTop();
    }
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
    topBar->setWalletModel(walletModel);
    dashboard->setWalletModel(walletModel);
    receiveWidget->setWalletModel(walletModel);


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
