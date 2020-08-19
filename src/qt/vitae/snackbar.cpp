// Copyright (c) 2019 The PIVX developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "qt/vitae/snackbar.h"
#include "qt/vitae/forms/ui_snackbar.h"
#include "qt/vitae/qtutils.h"
#include <QTimer>


SnackBar::SnackBar(VITAEGUI* _window, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SnackBar),
    window(_window)
{
    ui->setupUi(this);

    this->setStyleSheet(parent->styleSheet());
    ui->snackContainer->setProperty("cssClass", "container-snackbar");
    ui->label->setProperty("cssClass", "text-snackbar");
    ui->pushButton->setProperty("cssClass", "ic-close");

    connect(ui->pushButton, &QPushButton::clicked, this, &SnackBar::close);
    if (window)
        connect(window, &PIVXGUI::windowResizeEvent, this, &SnackBar::windowResizeEvent);
    else {
        ui->horizontalLayout->setContentsMargins(0,0,0,0);
        ui->label->setStyleSheet("font-size: 15px; color:white;");
    }
}

void SnackBar::windowResizeEvent(QResizeEvent* event) {
    this->resize(qobject_cast<QWidget*>(parent())->width(), this->height());
    this->move(QPoint(0, window->height() - this->height() ));
}

void SnackBar::showEvent(QShowEvent *event){
    QTimer::singleShot(3000, this, &SnackBar::hideAnim);
}

void SnackBar::hideAnim(){
    if (window) closeDialog(this, window);
    QTimer::singleShot(310, this, &SnackBar::hide);
}



void SnackBar::sizeTo(QWidget* widget){

}

void SnackBar::setText(QString text){
    ui->label->setText(text);
}

SnackBar::~SnackBar(){
    delete ui;
}
