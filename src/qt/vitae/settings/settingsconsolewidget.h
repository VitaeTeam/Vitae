#ifndef SETTINGSCONSOLEWIDGET_H
#define SETTINGSCONSOLEWIDGET_H

#include <QWidget>
#include "qt/vitae/pwidget.h"

namespace Ui {
class SettingsConsoleWidget;
}

class SettingsConsoleWidget : public PWidget
{
    Q_OBJECT

public:
    explicit SettingsConsoleWidget(VITAEGUI* _window, QWidget *parent = nullptr);
    ~SettingsConsoleWidget();

private:
    Ui::SettingsConsoleWidget *ui;
};

#endif // SETTINGSCONSOLEWIDGET_H
