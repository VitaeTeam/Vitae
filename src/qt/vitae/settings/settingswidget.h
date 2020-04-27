#ifndef SETTINGSWIDGET_H
#define SETTINGSWIDGET_H

#include <QWidget>
#include "qt/vitae/settings/settingsbackupwallet.h"
#include "qt/vitae/settings/settingsbittoolwidget.h"
#include "qt/vitae/settings/settingssignmessagewidgets.h"
#include "qt/vitae/settings/settingschangepasswordwidget.h"
#include "qt/vitae/settings/settingsopenurlwidget.h"
#include "qt/vitae/settings/settingswalletrepairwidget.h"
#include "qt/vitae/settings/settingsnetworkwidget.h"
#include "qt/vitae/settings/settingswalletoptionswidget.h"
#include "qt/vitae/settings/settingsmainoptionswidget.h"
#include "qt/vitae/settings/settingsdisplayoptionswidget.h"
#include "qt/vitae/settings/settingsmultisendwidget.h"
#include "qt/vitae/settings/settingsinformationwidget.h"
#include "qt/vitae/settings/settingspeerslistwidget.h"
#include "qt/vitae/settings/settingslockwalletwidget.h"
#include "qt/vitae/settings/settingsconsolewidget.h"
#include "qt/vitae/settings/settingswindowoptionswidget.h"
#include "qt/vitae/settings/settingsnetworkmonitorwidget.h"

class VITAEGUI;

namespace Ui {
class SettingsWidget;
}

class SettingsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SettingsWidget(VITAEGUI* _window, QWidget *parent = nullptr);
    ~SettingsWidget();



private slots:
    // File

    void onFileClicked();
    void onOpenUrlClicked();
    void onBackupWalletClicked();
    void onSignMessageClicked();
    void onVerifyMessageClicked();

    // Wallet Configuration

    void onConfigurationClicked();
    void onChangePasswordClicked();
    void onLockWalletClicked();
    void onBipToolClicked();
    void onMultisendClicked();

    // Options

    void onOptionsClicked();
    void onMainOptionsClicked();
    void onWalletOptionsClicked();
    void onNetworkOptionsClicked();
    void onWindowOptionsClicked();
    void onDisplayOptionsClicked();


    // Tools

    void onToolsClicked();
    void onInformationClicked();
    void onDebugConsoleClicked();
    void onNetworkMonitorClicked();
    void onPeersListClicked();
    void onWalletRepairClicked();

    // Help

    void onHelpClicked();
    void onFaqClicked();
    void onAboutClicked();



    void changeTheme(bool isLightTheme, QString &theme);
private:
    Ui::SettingsWidget *ui;
    VITAEGUI* window;

    SettingsBackupWallet *settingsBackupWallet;
    SettingsBitToolWidget *settingsBitToolWidget;
    SettingsSignMessageWidgets *settingsSingMessageWidgets;
    SettingsChangePasswordWidget *settingsChangePasswordWidget;
    SettingsOpenUrlWidget *settingsOpenUrlWidget;
    SettingsWalletRepairWidget *settingsWalletRepairWidget;
    SettingsNetworkWidget *settingsNetworkWidget;
    SettingsWalletOptionsWidget *settingsWalletOptionsWidget;
    SettingsMainOptionsWidget *settingsMainOptionsWidget;
    SettingsDisplayOptionsWidget *settingsDisplayOptionsWidget;
    SettingsMultisendWidget *settingsMultisendWidget;
    SettingsInformationWidget *settingsInformationWidget;
    SettingsPeersListWidget *settingsPeersListWidget;
    SettingsLockWalletWidget *settingsLockWalletWidget;
    SettingsConsoleWidget *settingsConsoleWidget;
    SettingsWindowOptionsWidget *settingsWindowOptionsWidget;
    SettingsNetworkMonitorWidget *settingsNetworkMonitorWidget;
};

#endif // SETTINGSWIDGET_H
