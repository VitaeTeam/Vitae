#include "qt/vitae/pwidget.h"
#include "qt/vitae/qtutils.h"

PWidget::PWidget(VITAEGUI* _window, QWidget *parent) : QWidget(parent), window(_window)
{
    if(window)
        connect(window, SIGNAL(themeChanged(bool, QString&)), this, SLOT(changeTheme(bool, QString&)));
}


void PWidget::changeTheme(bool isLightTheme, QString& theme){
    // Change theme in all of the childs here..
    this->setStyleSheet(theme);
    updateStyle(this);
}
