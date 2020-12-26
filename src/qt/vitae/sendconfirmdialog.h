// Copyright (c) 2019-2020 The PIVX developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SENDCONFIRMDIALOG_H
#define SENDCONFIRMDIALOG_H

#include <QDialog>
#include "walletmodeltransaction.h"
#include "qt/vitae/snackbar.h"

class WalletModelTransaction;
class WalletModel;

namespace Ui {
class TxDetailDialog;
}

QT_BEGIN_NAMESPACE
class QModelIndex;
QT_END_NAMESPACE

class TxDetailDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TxDetailDialog(QWidget *parent = nullptr, bool isConfirmDialog = true, const QString& warningStr = QString());
    ~TxDetailDialog();

    void showEvent(QShowEvent *event) override;
    bool isConfirm() { return this->confirm;}
    WalletModel::SendCoinsReturn getStatus() { return this->sendStatus;}

    void setData(WalletModel *model, WalletModelTransaction& tx);
    void setData(WalletModel *model, const QModelIndex &index);
    void setDisplayUnit(int unit){this->nDisplayUnit = unit;};

public Q_SLOTS:
    void acceptTx();
    void onInputsClicked();
    void onOutputsClicked();
    void closeDialog();

private:
    Ui::TxDetailDialog *ui;
    SnackBar *snackBar = nullptr;
    int nDisplayUnit = 0;
    bool isConfirmDialog = false;
    bool confirm = false;
    WalletModel *model = nullptr;
    WalletModel::SendCoinsReturn sendStatus;
    WalletModelTransaction *tx = nullptr;
    uint256 txHash;

    bool inputsLoaded = false;
    bool outputsLoaded = false;

protected:
    void keyPressEvent(QKeyEvent *e) override;
};

#endif // SENDCONFIRMDIALOG_H
