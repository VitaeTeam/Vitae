#ifndef SETTINGSLOCKWALLETWIDGET_H
#define SETTINGSLOCKWALLETWIDGET_H

#include <QWidget>
#include "qt/vitae/pwidget.h"

namespace Ui {
class SettingsLockWalletWidget;
}

class SettingsLockWalletWidget : public PWidget
{
    Q_OBJECT

public:
    explicit SettingsLockWalletWidget(VITAEGUI* _window, QWidget *parent = nullptr);
    ~SettingsLockWalletWidget();

private:
    Ui::SettingsLockWalletWidget *ui;
};

#endif // SETTINGSLOCKWALLETWIDGET_H
