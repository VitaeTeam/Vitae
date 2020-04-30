#ifndef TXVIEWHOLDER_H
#define TXVIEWHOLDER_H

#include "qt/vitae/furlistrow.h"
#include "bitcoinunits.h"
#include <transactionfilterproxy.h>

class TxViewHolder : public FurListRow<QWidget*>
{
public:
    TxViewHolder();

    explicit TxViewHolder(bool _isLightTheme) : FurListRow(), isLightTheme(_isLightTheme){}

    QWidget* createHolder(int pos) override;

    void init(QWidget* holder,const QModelIndex &index, bool isHovered, bool isSelected) const override;

    QColor rectColor(bool isHovered, bool isSelected) override;

    ~TxViewHolder() override{};

    bool isLightTheme;

    void setDisplayUnit(int displayUnit){
        this->nDisplayUnit = displayUnit;
    }

    void setFilter(TransactionFilterProxy *_filter){
        this->filter = _filter;
    }

private:
    int nDisplayUnit;
    TransactionFilterProxy *filter = nullptr;
};

#endif // TXVIEWHOLDER_H
