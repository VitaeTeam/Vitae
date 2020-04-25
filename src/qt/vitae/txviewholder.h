#ifndef TXVIEWHOLDER_H
#define TXVIEWHOLDER_H

#include "qt/vitae/furlistrow.h"


class TxViewHolder : public FurListRow<QWidget*>
{
public:
    TxViewHolder();

    explicit TxViewHolder(bool _isLightTheme) : FurListRow(), isLightTheme(_isLightTheme){}

    QWidget* createHolder(int pos) override;

    QColor rectColor(bool isHovered, bool isSelected) override;

    ~TxViewHolder() override{};

    bool isLightTheme;
};

#endif // TXVIEWHOLDER_H
