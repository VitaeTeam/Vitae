#include "qt/vitae/mnmodel.h"

#include "fundamentalnode-sync.h"
#include "fundamentalnodeman.h"
#include "activefundamentalnode.h"
#include "sync.h"

MNModel::MNModel(QObject *parent) : QAbstractTableModel(parent){
    updateMNList();
}

void MNModel::updateMNList(){
    int end = nodes.size();
    nodes.clear();
    for (CFundamentalnodeConfig::CFundamentalnodeEntry mne : fundamentalnodeConfig.getEntries()) {
        int nIndex;
        if(!mne.castOutputIndex(nIndex))
            continue;

        CFundamentalnode* pmn = mnodeman.Find(CTxIn(uint256S(mne.getTxHash()), uint32_t(nIndex)));
        nodes.insert(QString::fromStdString(mne.getAlias()), std::make_pair(QString::fromStdString(mne.getIp()), pmn));
    }
    emit dataChanged(index(0, 0, QModelIndex()), index(end, 5, QModelIndex()) );
}

int MNModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return nodes.size();
}

int MNModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return 5;
}


QVariant MNModel::data(const QModelIndex &index, int role) const
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
                return (isAvailable) ? QString::fromStdString(nodes.values().value(row).second->pubKeyFundamentalnode.GetHash().GetHex()) : "No available";
            case COLLATERAL_ID:
                return (isAvailable) ? QString::fromStdString(rec->vin.prevout.hash.GetHex()) : "No available";
            case COLLATERAL_OUT_INDEX:
                return (isAvailable) ? QString::number(rec->vin.prevout.n) : "No available";
            case STATUS: {
                std::pair<QString, CFundamentalnode*> pair = nodes.values().value(row);
                return (pair.second) ? QString::fromStdString(pair.second->Status()) : "MISSING";
            }
        }
    }
    return QVariant();
}

QModelIndex MNModel::index(int row, int column, const QModelIndex& parent) const
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


bool MNModel::removeMn(const QModelIndex& modelIndex) {
    QString alias = modelIndex.data(Qt::DisplayRole).toString();
    int idx = modelIndex.row();
    beginRemoveRows(QModelIndex(), idx, idx);
    nodes.take(alias);
    endRemoveRows();
    emit dataChanged(index(idx, 0, QModelIndex()), index(idx, 5, QModelIndex()) );
    return true;
}

bool MNModel::addMn(CFundamentalnodeConfig::CFundamentalnodeEntry* mne){
    beginInsertRows(QModelIndex(), nodes.size(), nodes.size());
    int nIndex;
    if(!mne->castOutputIndex(nIndex))
        return false;

    CFundamentalnode* pmn = mnodeman.Find(CTxIn(uint256S(mne->getTxHash()), uint32_t(nIndex)));
    nodes.insert(QString::fromStdString(mne->getAlias()), std::make_pair(QString::fromStdString(mne->getIp()), pmn));
    endInsertRows();
    return true;
}
