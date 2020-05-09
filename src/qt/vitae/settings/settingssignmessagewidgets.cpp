#include "qt/vitae/settings/settingssignmessagewidgets.h"
#include "qt/vitae/settings/forms/ui_settingssignmessagewidgets.h"
#include "qt/vitae/qtutils.h"
#include "addressbookpage.h"
#include "guiutil.h"
#include "walletmodel.h"

#include "base58.h"
#include "init.h"
#include "wallet.h"
#include "askpassphrasedialog.h"

#include <string>
#include <vector>

#include <QClipboard>

SettingsSignMessageWidgets::SettingsSignMessageWidgets(VITAEGUI* _window, QWidget *parent) :
    PWidget(_window, parent),
    ui(new Ui::SettingsSignMessageWidgets)
{
    ui->setupUi(this);

    this->setStyleSheet(parent->styleSheet());

    // Containers
    ui->left->setProperty("cssClass", "container");
    ui->left->setContentsMargins(10,10,10,10);

    // Title
    ui->labelTitle->setText(tr("Sign Message"));
    ui->labelTitle->setProperty("cssClass", "text-title-screen");

    // Subtitle
    ui->labelSubtitle1->setText("Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat.");
    ui->labelSubtitle1->setProperty("cssClass", "text-subtitle");

    // Address
    ui->labelSubtitleAddress->setText(tr("Enter a PIVX address or contact label"));
    ui->labelSubtitleAddress->setProperty("cssClass", "text-title");

    ui->addressIn_SM->setPlaceholderText(tr("Add address"));
    ui->addressIn_SM->setProperty("cssClass", "edit-primary-book");
    ui->addressIn_SM->setAttribute(Qt::WA_MacShowFocusRect, 0);
    setShadow(ui->addressIn_SM);

    // Message
    ui->labelSubtitleMessage->setText(tr("Message"));
    ui->labelSubtitleMessage->setProperty("cssClass", "text-title");

    ui->messageIn_SM->setPlaceholderText(tr("Write a message"));
    ui->messageIn_SM->setProperty("cssClass","edit-primary");
    setShadow(ui->messageIn_SM);
    ui->messageIn_SM->setAttribute(Qt::WA_MacShowFocusRect, 0);

    ui->labelSubtitleSignature->setText(tr("Signature"));
    ui->labelSubtitleSignature->setProperty("cssClass", "text-title");
    ui->signatureOut_SM->setPlaceholderText(tr("Signature"));
    ui->signatureOut_SM->setAttribute(Qt::WA_MacShowFocusRect, 0);
    initCssEditLine(ui->signatureOut_SM);
    setShadow(ui->signatureOut_SM);

    // Buttons
    ui->pushButtonSave->setText(tr("SIGN"));
    ui->pushButtonSave->setProperty("cssClass", "btn-primary");

    ui->statusLabel_SM->setVisible(false);

    connect(ui->pushButtonSave, SIGNAL(clicked()), this, SLOT(on_signMessageButton_SM_clicked()));
}

SettingsSignMessageWidgets::~SettingsSignMessageWidgets(){
    delete ui;
}


void SettingsSignMessageWidgets::setAddress_SM(const QString& address){
    ui->addressIn_SM->setText(address);
    ui->messageIn_SM->setFocus();
}

void SettingsSignMessageWidgets::on_addressBookButton_SM_clicked(){
    if (walletModel && walletModel->getAddressTableModel()) {
        AddressBookPage dlg(AddressBookPage::ForSelection, AddressBookPage::ReceivingTab, this);
        dlg.setModel(walletModel->getAddressTableModel());
        if (dlg.exec()) {
            setAddress_SM(dlg.getReturnValue());
        }
    }
}

void SettingsSignMessageWidgets::on_pasteButton_SM_clicked(){
    setAddress_SM(QApplication::clipboard()->text());
}

void SettingsSignMessageWidgets::on_signMessageButton_SM_clicked(){

    if (!walletModel)
        return;

    /* Clear old signature to ensure users don't get confused on error with an old signature displayed */
    ui->signatureOut_SM->clear();

    CBitcoinAddress addr(ui->addressIn_SM->text().toStdString());
    if (!addr.IsValid()) {
        ui->statusLabel_SM->setStyleSheet("QLabel { color: red; }");
        ui->statusLabel_SM->setText(tr("The entered address is invalid.") + QString(" ") + tr("Please check the address and try again."));
        return;
    }
    CKeyID keyID;
    if (!addr.GetKeyID(keyID)) {
        // TODO: change css..
        //ui->addressIn_SM->setValid(false);
        ui->statusLabel_SM->setStyleSheet("QLabel { color: red; }");
        ui->statusLabel_SM->setText(tr("The entered address does not refer to a key.") + QString(" ") + tr("Please check the address and try again."));
        return;
    }

    WalletModel::UnlockContext ctx(walletModel->requestUnlock(AskPassphraseDialog::Context::Sign_Message, true));
    if (!ctx.isValid()) {
        ui->statusLabel_SM->setStyleSheet("QLabel { color: red; }");
        ui->statusLabel_SM->setText(tr("Wallet unlock was cancelled."));
        return;
    }

    CKey key;
    if (!pwalletMain->GetKey(keyID, key)) {
        ui->statusLabel_SM->setStyleSheet("QLabel { color: red; }");
        ui->statusLabel_SM->setText(tr("Private key for the entered address is not available."));
        return;
    }

    CDataStream ss(SER_GETHASH, 0);
    ss << strMessageMagic;
    ss << ui->messageIn_SM->document()->toPlainText().toStdString();

    std::vector<unsigned char> vchSig;
    if (!key.SignCompact(Hash(ss.begin(), ss.end()), vchSig)) {
        ui->statusLabel_SM->setStyleSheet("QLabel { color: red; }");
        ui->statusLabel_SM->setText(QString("<nobr>") + tr("Message signing failed.") + QString("</nobr>"));
        return;
    }

    ui->statusLabel_SM->setStyleSheet("QLabel { color: green; }");
    ui->statusLabel_SM->setText(QString("<nobr>") + tr("Message signed.") + QString("</nobr>"));

    ui->signatureOut_SM->setText(QString::fromStdString(EncodeBase64(&vchSig[0], vchSig.size())));
}
