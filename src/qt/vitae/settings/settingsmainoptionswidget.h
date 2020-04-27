#ifndef SETTINGSMAINOPTIONSWIDGET_H
#define SETTINGSMAINOPTIONSWIDGET_H

#include "qt/vitae/pwidget.h"

#include <QWidget>

namespace Ui {
class SettingsMainOptionsWidget;
}

class SettingsMainOptionsWidget : public PWidget
{
    Q_OBJECT

public:
    explicit SettingsMainOptionsWidget(VITAEGUI* _window, QWidget *parent = nullptr);
    ~SettingsMainOptionsWidget();

private:
    Ui::SettingsMainOptionsWidget *ui;
};

#endif // SETTINGSMAINOPTIONSWIDGET_H
