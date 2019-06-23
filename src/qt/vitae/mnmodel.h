#ifndef MNMODEL_H
#define MNMODEL_H

#include <QAbstractTableModel>
#include "fundamentalnode.h"
#include "fundamentalnodeconfig.h"

class MNModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit MNModel(QObject *parent = nullptr);
    ~MNModel() override{}

    enum ColumnIndex {
        ALIAS = 0,  /**< User specified MN alias */
        ADDRESS = 1, /**< Node address */
        PROTO_VERSION = 2, /**< Node protocol version */
        STATUS = 3, /**< Node status */
        ACTIVE_TIMESTAMP = 4, /**<  */
        PUB_KEY = 5,
        COLLATERAL_ID = 6,
        COLLATERAL_OUT_INDEX = 7
    };

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QModelIndex index(int row, int column, const QModelIndex& parent) const override;
    bool removeMn(const QModelIndex& index);
    bool addMn(CFundamentalnodeConfig::CFundamentalnodeEntry* entry);
    void updateMNList();


private:
    // alias mn node ---> pair <ip, master node>
    QMap<QString, std::pair<QString,CFundamentalnode*>> nodes;
};

#endif // MNMODEL_H