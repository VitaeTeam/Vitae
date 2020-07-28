// Copyright (c) 2019 The PIVX developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "qt/vitae/tooltipmenu.h"
#include "qt/vitae/forms/ui_tooltipmenu.h"

#include "qt/vitae/vitaegui.h"
#include "qt/vitae/qtutils.h"
#include <QTimer>

TooltipMenu::TooltipMenu(VITAEGUI *_window, QWidget *parent) :
    PWidget(_window, parent),
    ui(new Ui::TooltipMenu)
{
    ui->setupUi(this);
    ui->btnLast->setVisible(false);
    setCssProperty(ui->container, "container-list-menu");
    setCssProperty({ui->btnCopy, ui->btnDelete, ui->btnEdit, ui->btnLast}, "btn-list-menu");
    connect(ui->btnCopy, SIGNAL(clicked()), this, SLOT(copyClicked()));
    connect(ui->btnDelete, SIGNAL(clicked()), this, SLOT(deleteClicked()));
    connect(ui->btnEdit, SIGNAL(clicked()), this, SLOT(editClicked()));
    connect(ui->btnLast, SIGNAL(clicked()), this, SLOT(lastClicked()));
}

void TooltipMenu::setEditBtnText(QString btnText){
    ui->btnEdit->setText(btnText);
}

void TooltipMenu::setDeleteBtnText(QString btnText){
    ui->btnDelete->setText(btnText);
}

void TooltipMenu::setCopyBtnText(QString btnText){
    ui->btnCopy->setText(btnText);
}

void TooltipMenu::setLastBtnText(QString btnText, int minHeight){
    ui->btnLast->setText(btnText);
    ui->btnLast->setMinimumHeight(minHeight);
}

void TooltipMenu::setCopyBtnVisible(bool visible){
    ui->btnCopy->setVisible(visible);
}

void TooltipMenu::setDeleteBtnVisible(bool visible){
    ui->btnDelete->setVisible(visible);
}

void TooltipMenu::setEditBtnVisible(bool visible) {
    ui->btnEdit->setVisible(visible);
}

void TooltipMenu::setLastBtnVisible(bool visible) {
    ui->btnLast->setVisible(visible);
}

void TooltipMenu::deleteClicked(){
    hide();
    Q_EMIT onDeleteClicked();
}

void TooltipMenu::copyClicked(){
    hide();
    Q_EMIT onCopyClicked();
}

void TooltipMenu::editClicked(){
    hide();
    Q_EMIT onEditClicked();
}

void TooltipMenu::lastClicked() {
    hide();
    Q_EMIT onLastClicked();
}

void TooltipMenu::showEvent(QShowEvent *event){
    QTimer::singleShot(5000, this, SLOT(hide()));
}

TooltipMenu::~TooltipMenu()
{
    delete ui;
}
