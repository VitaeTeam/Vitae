// Copyright (c) 2019-2020 The PIVX developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "qt/vitae/fnmodel.h"

#include "fundamentalnode-sync.h"
#include "fundamentalnodeman.h"
#include "activefundamentalnode.h"
#include "sync.h"
#include "uint256.h"
#include "wallet.h"

FNModel::FNModel(QObject *parent) : QAbstractTableModel(parent){
    updateMNList();
}

void FNModel::updateMNList(){
    int end = nodes.size();
    nodes.clear();
    collateralTxAccepted.clear();
    for (CFundamentalnodeConfig::CFundamentalnodeEntry mne : fundamentalnodeConfig.getEntries()) {
        int nIndex;
        if(!mne.castOutputIndex(nIndex))
            continue;

        uint256 txHash(mne.getTxHash());
        CTxIn txIn(txHash, uint32_t(nIndex));
        CFundamentalnode* pmn = mnodeman.Find(txIn);
        if (!pmn){
            pmn = new CFundamentalnode();
            pmn->vin = txIn;
            pmn->activeState = CFundamentalnode::FUNDAMENTALNODE_MISSING;
        }
        nodes.insert(QString::fromStdString(mne.getAlias()), std::make_pair(QString::fromStdString(mne.getIp()), pmn));
        if(pwalletMain) {
            bool txAccepted = false;
            {
                LOCK2(cs_main, pwalletMain->cs_wallet);
                const CWalletTx *walletTx = pwalletMain->GetWalletTx(txHash);
                if (walletTx && walletTx->GetDepthInMainChain() > 0) {
                    txAccepted = true;
                }
            }
            collateralTxAccepted.insert(mne.getTxHash(), txAccepted);
        }
    }
    Q_EMIT dataChanged(index(0, 0, QModelIndex()), index(end, 5, QModelIndex()) );
}

int FNModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return nodes.size();
}

int FNModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return 6;
}


QVariant FNModel::data(const QModelIndex &index, int role) const
{
    if(!index.isValid())
            return QVariant();

    // rec could be null, always verify it.
    CFundamentalnode* rec = static_cast<CFundamentalnode*>(index.internalPointer());
    bool isAvailable = rec;
    int row = index.row();
    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        switch (index.column()) {
            case ALIAS:
                return nodes.uniqueKeys().value(row);
            case ADDRESS:
                return nodes.values().value(row).first;
            case PUB_KEY:
                return (isAvailable) ? QString::fromStdString(nodes.values().value(row).second->pubKeyFundamentalnode.GetHash().GetHex()) : "Not available";
            case COLLATERAL_ID:
                return (isAvailable) ? QString::fromStdString(rec->vin.prevout.hash.GetHex()) : "Not available";
            case COLLATERAL_OUT_INDEX:
                return (isAvailable) ? QString::number(rec->vin.prevout.n) : "Not available";
            case STATUS: {
                std::pair<QString, CFundamentalnode*> pair = nodes.values().value(row);
                return (pair.second) ? QString::fromStdString(pair.second->Status()) : "MISSING";
            }
            case PRIV_KEY: {
                for (CFundamentalnodeConfig::CFundamentalnodeEntry mne : fundamentalnodeConfig.getEntries()) {
                    if (mne.getTxHash().compare(rec->vin.prevout.hash.GetHex()) == 0){
                        return QString::fromStdString(mne.getPrivKey());
                    }
                }
                return "Not available";
            }
            case WAS_COLLATERAL_ACCEPTED:{
                if (!isAvailable) return false;
                std::string txHash = rec->vin.prevout.hash.GetHex();
                if(!collateralTxAccepted.value(txHash)){
                    bool txAccepted = false;
                    {
                        LOCK2(cs_main, pwalletMain->cs_wallet);
                        const CWalletTx *walletTx = pwalletMain->GetWalletTx(rec->vin.prevout.hash);
                        txAccepted = walletTx && walletTx->GetDepthInMainChain() > 0;
                    }
                    return txAccepted;
                }
                return true;
            }
        }
    }
    return QVariant();
}

QModelIndex FNModel::index(int row, int column, const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    std::pair<QString, CFundamentalnode*> pair = nodes.values().value(row);
    CFundamentalnode* data = pair.second;
    if (data) {
        return createIndex(row, column, data);
    } else if (!pair.first.isEmpty()){
        return createIndex(row, column, nullptr);
    } else {
        return QModelIndex();
    }
}


bool FNModel::removeMn(const QModelIndex& modelIndex) {
    QString alias = modelIndex.data(Qt::DisplayRole).toString();
    int idx = modelIndex.row();
    beginRemoveRows(QModelIndex(), idx, idx);
    nodes.take(alias);
    endRemoveRows();
    Q_EMIT dataChanged(index(idx, 0, QModelIndex()), index(idx, 5, QModelIndex()) );
    return true;
}

bool FNModel::addMn(CFundamentalnodeConfig::CFundamentalnodeEntry* mne){
    beginInsertRows(QModelIndex(), nodes.size(), nodes.size());
    int nIndex;
    if(!mne->castOutputIndex(nIndex))
        return false;

    CFundamentalnode* pmn = mnodeman.Find(CTxIn(uint256S(mne->getTxHash()), uint32_t(nIndex)));
    nodes.insert(QString::fromStdString(mne->getAlias()), std::make_pair(QString::fromStdString(mne->getIp()), pmn));
    endInsertRows();
    return true;
}

int FNModel::getMNState(QString mnAlias) {
    QMap<QString, std::pair<QString, CFundamentalnode*>>::const_iterator it = nodes.find(mnAlias);
    if (it != nodes.end()) return it.value().second->activeState;
    throw std::runtime_error(std::string("Masternode alias not found"));
}

bool FNModel::isMNInactive(QString mnAlias) {
    int activeState = getMNState(mnAlias);
    return activeState == CFundamentalnode::FUNDAMENTALNODE_MISSING || activeState == CFundamentalnode::FUNDAMENTALNODE_EXPIRED || activeState == CFundamentalnode::FUNDAMENTALNODE_REMOVE;
}

bool FNModel::isMNActive(QString mnAlias) {
    int activeState = getMNState(mnAlias);
    return activeState == CFundamentalnode::FUNDAMENTALNODE_PRE_ENABLED || activeState == CFundamentalnode::FUNDAMENTALNODE_ENABLED;
}

bool FNModel::isMNsNetworkSynced() {
    return fundamentalnodeSync.IsSynced();
}
