#include "qt/pivx/mninfodialog.h"
#include "qt/pivx/forms/ui_mninfodialog.h"
#include "walletmodel.h"
#include "wallet/wallet.h"
#include "guiutil.h"
#include "qt/pivx/snackbar.h"
#include "qt/pivx/qtutils.h"
#include <QDateTime>

MnInfoDialog::MnInfoDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MnInfoDialog)
{
    ui->setupUi(this);
    this->setStyleSheet(parent->styleSheet());
    setCssProperty(ui->frame, "container-dialog");
    // Title
    ui->labelTitle->setText(tr("Master Node Information"));
    setCssProperty(ui->labelTitle, "text-title-dialog");
    // Labels
    setCssTextBodyDialog({ui->labelAmount, ui->labelSend, ui->labelInputs, ui->labelFee, ui->labelId, ui->labelSize, ui->labelConfirmations});
    setCssProperty({ui->labelDivider1, ui->labelDivider4, ui->labelDivider5, ui->labelDivider6, ui->labelDivider7, ui->labelDivider8, ui->labelDivider9}, "container-divider");
    setCssTextBodyDialog({ui->textAmount, ui->textAddress, ui->textInputs, ui->textStatus, ui->textId, ui->textSize, ui->textConfirmations});
    setCssProperty(ui->pushCopy, "ic-copy-big");
    setCssProperty(ui->pushCopyId, "ic-copy-big");
    setCssProperty(ui->btnEsc, "ic-close");
    ui->contentConfirmations->setVisible(false);
    ui->labelDivider7->setVisible(false);
    ui->contentSize->setVisible(false);
    ui->labelDivider5->setVisible(false);
    connect(ui->btnEsc, SIGNAL(clicked()), this, SLOT(close()));
    connect(ui->pushCopy, &QPushButton::clicked, [this](){
        GUIUtil::setClipboard(txId);
        SnackBar *snackBar = new SnackBar(nullptr, this);
        snackBar->setText(tr("Master Node public key copied"));
        snackBar->resize(this->width(), snackBar->height());
        openDialog(snackBar, this);
        snackBar->deleteLater();
    });
    connect(ui->pushCopyId, &QPushButton::clicked, [this](){
        GUIUtil::setClipboard(pubKey);
        SnackBar *snackBar = new SnackBar(nullptr, this);
        snackBar->setText(tr("Collateral tx id copied"));
        snackBar->resize(this->width(), snackBar->height());
        openDialog(snackBar, this);
        snackBar->deleteLater();
    });
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

MnInfoDialog::~MnInfoDialog()
{
    delete ui;
}
