#ifndef SETTINGSDISPLAYOPTIONSWIDGET_H
#define SETTINGSDISPLAYOPTIONSWIDGET_H

#include <QWidget>
#include "qt/vitae/pwidget.h"

namespace Ui {
class SettingsDisplayOptionsWidget;
}

class SettingsDisplayOptionsWidget : public PWidget
{
    Q_OBJECT

public:
    explicit SettingsDisplayOptionsWidget(VITAEGUI* _window = nullptr, QWidget *parent = nullptr);
    ~SettingsDisplayOptionsWidget();

private:
    Ui::SettingsDisplayOptionsWidget *ui;
};

#endif // SETTINGSDISPLAYOPTIONSWIDGET_H
