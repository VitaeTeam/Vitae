#ifndef SETTINGSNETWORKMONITORWIDGET_H
#define SETTINGSNETWORKMONITORWIDGET_H

#include <QWidget>
#include "qt/vitae/pwidget.h"

namespace Ui {
class SettingsNetworkMonitorWidget;
}

class SettingsNetworkMonitorWidget : public PWidget
{
    Q_OBJECT

public:
    explicit SettingsNetworkMonitorWidget(VITAEGUI* _window, QWidget *parent = nullptr);
    ~SettingsNetworkMonitorWidget();

private:
    Ui::SettingsNetworkMonitorWidget *ui;
};

#endif // SETTINGSNETWORKMONITORWIDGET_H
