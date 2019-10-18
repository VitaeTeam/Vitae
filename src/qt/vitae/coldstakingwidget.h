// Copyright (c) 2019 The PIVX developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef COLDSTAKINGWIDGET_H
#define COLDSTAKINGWIDGET_H

#include "qt/vitae/pwidget.h"
#include "qt/vitae/furabstractlistitemdelegate.h"
#include "qt/vitae/txviewholder.h"
#include "qt/vitae/tooltipmenu.h"
#include "qt/vitae/sendmultirow.h"
#include "qt/vitae/coldstakingmodel.h"
#include "qt/vitae/contactsdropdown.h"
#include "transactiontablemodel.h"
#include "coincontroldialog.h"

#include <QAction>
#include <QLabel>
#include <QWidget>

class VITAEGUI;
class WalletModel;
class CSDelegationHolder;

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
    void handleAddressClicked(const QModelIndex &index);
    void onCoinControlClicked();
    void onColdStakeClicked();
    void updateDisplayUnit();
    void showList(bool show);
    void onSendClicked();
    void onDelegateSelected(bool delegate);
    void onEditClicked();
    void onDeleteClicked();
    void onCopyClicked();
    void onCopyOwnerClicked();
    void onTxArrived(const QString& hash);
    void onContactsClicked(bool ownerAdd);
    void clearAll();
    void onLabelClicked();

private:
    Ui::ColdStakingWidget *ui;
    FurAbstractListItemDelegate *delegate = nullptr;
    TransactionTableModel* txModel = nullptr;
    ColdStakingModel* csModel = nullptr;
    CSDelegationHolder *txHolder = nullptr;
    CoinControlDialog *coinControlDialog = nullptr;
    QAction *btnOwnerContact;

    ContactsDropdown *menuContacts = nullptr;
    TooltipMenu* menu = nullptr;
    SendMultiRow *sendMultiRow = nullptr;
    bool isShowingDialog;

    bool isContactOwnerSelected;

    // Cached index
    QModelIndex index;

    int nDisplayUnit;

    void showAddressGenerationDialog(bool isPaymentRequest);
    void onContactsClicked();
};

#endif // COLDSTAKINGWIDGET_H
