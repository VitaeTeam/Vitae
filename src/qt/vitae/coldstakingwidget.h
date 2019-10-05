// Copyright (c) 2019 The PIVX developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef COLDSTAKINGWIDGET_H
#define COLDSTAKINGWIDGET_H

#include "qt/vitae/pwidget.h"
#include "qt/vitae/furabstractlistitemdelegate.h"
#include "qt/vitae/txviewholder.h"
#include "qt/vitae/sendmultirow.h"
#include "transactiontablemodel.h"
#include "transactionfilterproxy.h"
#include "coincontroldialog.h"

#include <QLabel>
#include <QWidget>

class VITAEGUI;
class WalletModel;

namespace Ui {
class ColdStakingWidget;
}

QT_BEGIN_NAMESPACE
class QModelIndex;
QT_END_NAMESPACE

class ColdStakingWidget : public PWidget
{
    Q_OBJECT

public:
    explicit ColdStakingWidget(VITAEGUI* parent);
    ~ColdStakingWidget();

    void loadWalletModel() override;
private slots:
    void changeTheme(bool isLightTheme, QString &theme) override;
    void onCoinControlClicked();
    void updateDisplayUnit();
    void showList(bool show);
    void onSendClicked();
    void onDelegateSelected(bool delegate);

private:
    Ui::ColdStakingWidget *ui;
    FurAbstractListItemDelegate *delegate = nullptr;
    TransactionTableModel* txModel = nullptr;
    TxViewHolder *txHolder = nullptr;
    TransactionFilterProxy* filter = nullptr;
    CoinControlDialog *coinControlDialog = nullptr;

    SendMultiRow *sendMultiRow = nullptr;

    int nDisplayUnit;
    void clearAll();
};

#endif // COLDSTAKINGWIDGET_H
