// Copyright (c) 2011-2014 The Bitcoin developers
// Copyright (c) 2014-2015 The Dash developers
// Copyright (c) 2015-2017 The VITAE developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "openuridialog.h"
#include "ui_openuridialog.h"

#include "guiutil.h"
#include "walletmodel.h"
#include "qt/vitae/qtutils.h"

#include <QUrl>

OpenURIDialog::OpenURIDialog(QWidget* parent) : QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint),
                                                ui(new Ui::OpenURIDialog)
{
    ui->setupUi(this);
    this->setStyleSheet(parent->styleSheet());
    ui->uriEdit->setPlaceholderText("vitae:");

    ui->labelSubtitle->setText("URI");
    ui->labelSubtitle->setProperty("cssClass", "text-title2-dialog");

    ui->frame->setProperty("cssClass", "container-dialog");
    ui->labelTitle->setProperty("cssClass", "text-title-dialog");

    setCssBtnPrimary(ui->pushButtonOK);
    setCssBtnPrimary(ui->selectFileButton);
    ui->pushButtonCancel->setProperty("cssClass", "btn-dialog-cancel");

    ui->uriEdit->setPlaceholderText("0.000001 zVIT");
    initCssEditLine(ui->uriEdit, true);
    connect(ui->pushButtonOK, SIGNAL(clicked()), this, SLOT(accept()));
    connect(ui->pushButtonCancel, SIGNAL(clicked()), this, SLOT(close()));
}

OpenURIDialog::~OpenURIDialog()
{
    delete ui;
}

QString OpenURIDialog::getURI()
{
    return ui->uriEdit->text();
}

void OpenURIDialog::accept()
{
    SendCoinsRecipient rcp;
    if (GUIUtil::parseBitcoinURI(getURI(), &rcp)) {
        /* Only accept value URIs */
        QDialog::accept();
    } else {
        setCssEditLineDialog(ui->uriEdit, false, true);
    }
}

void OpenURIDialog::on_selectFileButton_clicked()
{
    QString filename = GUIUtil::getOpenFileName(this, tr("Select payment request file to open"), "", "", NULL);
    if (filename.isEmpty())
        return;
    QUrl fileUri = QUrl::fromLocalFile(filename);
    ui->uriEdit->setText("vitae:?r=" + QUrl::toPercentEncoding(fileUri.toString()));
}
