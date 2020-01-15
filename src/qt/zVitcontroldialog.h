// Copyright (c) 2017-2020 The PIVX developers
// Copyright (c) 2018-2020 The VITAE developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef ZVITCONTROLDIALOG_H
#define ZVITCONTROLDIALOG_H

#include <QDialog>
#include <QTreeWidgetItem>
#include "zvit/zerocoin.h"

class CZerocoinMint;
class WalletModel;

namespace Ui {
class ZVitControlDialog;
}

class CZVitControlWidgetItem : public QTreeWidgetItem
{
public:
    explicit CZVitControlWidgetItem(QTreeWidget *parent, int type = Type) : QTreeWidgetItem(parent, type) {}
    explicit CZVitControlWidgetItem(int type = Type) : QTreeWidgetItem(type) {}
    explicit CZVitControlWidgetItem(QTreeWidgetItem *parent, int type = Type) : QTreeWidgetItem(parent, type) {}

    bool operator<(const QTreeWidgetItem &other) const;
};

class ZVitControlDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ZVitControlDialog(QWidget *parent);
    ~ZVitControlDialog();

    void setModel(WalletModel* model);

    static std::set<std::string> setSelectedMints;
    static std::set<CMintMeta> setMints;
    static std::vector<CMintMeta> GetSelectedMints();

private:
    Ui::ZVitControlDialog *ui;
    WalletModel* model;

    void updateList();
    void updateLabels();

    enum {
        COLUMN_CHECKBOX,
        COLUMN_DENOMINATION,
        COLUMN_PUBCOIN,
        COLUMN_VERSION,
        COLUMN_CONFIRMATIONS,
        COLUMN_ISSPENDABLE
    };
    friend class CZVitControlWidgetItem;

private slots:
    void updateSelection(QTreeWidgetItem* item, int column);
    void ButtonAllClicked();
};

#endif // ZVITCONTROLDIALOG_H
