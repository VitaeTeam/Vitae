#include "qt/vitae/txviewholder.h"
#include "qt/vitae/txrow.h"

#include "qt/vitae/qtutils.h"
#include "transactiontablemodel.h"
#include "transactionrecord.h"
#include <QModelIndex>
#include <iostream>

#define ADDRESS_SIZE 12

QWidget* TxViewHolder::createHolder(int pos){
    return new TxRow(isLightTheme);
}

void TxViewHolder::init(QWidget* holder,const QModelIndex &index, bool isHovered, bool isSelected) const{
    TxRow *txRow = static_cast<TxRow*>(holder);
    txRow->updateStatus(isLightTheme, isHovered, isSelected);

    QModelIndex rIndex = (filter) ? filter->mapToSource(index) : index;
    TransactionRecord *rec = static_cast<TransactionRecord*>(rIndex.internalPointer());
    QDateTime date = rIndex.data(TransactionTableModel::DateRole).toDateTime();
    qint64 amount = rIndex.data(TransactionTableModel::AmountRole).toLongLong();
    //bool isConfirmed = rIndex.data(TransactionTableModel::ConfirmedRole).toBool();
    QString amountText = BitcoinUnits::formatWithUnit(nDisplayUnit, amount, true, BitcoinUnits::separatorAlways);
    QModelIndex indexType = rIndex.sibling(rIndex.row(),TransactionTableModel::Type);
    bool isUnconfirmed = (rec->status.status == TransactionStatus::Unconfirmed) || (rec->status.status == TransactionStatus::Immature);
    QString label = indexType.data(Qt::DisplayRole).toString();
    if(rec->type != TransactionRecord::ZerocoinMint &&
            rec->type !=  TransactionRecord::ZerocoinSpend_Change_zVit &&
            rec->type !=  TransactionRecord::StakeZVIT){
        QString address = rIndex.data(Qt::DisplayRole).toString();
        if(address.length() > 20) {
            address = address.left(ADDRESS_SIZE) + "..." + address.right(ADDRESS_SIZE);
        }
        label += " " + address;
    }

    txRow->setDate(date);
    txRow->setLabel(label);
    txRow->setAmount(amountText);
    txRow->setType(isLightTheme, rec->type, !isUnconfirmed);
}

QColor TxViewHolder::rectColor(bool isHovered, bool isSelected) {
    return getRowColor(isLightTheme, isHovered, isSelected);
}
