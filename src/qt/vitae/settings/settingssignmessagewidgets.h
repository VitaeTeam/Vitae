#ifndef SETTINGSSIGNMESSAGEWIDGETS_H
#define SETTINGSSIGNMESSAGEWIDGETS_H

#include <QWidget>
#include "qt/vitae/pwidget.h"
namespace Ui {
class SettingsSignMessageWidgets;
}

class SettingsSignMessageWidgets : public PWidget
{
    Q_OBJECT

public:
    explicit SettingsSignMessageWidgets(VITAEGUI* _window, QWidget *parent = nullptr);
    ~SettingsSignMessageWidgets();

private:
    Ui::SettingsSignMessageWidgets *ui;
};

#endif // SETTINGSSIGNMESSAGEWIDGETS_H
