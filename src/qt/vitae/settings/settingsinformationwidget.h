#ifndef SETTINGSINFORMATIONWIDGET_H
#define SETTINGSINFORMATIONWIDGET_H

#include <QWidget>
#include "qt/vitae/pwidget.h"

namespace Ui {
class SettingsInformationWidget;
}

class SettingsInformationWidget : public PWidget
{
    Q_OBJECT

public:
    explicit SettingsInformationWidget(VITAEGUI* _window, QWidget *parent = nullptr);
    ~SettingsInformationWidget();

private:
    Ui::SettingsInformationWidget *ui;
};

#endif // SETTINGSINFORMATIONWIDGET_H
