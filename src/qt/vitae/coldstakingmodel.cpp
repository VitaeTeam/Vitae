// Copyright (c) 2019 The PIVX developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "qt/pivx/coldstakingmodel.h"
#include "uint256.h"
#include "bitcoinunits.h"
#include "guiutil.h"
#include <iostream>
#include "addressbook.h"

ColdStakingModel::ColdStakingModel(WalletModel* _model,
                                   TransactionTableModel* _tableModel,
                                   AddressTableModel* _addressTableModel,
                                   QObject *parent) : QAbstractTableModel(parent), model(_model), tableModel(_tableModel), addressTableModel(_addressTableModel){
    updateCSList();
}

void ColdStakingModel::updateCSList() {
    refresh();
    emit dataChanged(index(0, 0, QModelIndex()), index(rowCount(), COLUMN_COUNT, QModelIndex()) );
}

void ColdStakingModel::refresh() {
    cachedDelegations.clear();
    // First get all of the p2cs utxo inside the wallet
    std::vector<COutput> utxoList;
    pwalletMain->GetAvailableP2CSCoins(utxoList);

    if (!utxoList.empty()) {
        // Loop over each COutput into a CSDelegation
        for (const auto& utxo : utxoList) {

            const auto *wtx = utxo.tx;
            const QString txId = QString::fromStdString(wtx->GetHash().GetHex());
            const CTxOut& out = wtx->vout[utxo.i];

            // First parse the cs delegation
            CSDelegation delegation;
            if (!parseCSDelegation(out, delegation, txId, utxo.i))
                continue;

            // it's spendable only when this wallet has the keys to spend it, a.k.a is the owner
            delegation.isSpendable = pwalletMain->IsMine(out) & ISMINE_SPENDABLE_DELEGATED;
            delegation.cachedTotalAmount += out.nValue;
            delegation.delegatedUtxo.insert(txId, utxo.i);

            // Now verify if the delegation exists in the cached list
            int indexDel = cachedDelegations.indexOf(delegation);
            if (indexDel == -1) {
                // If it doesn't, let's append it.
                cachedDelegations.append(delegation);
            } else {
                CSDelegation& del = cachedDelegations[indexDel];
                del.delegatedUtxo.unite(delegation.delegatedUtxo);
                del.cachedTotalAmount += delegation.cachedTotalAmount;
            }
        }
    }
}

bool ColdStakingModel::parseCSDelegation(const CTxOut& out, CSDelegation& ret, const QString& txId, const int& utxoIndex) {
    CTxDestination stakingAddressDest;
    CTxDestination ownerAddressDest;

    if (!ExtractDestination(out.scriptPubKey, stakingAddressDest, true)) {
        return error("Error extracting staking destination for: %1 , output index: %2", txId.toStdString(), utxoIndex);
    }

    if (!ExtractDestination(out.scriptPubKey, ownerAddressDest, false)) {
        return error("Error extracting owner destination for: %1 , output index: %2", txId.toStdString(), utxoIndex);
    }

    std::string stakingAddressStr = CBitcoinAddress(
            stakingAddressDest,
            CChainParams::STAKING_ADDRESS
    ).ToString();

    std::string ownerAddressStr = CBitcoinAddress(
            ownerAddressDest,
            CChainParams::PUBKEY_ADDRESS
    ).ToString();

    ret = CSDelegation(stakingAddressStr, ownerAddressStr);

    return true;
}

int ColdStakingModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return cachedDelegations.size();
}

int ColdStakingModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return COLUMN_COUNT;
}


QVariant ColdStakingModel::data(const QModelIndex &index, int role) const
{
    if(!index.isValid())
            return QVariant();

    int row = index.row();
    CSDelegation rec = cachedDelegations[row];
    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        switch (index.column()) {
            case OWNER_ADDRESS:
                return QString::fromStdString(rec.ownerAddress);
            case OWNER_ADDRESS_LABEL:
                return addressTableModel->labelForAddress(QString::fromStdString(rec.ownerAddress));
            case STAKING_ADDRESS:
                return QString::fromStdString(rec.stakingAddress);
            case STAKING_ADDRESS_LABEL:
                return addressTableModel->labelForAddress(QString::fromStdString(rec.stakingAddress));
            case IS_WHITELISTED:
                return addressTableModel->purposeForAddress(rec.ownerAddress).compare(AddressBook::AddressBookPurpose::DELEGATOR) == 0;
            case IS_WHITELISTED_STRING:
                return (addressTableModel->purposeForAddress(rec.ownerAddress) == AddressBook::AddressBookPurpose::DELEGATOR ? "Staking" : "Not staking");
            case TOTAL_STACKEABLE_AMOUNT_STR:
                return GUIUtil::formatBalance(rec.cachedTotalAmount);
            case TOTAL_STACKEABLE_AMOUNT:
                return qint64(rec.cachedTotalAmount);
            case IS_RECEIVED_DELEGATION:
                return !rec.isSpendable;
        }
    }

    return QVariant();
}

bool ColdStakingModel::whitelist(const QModelIndex& modelIndex) {
    QString address = modelIndex.data(Qt::DisplayRole).toString();
    int idx = modelIndex.row();
    beginRemoveRows(QModelIndex(), idx, idx);
    bool ret = model->whitelistAddressFromColdStaking(address);
    endRemoveRows();
    emit dataChanged(index(idx, 0, QModelIndex()), index(idx, COLUMN_COUNT, QModelIndex()) );
    return ret;
}

bool ColdStakingModel::blacklist(const QModelIndex& modelIndex) {
    QString address = modelIndex.data(Qt::DisplayRole).toString();
    int idx = modelIndex.row();
    beginRemoveRows(QModelIndex(), idx, idx);
    bool ret = model->blacklistAddressFromColdStaking(address);
    endRemoveRows();
    emit dataChanged(index(idx, 0, QModelIndex()), index(idx, COLUMN_COUNT, QModelIndex()) );
    return ret;
}