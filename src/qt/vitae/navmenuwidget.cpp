// Copyright (c) 2019 The PIVX developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "qt/vitae/navmenuwidget.h"
#include "qt/vitae/forms/ui_navmenuwidget.h"
#include "qt/vitae/vitaegui.h"
#include "qt/vitae/qtutils.h"
#include "clientversion.h"
#include "optionsmodel.h"


NavMenuWidget::NavMenuWidget(VITAEGUI *mainWindow, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::NavMenuWidget),
    window(mainWindow)
{
    ui->setupUi(this);
    this->setFixedWidth(100);
    setCssProperty(ui->navContainer_2, "container-nav");
    setCssProperty(ui->imgLogo, "img-nav-logo");

    // App version
    ui->labelVersion->setText(QString(tr("v%1")).arg(QString::fromStdString(FormatVersionFriendly())));
    ui->labelVersion->setProperty("cssClass", "text-title-white");

    // Buttons
    ui->btnDashboard->setProperty("name", "dash");
    ui->btnDashboard->setText("HOME\n");
    ui->btnDashboard->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

    ui->btnSend->setProperty("name", "send");
    ui->btnSend->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    ui->btnSend->setText("SEND\n");

    ui->btnAddress->setProperty("name", "address");
    ui->btnAddress->setText("CONTACTS\n");
    ui->btnAddress->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

    ui->btnPrivacy->setProperty("name", "privacy");
    ui->btnPrivacy->setText("PRIVACY\n");
    ui->btnPrivacy->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

    ui->btnMaster->setProperty("name", "master");
    ui->btnMaster->setText("MASTER\r\nNODES");
    ui->btnMaster->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

    ui->btnColdStaking->setProperty("name", "cold-staking");
    ui->btnColdStaking->setText("COLD\r\nSTAKING");
    ui->btnColdStaking->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

    ui->btnSettings->setProperty("name", "settings");
    ui->btnSettings->setText("SETTINGS\n");
    ui->btnSettings->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

    ui->btnReceive->setProperty("name", "receive");
    ui->btnReceive->setText("RECEIVE\n");
    ui->btnReceive->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

    btns = {ui->btnDashboard, ui->btnSend, ui->btnReceive, ui->btnAddress, ui->btnPrivacy, ui->btnMaster, ui->btnColdStaking, ui->btnSettings, ui->btnColdStaking};
    onNavSelected(ui->btnDashboard, true);

    connectActions();
}

void NavMenuWidget::loadWalletModel() {
    if (walletModel && walletModel->getOptionsModel()) {
        ui->btnColdStaking->setVisible(walletModel->getOptionsModel()->isColdStakingScreenEnabled());
    }
}

/**
 * Actions
 */
void NavMenuWidget::connectActions() {
    connect(ui->btnDashboard,SIGNAL(clicked()),this, SLOT(onDashboardClicked()));
    connect(ui->btnSend,SIGNAL(clicked()),this, SLOT(onSendClicked()));
    connect(ui->btnAddress,SIGNAL(clicked()),this, SLOT(onAddressClicked()));
    connect(ui->btnPrivacy,SIGNAL(clicked()),this, SLOT(onPrivacyClicked()));
    connect(ui->btnMaster,SIGNAL(clicked()),this, SLOT(onMasterNodesClicked()));
    connect(ui->btnSettings,SIGNAL(clicked()),this, SLOT(onSettingsClicked()));
    connect(ui->btnReceive,SIGNAL(clicked()),this, SLOT(onReceiveClicked()));
    connect(ui->btnColdStaking,SIGNAL(clicked()),this, SLOT(onColdStakingClicked()));

    ui->btnDashboard->setShortcut(QKeySequence(SHORT_KEY + Qt::Key_1));
    ui->btnSend->setShortcut(QKeySequence(SHORT_KEY + Qt::Key_2));
    ui->btnReceive->setShortcut(QKeySequence(SHORT_KEY + Qt::Key_3));
    ui->btnAddress->setShortcut(QKeySequence(SHORT_KEY + Qt::Key_4));
    ui->btnPrivacy->setShortcut(QKeySequence(SHORT_KEY + Qt::Key_5));
    ui->btnMaster->setShortcut(QKeySequence(SHORT_KEY + Qt::Key_6));
    ui->btnColdStaking->setShortcut(QKeySequence(SHORT_KEY + Qt::Key_7));
    ui->btnSettings->setShortcut(QKeySequence(SHORT_KEY + Qt::Key_8));
}

void NavMenuWidget::onSendClicked(){
    window->goToSend();
    onNavSelected(ui->btnSend);
}

void NavMenuWidget::onDashboardClicked(){
    window->goToDashboard();
    onNavSelected(ui->btnDashboard);
}

void NavMenuWidget::onAddressClicked(){
    window->goToAddresses();
    onNavSelected(ui->btnAddress);
}


void NavMenuWidget::onPrivacyClicked(){
    window->goToPrivacy();
    onNavSelected(ui->btnPrivacy);
}

void NavMenuWidget::onMasterNodesClicked(){
    window->goToMasterNodes();
    onNavSelected(ui->btnMaster);
}

void NavMenuWidget::onColdStakingClicked() {
    window->goToColdStaking();
    onNavSelected(ui->btnColdStaking);
}

void NavMenuWidget::onSettingsClicked(){
    window->goToSettings();
    onNavSelected(ui->btnSettings);
}

void NavMenuWidget::onReceiveClicked(){
    window->goToReceive();
    onNavSelected(ui->btnReceive);
}

void NavMenuWidget::onNavSelected(QWidget* active, bool startup) {
    QString start = "btn-nav-";
    foreach (QWidget* w, btns) {
        QString clazz = start + w->property("name").toString();
        if (w == active) {
            clazz += "-active";
        }
        setCssProperty(w, clazz);
    }
    if (!startup) updateButtonStyles();
}

void NavMenuWidget::selectSettings() {
    onSettingsClicked();
}

void NavMenuWidget::onShowHideColdStakingChanged(bool show) {
    ui->btnColdStaking->setVisible(show);
    window->setMinimumHeight(show ? 780 : 740);
}

void NavMenuWidget::updateButtonStyles(){
    forceUpdateStyle({
         ui->btnDashboard,
         ui->btnSend,
         ui->btnAddress,
         ui->btnPrivacy,
         ui->btnMaster,
         ui->btnSettings,
         ui->btnReceive,
         ui->btnColdStaking
    });
}

NavMenuWidget::~NavMenuWidget(){
    delete ui;
}
