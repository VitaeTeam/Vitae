// Copyright (c) 2019 The PIVX developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "qt/vitae/sendchangeaddressdialog.h"
#include "qt/vitae/forms/ui_sendchangeaddressdialog.h"

#include "coincontrol.h"
#include "qt/vitae/qtutils.h"

SendChangeAddressDialog::SendChangeAddressDialog(QWidget* parent, WalletModel* model) :
    QDialog(parent),
    walletModel(model),
    ui(new Ui::SendChangeAddressDialog)
{
    if (!walletModel) {
        throw std::runtime_error(strprintf("%s: No wallet model set", __func__));
    }
    ui->setupUi(this);
    this->setStyleSheet(parent->styleSheet());

    // Container
    ui->frame->setProperty("cssClass", "container-dialog");

    // Text
    ui->labelTitle->setText(tr("Custom Change Address"));
    ui->labelTitle->setProperty("cssClass", "text-title-dialog");

    ui->labelMessage->setText(tr("The remainder of the value resultant from the inputs minus the outputs value goes to the \"change\" PIVX address"));
    ui->labelMessage->setProperty("cssClass", "text-main-grey");

    ui->lineEditAddress->setPlaceholderText("Enter PIVX address (e.g D7VFR83SQbiezrW72hjc… ");
    initCssEditLine(ui->lineEditAddress, true);

    // Buttons
    ui->btnEsc->setText("");
    ui->btnEsc->setProperty("cssClass", "ic-close");

    ui->btnCancel->setProperty("cssClass", "btn-dialog-cancel");
    ui->btnSave->setText(tr("SAVE"));
    setCssBtnPrimary(ui->btnSave);

    connect(ui->btnEsc, &QPushButton::clicked, this, &SendChangeAddressDialog::close);
    connect(ui->btnCancel, &QPushButton::clicked, this, &SendChangeAddressDialog::reset);
    connect(ui->btnSave, &QPushButton::clicked, this, &SendChangeAddressDialog::save);
}

void SendChangeAddressDialog::setAddress(QString address)
{
    ui->lineEditAddress->setText(address);
    ui->btnCancel->setText(tr("RESET"));
}

QString SendChangeAddressDialog::getAddress() const
{
    return ui->lineEditAddress->text();
}

void SendChangeAddressDialog::showEvent(QShowEvent *event)
{
    if (ui->lineEditAddress) ui->lineEditAddress->setFocus();
}

void SendChangeAddressDialog::reset()
{
    if (!ui->lineEditAddress->text().isEmpty()) {
        ui->lineEditAddress->clear();
        ui->btnCancel->setText(tr("CANCEL"));
        CoinControlDialog::coinControl->destChange = CNoDestination();
    }
    close();
}

void SendChangeAddressDialog::save()
{
    // validate address
    if (!walletModel->validateAddress(ui->lineEditAddress->text())) {
        inform(tr("Invalid address"));
    } else {
        accept();
    }
}

void SendChangeAddressDialog::inform(const QString& text)
{
    if (!snackBar) snackBar = new SnackBar(nullptr, this);
    snackBar->setText(text);
    snackBar->resize(this->width(), snackBar->height());
    openDialog(snackBar, this);
}

SendChangeAddressDialog::~SendChangeAddressDialog()
{
    delete ui;
}
