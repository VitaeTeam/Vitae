#ifndef SETTINGSNETWORKWIDGET_H
#define SETTINGSNETWORKWIDGET_H

#include <QWidget>
#include "qt/vitae/pwidget.h"

namespace Ui {
class SettingsNetworkWidget;
}

class SettingsNetworkWidget : public PWidget
{
    Q_OBJECT

public:
    explicit SettingsNetworkWidget(VITAEGUI* _window, QWidget *parent = nullptr);
    ~SettingsNetworkWidget();

private:
    Ui::SettingsNetworkWidget *ui;
};

#endif // SETTINGSNETWORKWIDGET_H
