// Copyright (c) 2019 The PIVX developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef FUNDAMENTALNODEWIZARDDIALOG_H
#define FUNDAMENTALNODEWIZARDDIALOG_H

#include <QDialog>
#include "walletmodel.h"
#include "qt/vitae/snackbar.h"
#include "fundamentalnodeconfig.h"
#include "qt/vitae/pwidget.h"

class WalletModel;

namespace Ui {
class FundamentalNodeWizardDialog;
class QPushButton;
}

class FundamentalNodeWizardDialog : public QDialog, public PWidget::Translator
{
    Q_OBJECT

public:
    explicit FundamentalNodeWizardDialog(WalletModel *walletMode, QWidget *parent = nullptr);
    ~FundamentalNodeWizardDialog();
    void showEvent(QShowEvent *event) override;
    QString translate(const char *msg) override { return tr(msg); }

    QString returnStr = "";
    bool isOk = false;
    CFundamentalnodeConfig::CFundamentalnodeEntry* mnEntry = nullptr;

private Q_SLOTS:
    void onNextClicked();
    void onBackClicked();
private:
    Ui::FundamentalNodeWizardDialog *ui;
    QPushButton* icConfirm1;
    QPushButton* icConfirm3;
    QPushButton* icConfirm4;
    SnackBar *snackBar = nullptr;
    int pos = 0;

    WalletModel *walletModel = nullptr;
    bool createMN();
    void inform(QString text);
    void initBtn(std::initializer_list<QPushButton*> args);
};

#endif // FUNDAMENTALNODEWIZARDDIALOG_H
