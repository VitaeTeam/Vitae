// Copyright (c) 2019 The PIVX developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SETTINGSEXPORTCSV_H
#define SETTINGSEXPORTCSV_H

#include <QWidget>
#include "qt/vitae/pwidget.h"
#include "transactionfilterproxy.h"

namespace Ui {
class SettingsExportCSV;
}

class SettingsExportCSV : public PWidget
{
    Q_OBJECT

public:
    explicit SettingsExportCSV(VITAEGUI* _window, QWidget *parent = nullptr);
    ~SettingsExportCSV();

private Q_SLOTS:
    void selectFileOutput(const bool& isTxExport);
    void exportClicked();
    void onExportAddressesClicked();

private:
    Ui::SettingsExportCSV *ui;
    TransactionFilterProxy* txFilter{nullptr};
    QString filename;
    QString filenameAddressBook;
};

#endif // SETTINGSEXPORTCSV_H
