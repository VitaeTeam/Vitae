#include "qt/vitae/defaultdialog.h"
#include "qt/vitae/forms/ui_defaultdialog.h"

DefaultDialog::DefaultDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DefaultDialog)
{
    ui->setupUi(this);

    // Stylesheet
    this->setStyleSheet(parent->styleSheet());

    // Container

    ui->frame->setProperty("cssClass", "container-dialog");

    // Text

    ui->labelTitle->setText("Dialog Title");
    ui->labelTitle->setProperty("cssClass", "text-title-dialog");


    ui->labelMessage->setText("Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat.");
    ui->labelMessage->setProperty("cssClass", "text-main-grey");

    // Buttons

    ui->btnEsc->setText("");
    ui->btnEsc->setProperty("cssClass", "ic-close");

    ui->btnCancel->setProperty("cssClass", "btn-dialog-cancel");
    ui->btnSave->setText("OK");
    ui->btnSave->setProperty("cssClass", "btn-primary");

    connect(ui->btnEsc, SIGNAL(clicked()), this, SLOT(close()));
    connect(ui->btnCancel, SIGNAL(clicked()), this, SLOT(close()));
    connect(ui->btnSave, SIGNAL(clicked()), this, SLOT(accept()));
}

void DefaultDialog::setText(QString title, QString message, QString okBtnText, QString cancelBtnText){
    if(!okBtnText.isNull()) ui->btnSave->setText(okBtnText);
    if(!cancelBtnText.isNull()){
        ui->btnCancel->setVisible(true);
        ui->btnCancel->setText(cancelBtnText);
    }else{
        ui->btnCancel->setVisible(false);
    }
    if(!message.isNull()) ui->labelMessage->setText(message);
    if(!title.isNull()) ui->labelTitle->setText(title);
}

DefaultDialog::~DefaultDialog()
{
    delete ui;
}