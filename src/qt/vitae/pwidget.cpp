#include "qt/vitae/pwidget.h"
#include "qt/vitae/qtutils.h"

PWidget::PWidget(VITAEGUI* _window, QWidget *parent) : QWidget(parent), window(_window)
{
    if(window)
        connect(window, SIGNAL(themeChanged(bool, QString&)), this, SLOT(changeTheme(bool, QString&)));
}

void PWidget::setClientModel(ClientModel* model){
    this->clientModel = model;
    loadClientModel();
}

void PWidget::setWalletModel(WalletModel* model){
    this->walletModel = model;
    loadWalletModel();
}

void PWidget::changeTheme(bool isLightTheme, QString& theme){
    // Change theme in all of the childs here..
    this->setStyleSheet(theme);
    updateStyle(this);
}

void PWidget::inform(const QString& message){
    emitMessage("", message, CClientUIInterface::MSG_INFORMATION);
}

void PWidget::warn(const QString& title, const QString& message){
    emitMessage(title, message, CClientUIInterface::MSG_ERROR);
}

void PWidget::ask(const QString& title, const QString& message, bool* ret){
    emitMessage(title, message, CClientUIInterface::MSG_INFORMATION | CClientUIInterface::BTN_MASK | CClientUIInterface::MODAL, ret);
}

void PWidget::emitMessage(const QString& title, const QString& body, unsigned int style, bool* ret){
    emit message(title, body, style, ret);
}

void PWidget::loadClientModel(){
    // override
}

void PWidget::loadWalletModel(){
    // override
}
