#ifndef SETTINGSMULTISENDWIDGET_H
#define SETTINGSMULTISENDWIDGET_H

#include <QWidget>
#include "qt/vitae/pwidget.h"

class PIVXGUI;

namespace Ui {
class SettingsMultisendWidget;
}

class SettingsMultisendWidget : public PWidget
{
    Q_OBJECT

public:
    explicit SettingsMultisendWidget(VITAEGUI* _window, QWidget *parent = nullptr);
    ~SettingsMultisendWidget();

private slots:
    void onAddRecipientClicked();

private:
    Ui::SettingsMultisendWidget *ui;
    VITAEGUI* window;
};

#endif // SETTINGSMULTISENDWIDGET_H
