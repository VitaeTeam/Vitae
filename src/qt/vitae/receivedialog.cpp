#include "qt/vitae/receivedialog.h"
#include "qt/vitae/forms/ui_receivedialog.h"
#include "qt/vitae/qtutils.h"
#include "walletmodel.h"
#include <QFile>

ReceiveDialog::ReceiveDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ReceiveDialog)
{
    ui->setupUi(this);
    // Stylesheet
    this->setStyleSheet(parent->styleSheet());

    ui->frameContainer->setProperty("cssClass", "container-dialog");
    ui->frameContainer->setContentsMargins(10,10,10,10);


    // Title

    ui->labelTitle->setText("My Address");
    ui->labelTitle->setProperty("cssClass", "text-title-dialog");

    // Address

    ui->labelAddress->setText("D7VFR83SQbiezrW72hjcWJtcfip5krte2Z ");
    ui->labelAddress->setProperty("cssClass", "text-main-grey-big");

    // QR image

    QPixmap pixmap(":/res/img/img-qr-test-big.png");
    ui->labelQrImg->setPixmap(pixmap.scaled(
                                  ui->labelQrImg->width(),
                                  ui->labelQrImg->height(),
                                  Qt::KeepAspectRatio)
                              );


    // Buttons
    
    ui->btnEsc->setText("");
    ui->btnEsc->setProperty("cssClass", "ic-close");

    ui->btnCancel->setProperty("cssClass", "btn-dialog-cancel");
    ui->btnSave->setText("COPY");
    ui->btnSave->setProperty("cssClass", "btn-primary");
    ui->btnCancel->setVisible(false);


    connect(ui->btnEsc, SIGNAL(clicked()), this, SLOT(close()));
    connect(ui->btnSave, SIGNAL(clicked()), this, SLOT(onCopy()));
}

void ReceiveDialog::updateQr(QString address){
    if(!info) info = new SendCoinsRecipient();
    info->address = address;
    QString uri = GUIUtil::formatBitcoinURI(*info);
    ui->labelQrImg->setText("");
    ui->labelAddress->setText(address);
    QString error;
    QPixmap pixmap = encodeToQr(uri, error);
    if(!pixmap.isNull()){
        qrImage = &pixmap;
        ui->labelQrImg->setPixmap(qrImage->scaled(ui->labelQrImg->width(), ui->labelQrImg->height()));
    }else{
        ui->labelQrImg->setText(!error.isEmpty() ? error : "Error encoding address");
    }
}

void ReceiveDialog::onCopy(){
    GUIUtil::setClipboard(info->address);
    accept();
}

ReceiveDialog::~ReceiveDialog()
{
    delete ui;
}
