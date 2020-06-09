// Copyright (c) 2019 The PIVX developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef COINCONTROLPIVWIDGET_H
#define COINCONTROLPIVWIDGET_H

#include <QDialog>

namespace Ui {
class CoinControlVitWidget;
}

class CoinControlVitWidget : public QDialog
{
    Q_OBJECT

public:
    explicit CoinControlVitWidget(QWidget *parent = nullptr);
    ~CoinControlVitWidget();

private:
    Ui::CoinControlVitWidget *ui;
};

#endif // COINCONTROLPIVWIDGET_H
