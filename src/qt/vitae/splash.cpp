#include "qt/pivx/splash.h"
#include "qt/pivx/forms/ui_splash.h"
#include "QFile"

#include "init.h"
#include "ui_interface.h"
#include "networkstyle.h"
#include "util.h"
#include "version.h"
#include "guiutil.h"

#ifdef ENABLE_WALLET
#include "wallet.h"
#endif

#include <QCloseEvent>

#include <iostream>

Splash::Splash(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Splash)
{
    ui->setupUi(this);
    setWindowTitle("PIVX Wallet");

    this->setStyleSheet(GUIUtil::loadStyleSheet());

    ui->progressBar->setTextVisible(false);
    ui->frame->setProperty("cssClass", "container-welcome");
    ui->layoutProgress->setProperty("cssClass", "bg-progress");

    subscribeToCoreSignals();
}

Splash::~Splash(){
    unsubscribeFromCoreSignals();
    delete ui;
}

void Splash::slotFinish(QWidget* mainWin){
    Q_UNUSED(mainWin);
    hide();
}

static void InitMessage(Splash* splash, const std::string& message){
    QMetaObject::invokeMethod(splash, "showMessage",
                              Qt::QueuedConnection,
                              Q_ARG(QString, QString::fromStdString(message)),
                              Q_ARG(int, Qt::AlignBottom | Qt::AlignHCenter),
                              Q_ARG(QColor, QColor(100, 100, 100)));
}

static void ShowProgress(Splash* splash, const std::string& title, int nProgress){
    InitMessage(splash, title + strprintf("%d", nProgress) + "%");
}

#ifdef ENABLE_WALLET
static void ConnectWallet(Splash* splash, CWallet* wallet){
    wallet->ShowProgress.connect(boost::bind(ShowProgress, splash, _1, _2));
}
#endif

void Splash::subscribeToCoreSignals(){
    // Connect signals to client
    uiInterface.InitMessage.connect(boost::bind(InitMessage, this, _1));
    uiInterface.ShowProgress.connect(boost::bind(ShowProgress, this, _1, _2));
#ifdef ENABLE_WALLET
    uiInterface.LoadWallet.connect(boost::bind(ConnectWallet, this, _1));
#endif
}

void Splash::unsubscribeFromCoreSignals(){
    // Disconnect signals from client
    uiInterface.InitMessage.disconnect(boost::bind(InitMessage, this, _1));
    uiInterface.ShowProgress.disconnect(boost::bind(ShowProgress, this, _1, _2));
#ifdef ENABLE_WALLET
    if (pwalletMain)
        pwalletMain->ShowProgress.disconnect(boost::bind(ShowProgress, this, _1, _2));
#endif
}

void Splash::showMessage(const QString& message, int alignment, const QColor& color){
    ui->lblMessage->setText(message);
}

void Splash::closeEvent(QCloseEvent* event){
    StartShutdown(); // allows an "emergency" shutdown during startup
    event->ignore();
}