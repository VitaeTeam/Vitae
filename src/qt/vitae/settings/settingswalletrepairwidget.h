#ifndef SETTINGSWALLETREPAIRWIDGET_H
#define SETTINGSWALLETREPAIRWIDGET_H

#include <QWidget>
#include "qt/vitae/pwidget.h"

namespace Ui {
class SettingsWalletRepairWidget;
}

class SettingsWalletRepairWidget : public PWidget
{
    Q_OBJECT

public:
    explicit SettingsWalletRepairWidget(VITAEGUI* _window, QWidget *parent = nullptr);
    ~SettingsWalletRepairWidget();

    /** Build parameter list for restart */
    void buildParameterlist(QString arg);

signals:
    /** Get restart command-line parameters and handle restart */
    void handleRestart(QStringList args);

public slots:
    void walletSalvage();
    void walletRescan();
    void walletZaptxes1();
    void walletZaptxes2();
    void walletUpgrade();
    void walletReindex();
    void walletResync();

private:
    Ui::SettingsWalletRepairWidget *ui;
};

#endif // SETTINGSWALLETREPAIRWIDGET_H
