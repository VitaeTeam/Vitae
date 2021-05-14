// Copyright (c) 2019 The PIVX developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef FNROW_H
#define FNROW_H

#include <QWidget>

namespace Ui {
class FNRow;
}

class FNRow : public QWidget
{
    Q_OBJECT

public:
    explicit FNRow(QWidget *parent = nullptr);
    ~FNRow();

    void updateView(QString address, QString label, QString status, bool wasCollateralAccepted);

signals:
    void onMenuClicked();
private:
    Ui::FNRow *ui;
};

#endif // MNROW_H
