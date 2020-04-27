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

private:
    Ui::SettingsWalletRepairWidget *ui;
};

#endif // SETTINGSWALLETREPAIRWIDGET_H
