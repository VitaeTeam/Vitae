// Copyright (c) 2019 The PIVX developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "qt/vitae/mninfodialog.h"
#include "qt/vitae/forms/ui_mninfodialog.h"
#include "walletmodel.h"
#include "wallet.h"
#include "guiutil.h"
#include "qt/vitae/snackbar.h"
#include "qt/vitae/qtutils.h"
#include <QDateTime>

MnInfoDialog::MnInfoDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MnInfoDialog)
{
    ui->setupUi(this);
    this->setStyleSheet(parent->styleSheet());
    setCssProperty(ui->frame, "container-dialog");
    ui->labelTitle->setText(tr("Master Node Information"));
    setCssProperty(ui->labelTitle, "text-title-dialog");
    setCssTextBodyDialog({ui->labelAmount, ui->labelSend, ui->labelInputs, ui->labelFee, ui->labelId, ui->labelSize, ui->labelExport});
    setCssProperty({ui->labelDivider1, ui->labelDivider4, ui->labelDivider5, ui->labelDivider6, ui->labelDivider7, ui->labelDivider8, ui->labelDivider9}, "container-divider");
    setCssTextBodyDialog({ui->textAmount, ui->textAddress, ui->textInputs, ui->textStatus, ui->textId, ui->textSize, ui->textExport});
    setCssProperty({ui->pushCopy, ui->pushCopyId}, "ic-copy-big");
    setCssProperty(ui->btnEsc, "ic-close");
    ui->contentExport->setVisible(false);
    ui->labelDivider7->setVisible(false);
    ui->contentSize->setVisible(false);
    ui->labelDivider5->setVisible(false);
    connect(ui->btnEsc, SIGNAL(clicked()), this, SLOT(close()));
    connect(ui->pushCopy, &QPushButton::clicked, [this](){ copyInform(txId, "Master Node public key copied"); });
    connect(ui->pushCopyId, &QPushButton::clicked, [this](){ copyInform(pubKey, "Collateral tx id copied"); });
}

void MnInfoDialog::setData(QString pubKey, QString name, QString address, QString txId, QString outputIndex, QString status){
    this->pubKey = pubKey;
    this->txId = txId;
    QString shortPubKey = pubKey;
    QString shortTxId = txId;
    if(shortPubKey.length() > 20) {
        shortPubKey = shortPubKey.left(12) + "..." + shortPubKey.right(12);
    }
    if(shortTxId.length() > 20) {
        shortTxId = shortTxId.left(12) + "..." + shortTxId.right(12);
    }
    ui->textId->setText(shortPubKey);
    ui->textAddress->setText(address);
    ui->textAmount->setText(shortTxId);
    ui->textInputs->setText(outputIndex);
    ui->textStatus->setText(status);
}

void MnInfoDialog::copyInform(QString& copyStr, QString message){
    GUIUtil::setClipboard(copyStr);
    SnackBar *snackBar = new SnackBar(nullptr, this);
    snackBar->setText(tr(message.toStdString().c_str()));
    snackBar->resize(this->width(), snackBar->height());
    openDialog(snackBar, this);
    snackBar->deleteLater();
}

MnInfoDialog::~MnInfoDialog(){
    delete ui;
}
