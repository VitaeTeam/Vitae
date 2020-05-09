#include "qt/vitae/settings/settingsbittoolwidget.h"
#include "qt/vitae/settings/forms/ui_settingsbittoolwidget.h"
#include "qt/vitae/qtutils.h"

#include "guiutil.h"
#include "walletmodel.h"

#include "base58.h"
#include "bip38.h"
#include "init.h"
#include "wallet.h"
#include "askpassphrasedialog.h"

#include <string>
#include <vector>


SettingsBitToolWidget::SettingsBitToolWidget(VITAEGUI* _window, QWidget *parent) :
    PWidget(_window, parent),
    ui(new Ui::SettingsBitToolWidget)
{
    ui->setupUi(this);

    this->setStyleSheet(parent->styleSheet());

    /* Containers */
    ui->left->setProperty("cssClass", "container");
    ui->left->setContentsMargins(10,10,10,10);

    /* Title */
    ui->labelTitle->setText(tr("BIP38 Tool"));
    ui->labelTitle->setProperty("cssClass", "text-title-screen");

    //Button Group
    ui->pushLeft->setText(tr("Encrypt"));
    ui->pushLeft->setProperty("cssClass", "btn-check-left");
    ui->pushRight->setText(tr("Decrypt"));
    ui->pushRight->setProperty("cssClass", "btn-check-right");
    ui->pushLeft->setChecked(true);

    // Subtitle
    ui->labelSubtitle1->setText("Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat.");
    ui->labelSubtitle1->setProperty("cssClass", "text-subtitle");

    // Key
    ui->labelSubtitleKey->setText(tr("Encrypted key"));
    ui->labelSubtitleKey->setProperty("cssClass", "text-title");

    ui->lineEditKey->setPlaceholderText(tr("Enter a encrypted key"));
    initCssEditLine(ui->lineEditKey);

    // Passphrase
    ui->labelSubtitlePassphrase->setText(tr("Passphrase"));
    ui->labelSubtitlePassphrase->setProperty("cssClass", "text-title");

    ui->lineEditPassphrase->setPlaceholderText(tr("Enter a passphrase "));
    initCssEditLine(ui->lineEditPassphrase);

    // Buttons
    ui->pushButtonSave->setText(tr("DECRYPT KEY"));
    ui->pushButtonSave->setProperty("cssClass", "btn-primary");

    ui->pushButtonImport->setText(tr("Import Address"));
    ui->pushButtonImport->setProperty("cssClass", "btn-text-primary");

    connect(ui->pushLeft, &QPushButton::clicked, [this](){onEncryptSelected(true);});
    connect(ui->pushRight,  &QPushButton::clicked, [this](){onEncryptSelected(false);});


    // Encrypt

    // Address
    ui->labelSubtitleAddress->setText(tr("Enter a PIVX address"));
    ui->labelSubtitleAddress->setProperty("cssClass", "text-title");

    ui->addressIn_ENC->setPlaceholderText(tr("Add address"));
    ui->addressIn_ENC->setProperty("cssClass", "edit-primary-multi-book");
    ui->addressIn_ENC->setAttribute(Qt::WA_MacShowFocusRect, 0);
    setShadow(ui->addressIn_ENC);

    // Message
    ui->labelSubtitleMessage->setText(tr("Passphrase"));
    ui->labelSubtitleMessage->setProperty("cssClass", "text-title");

    ui->passphraseIn_ENC->setProperty("cssClass", "edit-primary");
    ui->passphraseIn_ENC->setPlaceholderText(tr("Write a message"));
    ui->passphraseIn_ENC->setProperty("cssClass","edit-primary");
    setShadow(ui->passphraseIn_ENC);
    ui->passphraseIn_ENC->setAttribute(Qt::WA_MacShowFocusRect, 0);

    ui->labelSubtitleEncryptedKey->setText(tr("Encrypted Key"));
    ui->labelSubtitleEncryptedKey->setProperty("cssClass", "text-title");
    ui->encryptedKeyOut_ENC->setPlaceholderText(tr("Encrypted key"));
    ui->encryptedKeyOut_ENC->setAttribute(Qt::WA_MacShowFocusRect, 0);
    ui->encryptedKeyOut_ENC->setReadOnly(true);
    initCssEditLine(ui->encryptedKeyOut_ENC);
    setShadow(ui->encryptedKeyOut_ENC);

    btnContact = ui->addressIn_ENC->addAction(QIcon("://ic-contact-arrow-down"), QLineEdit::TrailingPosition);
    ui->pushButtonEncrypt->setText(tr("ENCRYPT"));
    ui->pushButtonEncrypt->setProperty("cssClass", "btn-primary");
    ui->pushButtonClear->setText(tr("CLEAR ALL"));
    ui->pushButtonClear->setProperty("cssClass", "btn-secundary-clear");

    ui->statusLabel_ENC->setStyleSheet("QLabel { color: transparent; }");

    connect(ui->pushButtonEncrypt, SIGNAL(clicked()), this, SLOT(on_encryptKeyButton_ENC_clicked()));
    connect(btnContact, SIGNAL(triggered()), this, SLOT(onAddressesClicked()));
    connect(ui->pushButtonClear, SIGNAL(clicked()), this, SLOT(on_clear_all()));
}

void SettingsBitToolWidget::setAddress_ENC(const QString& address){
    ui->addressIn_ENC->setText(address);
    ui->passphraseIn_ENC->setFocus();
}

void SettingsBitToolWidget::onEncryptSelected(bool isEncr) {
    if (isEncr) {
        ui->stackedWidget->setCurrentIndex(1);
    } else {
        ui->stackedWidget->setCurrentIndex(0);
    }
}

QString specialChar = "\"@!#$%&'()*+,-./:;<=>?`{|}~^_[]\\";
QString validChar = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz" + specialChar;
bool isValidPassphrase(QString strPassphrase, QString& strInvalid)
{
    for (int i = 0; i < strPassphrase.size(); i++) {
        if (!validChar.contains(strPassphrase[i], Qt::CaseSensitive)) {
            if (QString("\"'").contains(strPassphrase[i]))
                continue;

            strInvalid = strPassphrase[i];
            return false;
        }
    }

    return true;
}

void SettingsBitToolWidget::on_encryptKeyButton_ENC_clicked()
{
    if (!walletModel)
        return;

    QString qstrPassphrase = ui->passphraseIn_ENC->text();
    QString strInvalid;
    if (!isValidPassphrase(qstrPassphrase, strInvalid)) {
        ui->statusLabel_ENC->setStyleSheet("QLabel { color: red; }");
        ui->statusLabel_ENC->setText(tr("The entered passphrase is invalid. ") + strInvalid + QString(" is not valid") + QString(" ") + tr("Allowed: 0-9,a-z,A-Z,") + specialChar);
        return;
    }

    CBitcoinAddress addr(ui->addressIn_ENC->text().toStdString());
    if (!addr.IsValid()) {
        ui->statusLabel_ENC->setStyleSheet("QLabel { color: red; }");
        ui->statusLabel_ENC->setText(tr("The entered address is invalid.") + QString(" ") + tr("Please check the address and try again."));
        return;
    }

    CKeyID keyID;
    if (!addr.GetKeyID(keyID)) {
        //ui->addressIn_ENC->setValid(false);
        ui->statusLabel_ENC->setStyleSheet("QLabel { color: red; }");
        ui->statusLabel_ENC->setText(tr("The entered address does not refer to a key.") + QString(" ") + tr("Please check the address and try again."));
        return;
    }

    WalletModel::UnlockContext ctx(walletModel->requestUnlock(AskPassphraseDialog::Context::BIP_38, true));
    if (!ctx.isValid()) {
        ui->statusLabel_ENC->setStyleSheet("QLabel { color: red; }");
        ui->statusLabel_ENC->setText(tr("Wallet unlock was cancelled."));
        return;
    }

    CKey key;
    if (!pwalletMain->GetKey(keyID, key)) {
        ui->statusLabel_ENC->setStyleSheet("QLabel { color: red; }");
        ui->statusLabel_ENC->setText(tr("Private key for the entered address is not available."));
        return;
    }

    std::string encryptedKey = BIP38_Encrypt(addr.ToString(), qstrPassphrase.toStdString(), key.GetPrivKey_256(), key.IsCompressed());
    ui->encryptedKeyOut_ENC->setText(QString::fromStdString(encryptedKey));

    ui->statusLabel_ENC->setStyleSheet("QLabel { color: green; }");
    ui->statusLabel_ENC->setText(QString("<nobr>") + tr("Address encrypted.") + QString("</nobr>"));
}

void SettingsBitToolWidget::on_clear_all(){
    ui->addressIn_ENC->clear();
    ui->passphraseIn_ENC->clear();
    ui->encryptedKeyOut_ENC->clear();
    ui->statusLabel_ENC->clear();
    ui->addressIn_ENC->setFocus();
}

void SettingsBitToolWidget::onAddressesClicked(){
    // TODO: Complete me..
}

SettingsBitToolWidget::~SettingsBitToolWidget()
{
    delete ui;
}
