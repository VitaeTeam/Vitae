#ifndef SENDCONFIRMDIALOG_H
#define SENDCONFIRMDIALOG_H

#include <QDialog>
#include "walletmodeltransaction.h"

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
    explicit TxDetailDialog(QWidget *parent = nullptr, bool isConfirmDialog = true);
    ~TxDetailDialog();

    bool isConfirm() { return this->confirm;}
    WalletModel::SendCoinsReturn getStatus() { return this->sendStatus;}

    void setData(WalletModel *model, WalletModelTransaction &tx);
    void setData(WalletModel *model, QModelIndex &index);
    void setDisplayUnit(int unit){this->nDisplayUnit = unit;};

public slots:
    void acceptTx();
    void onInputsClicked();
    void onOutputsClicked();

private:
    Ui::TxDetailDialog *ui;
    int nDisplayUnit = 0;
    bool confirm = false;
    WalletModel *model = nullptr;
    WalletModel::SendCoinsReturn sendStatus;
    WalletModelTransaction *tx = nullptr;
    uint256 txHash;

    bool inputsLoaded = false;
    bool outputsLoaded = false;
};

#endif // SENDCONFIRMDIALOG_H
