#include "qt/vitae/sendconfirmdialog.h"
#include "qt/vitae/forms/ui_sendconfirmdialog.h"
#include "bitcoinunits.h"
#include "walletmodel.h"
#include "transactiontablemodel.h"
#include "transactionrecord.h"
#include "wallet.h"
#include "guiutil.h"
#include <QList>
#include <QDateTime>

TxDetailDialog::TxDetailDialog(QWidget *parent, bool isConfirmDialog) :
    QDialog(parent),
    ui(new Ui::TxDetailDialog)
{
    ui->setupUi(this);

    this->setStyleSheet(parent->styleSheet());

    // Container

    ui->frame->setProperty("cssClass", "container-dialog");

    // Title

    ui->labelTitle->setText("Confirm your transaction");
    ui->labelTitle->setProperty("cssClass", "text-title-dialog");

    // Labels

    ui->labelAmount->setProperty("cssClass", "text-body1-dialog");
    ui->labelSend->setProperty("cssClass", "text-body1-dialog");
    ui->labelInputs->setProperty("cssClass", "text-body1-dialog");
    ui->labelFee->setProperty("cssClass", "text-body1-dialog");
    ui->labelChange->setProperty("cssClass", "text-body1-dialog");
    ui->labelId->setProperty("cssClass", "text-body1-dialog");
    ui->labelSize->setProperty("cssClass", "text-body1-dialog");
    ui->labelStatus->setProperty("cssClass", "text-body1-dialog");
    ui->labelConfirmations->setProperty("cssClass", "text-body1-dialog");
    ui->labelDate->setProperty("cssClass", "text-body1-dialog");

    ui->contentInputs->setProperty("cssClass", "layout-arrow");

    ui->labelDivider1->setProperty("cssClass", "container-divider");
    ui->labelDivider2->setProperty("cssClass", "container-divider");
    ui->labelDivider3->setProperty("cssClass", "container-divider");
    ui->labelDivider4->setProperty("cssClass", "container-divider");
    ui->labelDivider5->setProperty("cssClass", "container-divider");
    ui->labelDivider6->setProperty("cssClass", "container-divider");
    ui->labelDivider7->setProperty("cssClass", "container-divider");
    ui->labelDivider8->setProperty("cssClass", "container-divider");
    ui->labelDivider9->setProperty("cssClass", "container-divider");


    // Content

    ui->textAmount->setProperty("cssClass", "text-body1-dialog");
    ui->textSend->setProperty("cssClass", "text-body1-dialog");
    ui->textInputs->setProperty("cssClass", "text-body1-dialog");
    ui->textFee->setProperty("cssClass", "text-body1-dialog");
    ui->textChange->setProperty("cssClass", "text-body1-dialog");
    ui->textId->setProperty("cssClass", "text-body1-dialog");
    ui->textSize->setProperty("cssClass", "text-body1-dialog");
    ui->textStatus->setProperty("cssClass", "text-body1-dialog");
    ui->textConfirmations->setProperty("cssClass", "text-body1-dialog");
    ui->textDate->setProperty("cssClass", "text-body1-dialog");

    ui->btnEsc->setText("");
    ui->btnEsc->setProperty("cssClass", "ic-close");
    ui->contentChangeAddress->setVisible(false);
    ui->labelDivider4->setVisible(false);
    if(isConfirmDialog){

        ui->btnCancel->setProperty("cssClass", "btn-dialog-cancel");
        ui->btnSave->setText("SEND");
        ui->btnSave->setProperty("cssClass", "btn-primary");

        // hide change address for now
        ui->contentConfirmations->setVisible(false);
        ui->contentStatus->setVisible(false);
        ui->contentDate->setVisible(false);
        ui->contentSize->setVisible(false);
        ui->contentConfirmations->setVisible(false);
        ui->contentID->setVisible(false);
        ui->labelDivider7->setVisible(false);
        ui->labelDivider5->setVisible(false);
        ui->labelDivider3->setVisible(false);

        connect(ui->btnCancel, SIGNAL(clicked()), this, SLOT(close()));
        connect(ui->btnSave, &QPushButton::clicked, [this](){acceptTx();});
    }else{
        ui->containerButtons->setVisible(false);

    }


    connect(ui->btnEsc, SIGNAL(clicked()), this, SLOT(close()));

}

void TxDetailDialog::setData(WalletModel *model, QModelIndex &index){
    TransactionRecord *rec = static_cast<TransactionRecord*>(index.internalPointer());
    QDateTime date = index.data(TransactionTableModel::DateRole).toDateTime();
    QString address = index.data(Qt::DisplayRole).toString();
    qint64 amount = index.data(TransactionTableModel::AmountRole).toLongLong();
    bool isConfirmed = index.data(TransactionTableModel::ConfirmedRole).toBool();
    QString amountText = BitcoinUnits::formatWithUnit(nDisplayUnit, amount, true, BitcoinUnits::separatorAlways);

    ui->textAmount->setText(amountText);

    const CWalletTx* tx = model->getTx(rec->hash);
    if(tx) {
        ui->textId->setText(QString::fromStdString(tx->GetHash().GetHex()));
        ui->textId->setTextInteractionFlags(Qt::TextSelectableByMouse);
        if (tx->vout.size() == 1) {
            ui->textSend->setText(address);
        } else {
            ui->textSend->setText(QString::number(tx->vout.size()) + " recipients");
        }
        ui->textInputs->setText(QString::number(tx->vin.size()));
        ui->textConfirmations->setText(QString::number(tx->GetDepthInMainChain()));
        ui->textDate->setText(GUIUtil::dateTimeStr(date));
        ui->textStatus->setText(QString::fromStdString(rec->statusToString()));
        // TODO: Complete me..
        //unsigned int nBytes = ::GetSerializeSize(*(CTransaction*)&tx, SER_NETWORK, PROTOCOL_VERSION);
        //ui->textSize->setText(QString::number(nBytes) + " bytes");
    }

}

void TxDetailDialog::setData(WalletModel *model, WalletModelTransaction &tx){
    this->model = model;
    this->tx = &tx;
    CAmount txFee = tx.getTransactionFee();
    CAmount totalAmount = tx.getTotalTransactionAmount() + txFee;

    ui->textAmount->setText(BitcoinUnits::formatWithUnit(nDisplayUnit, totalAmount, false, BitcoinUnits::separatorAlways) + " (Fee included)");
    if(tx.getRecipients().size() == 1){
        ui->textSend->setText(tx.getRecipients().at(0).address);
    }else{
        ui->textSend->setText(QString::number(tx.getRecipients().size()) + " recipients");
    }
    ui->textInputs->setText(QString::number(tx.getTransaction()->vin.size()));
    ui->textFee->setText(BitcoinUnits::formatWithUnit(nDisplayUnit, txFee, false, BitcoinUnits::separatorAlways));
}

void TxDetailDialog::acceptTx(){
    this->confirm = true;
    this->sendStatus = model->sendCoins(*this->tx);
    accept();
}

TxDetailDialog::~TxDetailDialog()
{
    delete ui;
}
