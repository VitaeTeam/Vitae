#ifndef SETTINGSWALLETOPTIONSWIDGET_H
#define SETTINGSWALLETOPTIONSWIDGET_H

#include <QWidget>
#include "qt/vitae/pwidget.h"
namespace Ui {
class SettingsWalletOptionsWidget;
}

class SettingsWalletOptionsWidget : public PWidget
{
    Q_OBJECT

public:
    explicit SettingsWalletOptionsWidget(VITAEGUI* _window, QWidget *parent = nullptr);
    ~SettingsWalletOptionsWidget();

private:
    Ui::SettingsWalletOptionsWidget *ui;
};

#endif // SETTINGSWALLETOPTIONSWIDGET_H
