// Copyright (c) 2019 The PIVX developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef CSROW_H
#define CSROW_H

#include <QWidget>

namespace Ui {
class CSRow;
}

class CSRow : public QWidget
{
    Q_OBJECT

public:
    explicit CSRow(QWidget *parent = nullptr);
    ~CSRow();

    void updateView(QString address, QString label, QString active, QString amount);
    void updateState(bool isLightTheme, bool isHovered, bool isSelected);
protected:
    virtual void enterEvent(QEvent *);
    virtual void leaveEvent(QEvent *);

private:
    Ui::CSRow *ui;
};

#endif // CSROW_H
