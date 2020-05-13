// Copyright (c) 2019 The PIVX developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "qt/vitae/txrow.h"
#include "qt/vitae/forms/ui_txrow.h"

#include "guiutil.h"
#include "qt/vitae/qtutils.h"

TxRow::TxRow(bool isLightTheme, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TxRow)
{
    ui->setupUi(this);
    setConfirmStatus(true);
    updateStatus(isLightTheme, false, false);
}

void TxRow::setConfirmStatus(bool isConfirm){
    if(isConfirm){
        setCssProperty(ui->lblAddress, "text-list-body1");
        setCssProperty(ui->lblDate, "text-list-caption");
    }else{
        setCssProperty(ui->lblAddress, "text-list-body-unconfirmed");
        setCssProperty(ui->lblDate,"text-list-caption-unconfirmed");
    }
}

void TxRow::updateStatus(bool isLightTheme, bool isHover, bool isSelected){
    if(isLightTheme)
        ui->lblDivisory->setStyleSheet("background-color:#bababa");
    else
        ui->lblDivisory->setStyleSheet("background-color:#40ffffff");
}

void TxRow::setDate(QDateTime date){
    ui->lblDate->setText(GUIUtil::dateTimeStr(date));
}

void TxRow::setLabel(QString str){
    ui->lblAddress->setText(str);
}

void TxRow::setAmount(QString str){
    ui->lblAmount->setText(str);
}

void TxRow::setType(bool isLightTheme, int type, bool isConfirmed){
    QString path;
    QString css;
    switch (type) {
        case TransactionRecord::ZerocoinMint:
            path = "://ic-transaction-mint";
            css = "text-list-amount-send";
            break;
        case TransactionRecord::Generated:
        case TransactionRecord::StakeZVIT:
        case TransactionRecord::MNReward:
        case TransactionRecord::StakeMint:
            path = "://ic-transaction-staked";
            css = "text-list-amount-receive";
            break;
        case TransactionRecord::RecvWithObfuscation:
        case TransactionRecord::RecvWithAddress:
        case TransactionRecord::RecvFromOther:
        case TransactionRecord::RecvFromZerocoinSpend:
            path = "://ic-transaction-received";
            css = "text-list-amount-receive";
            break;
        case TransactionRecord::SendToAddress:
        case TransactionRecord::SendToOther:
        case TransactionRecord::ZerocoinSpend:
        case TransactionRecord::ZerocoinSpend_Change_zVit:
        case TransactionRecord::ZerocoinSpend_FromMe:
            path = "://ic-transaction-sent";
            css = "text-list-amount-send";
            break;
        case TransactionRecord::SendToSelf:
            path = "://ic-transaction-mint";
            css = "text-list-amount-send";
            break;
        default:
            path = ":/icons/tx_inout";
            break;
    }

    if (!isLightTheme){
        path += "-dark";
    }

    if (!isConfirmed){
        css = "text-list-amount-unconfirmed";
        path += "-inactive";
        setConfirmStatus(false);
    }else{
        setConfirmStatus(true);
    }
    setCssProperty(ui->lblAmount, css);
    ui->icon->setIcon(QIcon(path));
}

TxRow::~TxRow(){
    delete ui;
}
