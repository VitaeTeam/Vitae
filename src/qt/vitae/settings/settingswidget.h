#ifndef SETTINGSWIDGET_H
#define SETTINGSWIDGET_H

#include <QWidget>
#include "qt/vitae/pwidget.h"
#include "qt/vitae/settings/settingsbackupwallet.h"
#include "qt/vitae/settings/settingsbittoolwidget.h"
#include "qt/vitae/settings/settingssignmessagewidgets.h"
#include "qt/vitae/settings/settingschangepasswordwidget.h"
#include "qt/vitae/settings/settingswalletrepairwidget.h"
#include "qt/vitae/settings/settingsnetworkwidget.h"
#include "qt/vitae/settings/settingswalletoptionswidget.h"
#include "qt/vitae/settings/settingsmainoptionswidget.h"
#include "qt/vitae/settings/settingsdisplayoptionswidget.h"
#include "qt/vitae/settings/settingsmultisendwidget.h"
#include "qt/vitae/settings/settingsinformationwidget.h"
#include "qt/vitae/settings/settingsconsolewidget.h"
#include "qt/vitae/settings/settingswindowoptionswidget.h"

class VITAEGUI;

namespace Ui {
class SettingsWidget;
}

class SettingsWidget : public PWidget
{
    Q_OBJECT

public:
    explicit SettingsWidget(VITAEGUI* _window, QWidget *parent = nullptr);
    ~SettingsWidget();

    void loadClientModel() override;
    void loadWalletModel() override;

    void showDebugConsole();

signals:
    /** Get restart command-line parameters and handle restart */
    void handleRestart(QStringList args);


private slots:
    // File

    void onFileClicked();
    void onBackupWalletClicked();
    void onSignMessageClicked();
    void onVerifyMessageClicked();

    // Wallet Configuration

    void onConfigurationClicked();
    void onChangePasswordClicked();
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
    SettingsWalletRepairWidget *settingsWalletRepairWidget;
    SettingsNetworkWidget *settingsNetworkWidget;
    SettingsWalletOptionsWidget *settingsWalletOptionsWidget;
    SettingsMainOptionsWidget *settingsMainOptionsWidget;
    SettingsDisplayOptionsWidget *settingsDisplayOptionsWidget;
    SettingsMultisendWidget *settingsMultisendWidget;
    SettingsInformationWidget *settingsInformationWidget;
    SettingsConsoleWidget *settingsConsoleWidget;
    SettingsWindowOptionsWidget *settingsWindowOptionsWidget;
};

#endif // SETTINGSWIDGET_H
