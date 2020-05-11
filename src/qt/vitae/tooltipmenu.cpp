#include "qt/vitae/tooltipmenu.h"
#include "qt/vitae/forms/ui_tooltipmenu.h"

#include "qt/vitae/VITAEGUI.h"
#include "qt/vitae/qtutils.h"
#include <QTimer>

TooltipMenu::TooltipMenu(VITAEGUI *_window, QWidget *parent) :
    PWidget(_window, parent),
    ui(new Ui::TooltipMenu)
{
    ui->setupUi(this);
    setCssProperty(ui->container, "container-list-menu");
    setCssProperty({ui->btnCopy, ui->btnDelete, ui->btnEdit}, "btn-list-menu");
    connect(ui->btnCopy, SIGNAL(clicked()), this, SLOT(copyClicked()));
    connect(ui->btnDelete, SIGNAL(clicked()), this, SLOT(deleteClicked()));
    connect(ui->btnEdit, SIGNAL(clicked()), this, SLOT(editClicked()));
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

void TooltipMenu::setCopyBtnVisible(bool visible){
    ui->btnCopy->setVisible(visible);
}

void TooltipMenu::setDeleteBtnVisible(bool visible){
    ui->btnDelete->setVisible(visible);
}

void TooltipMenu::deleteClicked(){
    hide();
    emit onDeleteClicked();
}

void TooltipMenu::copyClicked(){
    hide();
    emit onCopyClicked();
}

void TooltipMenu::editClicked(){
    hide();
    emit onEditClicked();
}

void TooltipMenu::showEvent(QShowEvent *event){
    QTimer::singleShot(5000, this, SLOT(hide()));
}

TooltipMenu::~TooltipMenu()
{
    delete ui;
}
