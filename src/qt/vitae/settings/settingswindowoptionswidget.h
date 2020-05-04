#ifndef SETTINGSWINDOWOPTIONSWIDGET_H
#define SETTINGSWINDOWOPTIONSWIDGET_H

#include <QWidget>
#include <QDataWidgetMapper>
#include "qt/vitae/pwidget.h"

namespace Ui {
class SettingsWindowOptionsWidget;
}

class SettingsWindowOptionsWidget : public PWidget
{
    Q_OBJECT

public:
    explicit SettingsWindowOptionsWidget(VITAEGUI* _window, QWidget *parent = nullptr);
    ~SettingsWindowOptionsWidget();

    void setMapper(QDataWidgetMapper *mapper);

private:
    Ui::SettingsWindowOptionsWidget *ui;
};

#endif // SETTINGSWINDOWOPTIONSWIDGET_H
