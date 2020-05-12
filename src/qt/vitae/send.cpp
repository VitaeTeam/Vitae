#include "qt/vitae/send.h"
#include "qt/vitae/forms/ui_send.h"

#include "qt/vitae/addnewcontactdialog.h"
#include "qt/vitae/qtutils.h"
#include "qt/vitae/sendchangeaddressdialog.h"
#include "qt/vitae/optionbutton.h"
#include "qt/vitae/sendcustomfeedialog.h"
#include "qt/vitae/sendconfirmdialog.h"
#include "qt/vitae/myaddressrow.h"
#include "optionsmodel.h"
#include "addresstablemodel.h"
#include "coincontrol.h"
#include "script/standard.h"
#include "zvit/deterministicmint.h"
#include "openuridialog.h"
#include "qt/zVitcontroldialog.h"

#include <iostream>

SendWidget::SendWidget(VITAEGUI* parent) :
    PWidget(parent),
    ui(new Ui::send),
    coinIcon(new QPushButton()),
    btnContacts(new QPushButton())
{
    ui->setupUi(this);

    this->setStyleSheet(parent->styleSheet());

    /* Containers */
    ui->left->setProperty("cssClass", "container");
    ui->left->setContentsMargins(0,20,0,20);
    ui->right->setProperty("cssClass", "container-right");
    ui->right->setContentsMargins(20,10,20,20);

    /* Light Font */
    QFont fontLight;
    fontLight.setWeight(QFont::Light);

    /* Title */
    ui->labelTitle->setText(tr("Send"));
    ui->labelTitle->setProperty("cssClass", "text-title-screen");
    ui->labelTitle->setFont(fontLight);

    /* Button Group */
    ui->pushLeft->setText("PIV");
    ui->pushLeft->setProperty("cssClass", "btn-check-left");
    ui->pushLeft->setChecked(true);
    ui->pushRight->setText("zPIV");
    ui->pushRight->setProperty("cssClass", "btn-check-right");

    /* Subtitle */

    ui->labelSubtitle1->setText(tr("You can transfer public coins: PIV or private ones: zPIV"));
    ui->labelSubtitle1->setProperty("cssClass", "text-subtitle");

    ui->labelSubtitle2->setText(tr("Select coin type to spend"));
    ui->labelSubtitle2->setProperty("cssClass", "text-subtitle");

    /* Address */

    ui->labelSubtitleAddress->setText(tr("Enter a PIVX address or contact label"));
    ui->labelSubtitleAddress->setProperty("cssClass", "text-title");


    /* Amount */

    ui->labelSubtitleAmount->setText(tr("Amount"));
    ui->labelSubtitleAmount->setProperty("cssClass", "text-title");

    // Buttons

    ui->pushButtonFee->setText(tr("Standard Fee %1").arg("0.000005 PIV"));
    ui->pushButtonFee->setProperty("cssClass", "btn-secundary");

    ui->pushButtonClear->setText(tr("Clear all"));
    ui->pushButtonClear->setProperty("cssClass", "btn-secundary-clear");

    ui->pushButtonAddRecipient->setText(tr("Add recipient"));
    ui->pushButtonAddRecipient->setProperty("cssClass", "btn-secundary-add");

    ui->pushButtonSave->setProperty("cssClass", "btn-primary");

    ui->pushButtonReset->setText(tr("Reset to default"));
    ui->pushButtonReset->setProperty("cssClass", "btn-secundary");

    // Coin control
    ui->btnCoinControl->setTitleClassAndText("btn-title-grey", "Control coin");
    ui->btnCoinControl->setSubTitleClassAndText("text-subtitle", "Select the source of the coins.");

    // Change address option
    ui->btnChangeAddress->setTitleClassAndText("btn-title-grey", "Change address");
    ui->btnChangeAddress->setSubTitleClassAndText("text-subtitle", "Customize the change address.");

    // Uri
    ui->btnUri->setTitleClassAndText("btn-title-grey", "Open URI");
    ui->btnUri->setSubTitleClassAndText("text-subtitle", "Parse a payment request.");
    ui->btnUri->setVisible(false);

    connect(ui->pushButtonFee, SIGNAL(clicked()), this, SLOT(onChangeCustomFeeClicked()));
    connect(ui->btnCoinControl, SIGNAL(clicked()), this, SLOT(onCoinControlClicked()));
    connect(ui->btnChangeAddress, SIGNAL(clicked()), this, SLOT(onChangeAddressClicked()));
    connect(ui->btnUri, SIGNAL(clicked()), this, SLOT(onOpenUriClicked()));

    ui->coinWidget->setProperty("cssClass", "container-coin-type");
    ui->labelLine->setProperty("cssClass", "container-divider");


    // Total Send

    ui->labelTitleTotalSend->setText(tr("Total to send"));
    ui->labelTitleTotalSend->setProperty("cssClass", "text-title");

    ui->labelAmountSend->setText("0.00 zPIV");
    ui->labelAmountSend->setProperty("cssClass", "text-body1");

    // Total Remaining

    ui->labelTitleTotalRemaining->setText(tr("Total remaining"));
    ui->labelTitleTotalRemaining->setProperty("cssClass", "text-title");

    ui->labelAmountRemaining->setProperty("cssClass", "text-body1");

    // Icon Send
    ui->stackedWidget->addWidget(coinIcon);
    coinIcon->show();
    coinIcon->raise();

    coinIcon->setProperty("cssClass", "coin-icon-zpiv");

    QSize BUTTON_SIZE = QSize(24, 24);
    coinIcon->setMinimumSize(BUTTON_SIZE);
    coinIcon->setMaximumSize(BUTTON_SIZE);

    int posX = 0;
    int posY = 20;
    coinIcon->move(posX, posY);

    // Entry
    addEntry();

    // Connect
    connect(ui->pushLeft, &QPushButton::clicked, [this](){onPIVSelected(true);});
    connect(ui->pushRight,  &QPushButton::clicked, [this](){onPIVSelected(false);});
    connect(ui->pushButtonSave, SIGNAL(clicked()), this, SLOT(onSendClicked()));
    connect(ui->pushButtonAddRecipient, SIGNAL(clicked()), this, SLOT(onAddEntryClicked()));
    connect(ui->pushButtonClear, SIGNAL(clicked()), this, SLOT(clearAll()));
}

void SendWidget::refreshView(){
    QString btnText;
    if(ui->pushLeft->isChecked()){
        btnText = tr("Send PIV");
        ui->pushButtonAddRecipient->setVisible(true);
    }else{
        btnText = tr("Send zPIV");
        ui->pushButtonAddRecipient->setVisible(false);
    }
    ui->pushButtonSave->setText(btnText);

    refreshAmounts();
}

void SendWidget::refreshAmounts() {

    CAmount total = 0;
    QMutableListIterator<SendMultiRow*> it(entries);
    while (it.hasNext()) {
        SendMultiRow* entry = it.next();
        CAmount amount = entry->getAmountValue();
        if (amount > 0)
            total += amount;
    }

    bool isZpiv = ui->pushRight->isChecked();
    int nDisplayUnit = walletModel->getOptionsModel()->getDisplayUnit();

    ui->labelAmountSend->setText(GUIUtil::formatBalance(total, nDisplayUnit, isZpiv));
    ui->labelAmountRemaining->setText(
            GUIUtil::formatBalance(
                    (isZpiv ? walletModel->getZerocoinBalance() : walletModel->getBalance()) - total,
                    nDisplayUnit,
                    isZpiv
                    )
    );
}

void SendWidget::loadClientModel(){
    if (clientModel) {
        // TODO: Complete me..
        //connect(clientModel, SIGNAL(numBlocksChanged(int)), this, SLOT(updateSmartFeeLabel()));
    }
}

void SendWidget::loadWalletModel() {
    if (walletModel && walletModel->getOptionsModel()) {
        for(SendMultiRow *entry : entries){
            if(entry){
                entry->setWalletModel(walletModel);
            }
        }

        // Refresh view
        refreshView();

        // TODO: Coin control complet eme
        // Coin Control
        //connect(model->getOptionsModel(), SIGNAL(coinControlFeaturesChanged(bool)), this, SLOT(coinControlFeatureChanged(bool)));
        //ui->frameCoinControl->setVisible(model->getOptionsModel()->getCoinControlFeatures());
        //coinControlUpdateLabels();

        // TODO: fee section, check sendDialog, same method
    }
}

void SendWidget::clearAll(){
    CoinControlDialog::coinControl->SetNull();
    ui->btnChangeAddress->setActive(false);
    ui->btnCoinControl->setActive(false);
    clearEntries();
}

void SendWidget::clearEntries(){
    int num = entries.length();
    for (int i = 0; i < num; ++i) {
        ui->scrollAreaWidgetContents->layout()->takeAt(0)->widget()->deleteLater();
    }
    entries.clear();

    addEntry();
}

void SendWidget::addEntry(){
    if(entries.isEmpty()){
        createEntry();
    } else {
        if (entries.length() == 1) {
            SendMultiRow *entry = entries.at(0);
            entry->hideLabels();
            entry->setNumber(1);
        }else if(entries.length() == MAX_SEND_POPUP_ENTRIES){
            inform(tr("Maximum amount of outputs reached"));
            return;
        }

        SendMultiRow *sendMultiRow = createEntry();
        sendMultiRow->setNumber(entries.length());
        sendMultiRow->hideLabels();
    }
}

SendMultiRow* SendWidget::createEntry(){
    SendMultiRow *sendMultiRow = new SendMultiRow(this);
    if(this->walletModel) sendMultiRow->setWalletModel(this->walletModel);
    entries.append(sendMultiRow);
    ui->scrollAreaWidgetContents->layout()->addWidget(sendMultiRow);
    connect(sendMultiRow, &SendMultiRow::onContactsClicked, this, &SendWidget::onContactsClicked);
    connect(sendMultiRow, &SendMultiRow::onMenuClicked, this, &SendWidget::onMenuClicked);
    connect(sendMultiRow, &SendMultiRow::onValueChanged, this, &SendWidget::onValueChanged);
    return sendMultiRow;
}

void SendWidget::onUriParsed(SendCoinsRecipient rcp){
    // Amount changed..
}

void SendWidget::onAddEntryClicked(){
    // TODO: Validations here..
    addEntry();
}

void SendWidget::resizeEvent(QResizeEvent *event){
    resizeMenu();
    QWidget::resizeEvent(event);
}


void SendWidget::onSendClicked(){

    if (!walletModel || !walletModel->getOptionsModel())
        return;

    QList<SendCoinsRecipient> recipients;

    for (SendMultiRow* entry : entries){
        // TODO: Check what is the UTXO splitter here..

        // Validate send..
        if(entry && entry->validate()) {
            recipients.append(entry->getValue());
        }else{
            inform(tr("Invalid entry"));
            return;
        }

    }

    if (recipients.isEmpty()) {
        inform(tr("No set recipients"));
        return;
    }

    bool sendPiv = ui->pushLeft->isChecked();

    // request unlock only if was locked or unlocked for mixing:
    // this way we let users unlock by walletpassphrase or by menu
    // and make many transactions while unlocking through this dialog
    // will call relock
    if(!GUIUtil::requestUnlock(walletModel, sendPiv ? AskPassphraseDialog::Context::Send_PIV : AskPassphraseDialog::Context::Send_zPIV, true)){
        // Unlock wallet was cancelled
        inform(tr("Cannot send, wallet locked"));
        return;
    }

    if((sendPiv) ? send(recipients) : sendZpiv(recipients)) {
        updateEntryLabels(recipients);
    }
}

bool SendWidget::send(QList<SendCoinsRecipient> recipients){
    // prepare transaction for getting txFee earlier
    WalletModelTransaction currentTransaction(recipients);
    WalletModel::SendCoinsReturn prepareStatus;

    prepareStatus = walletModel->prepareTransaction(currentTransaction, CoinControlDialog::coinControl);

    // process prepareStatus and on error generate message shown to user
    processSendCoinsReturn(prepareStatus,
                           BitcoinUnits::formatWithUnit(walletModel->getOptionsModel()->getDisplayUnit(),
                                                        currentTransaction.getTransactionFee()),
                           true
    );

    if (prepareStatus.status != WalletModel::OK) {
        inform(tr("Prepare status failed.."));
        return false;
    }

    showHideOp(true);
    TxDetailDialog* dialog = new TxDetailDialog(window);
    dialog->setDisplayUnit(walletModel->getOptionsModel()->getDisplayUnit());
    dialog->setData(walletModel, currentTransaction);
    dialog->adjustSize();
    openDialogWithOpaqueBackgroundY(dialog, window, 3, 5);

    if(dialog->isConfirm()){
        // now send the prepared transaction
        WalletModel::SendCoinsReturn sendStatus = dialog->getStatus();
        // process sendStatus and on error generate message shown to user
        processSendCoinsReturn(sendStatus);

        if (sendStatus.status == WalletModel::OK) {
            CoinControlDialog::coinControl->UnSelectAll();
            clearAll();
            inform(tr("Transaction sent"));
            return true;
        }

    }

    dialog->deleteLater();
    return false;
}

bool SendWidget::sendZpiv(QList<SendCoinsRecipient> recipients){
    if (!walletModel || !walletModel->getOptionsModel())
        return false;

    if(GetAdjustedTime() > GetSporkValue(SPORK_20_ZEROCOIN_MAINTENANCE_MODE)) {
        emit message(tr("Spend Zerocoin"), tr("zVIT is currently undergoing maintenance."), CClientUIInterface::MSG_ERROR);
        return false;
    }

    std::list<std::pair<CBitcoinAddress*, CAmount>> outputs;
    CAmount total = 0;
    for (SendCoinsRecipient rec : recipients){
        total += rec.amount;
        outputs.push_back(std::pair<CBitcoinAddress*, CAmount>(new CBitcoinAddress(rec.address.toStdString()),rec.amount));
    }

    // use mints from zPIV selector if applicable
    std::vector<CMintMeta> vMintsToFetch;
    std::vector<CZerocoinMint> vMintsSelected;
    if (!ZVitControlDialog::setSelectedMints.empty()) {
        vMintsToFetch = ZVitControlDialog::GetSelectedMints();

        for (auto& meta : vMintsToFetch) {
            CZerocoinMint mint;
            if (!walletModel->getMint(meta.hashSerial, mint)){
                inform(tr("Coin control mint not found"));
                return false;
            }
            vMintsSelected.emplace_back(mint);
        }
    }

    QString sendBody = outputs.size() == 1 ?
            tr("Sending %1 to address %2\n")
            .arg(BitcoinUnits::formatWithUnit(walletModel->getOptionsModel()->getDisplayUnit(), total, false, BitcoinUnits::separatorAlways))
            .arg(recipients.first().address)
            :
           tr("Sending %1 to addresses:\n%2")
           .arg(BitcoinUnits::formatWithUnit(walletModel->getOptionsModel()->getDisplayUnit(), total, false, BitcoinUnits::separatorAlways))
           .arg(recipientsToString(recipients));

    bool ret = false;
    emit message(
            tr("Spend Zerocoin"),
            sendBody,
            CClientUIInterface::MSG_INFORMATION | CClientUIInterface::BTN_MASK | CClientUIInterface::MODAL,
            &ret);

    if(!ret) return false;

    CZerocoinSpendReceipt receipt;

    std::string changeAddress = "";
    if(!boost::get<CNoDestination>(&CoinControlDialog::coinControl->destChange)){
        changeAddress = CBitcoinAddress(CoinControlDialog::coinControl->destChange).ToString();
    }else{
        changeAddress = walletModel->getAddressTableModel()->getLastUnusedAddress().toStdString();
    }

    if (walletModel->sendZpiv(
            vMintsSelected,
            true,
            true,
            receipt,
            outputs,
            changeAddress
    )
            ) {
        inform(tr("zPIV transaction sent!"));
        ZVitControlDialog::setSelectedMints.clear();
        clearAll();
        return true;
    } else {
        QString body;
        if (receipt.GetStatus() == ZVIT_SPEND_V1_SEC_LEVEL) {
            body = tr("Version 1 zPIV require a security level of 100 to successfully spend.");
        } else {
            int nNeededSpends = receipt.GetNeededSpends(); // Number of spends we would need for this transaction
            const int nMaxSpends = Params().Zerocoin_MaxSpendsPerTransaction(); // Maximum possible spends for one zPIV transaction
            if (nNeededSpends > nMaxSpends) {
                body = tr("Too much inputs (") + QString::number(nNeededSpends, 10) +
                       tr(") needed.\nMaximum allowed: ") + QString::number(nMaxSpends, 10);
                body += tr(
                        "\nEither mint higher denominations (so fewer inputs are needed) or reduce the amount to spend.");
            } else {
                body = QString::fromStdString(receipt.GetStatusMessage());
            }
        }
        emit message("zPIV transaction failed", body, CClientUIInterface::MSG_ERROR);
        return false;
    }
}

QString SendWidget::recipientsToString(QList<SendCoinsRecipient> recipients){
    QString s = "";
    for (SendCoinsRecipient rec : recipients){
        s += rec.address + " -> " + BitcoinUnits::formatWithUnit(walletModel->getOptionsModel()->getDisplayUnit(), rec.amount, false, BitcoinUnits::separatorAlways) + "\n";
    }
    return s;
}

void SendWidget::updateEntryLabels(QList<SendCoinsRecipient> recipients){
    for (SendCoinsRecipient rec : recipients){
        QString label = rec.label;
        if(!label.isNull()) {
            QString labelOld = walletModel->getAddressTableModel()->labelForAddress(rec.address);
            if(label.compare(labelOld) != 0) {
                CTxDestination dest = CBitcoinAddress(rec.address.toStdString()).Get();
                if (!walletModel->updateAddressBookLabels(dest, label.toStdString(),
                                                          this->walletModel->isMine(dest) ? "receive" : "send")) {
                    // Label update failed
                    emit message("", tr("Address label update failed for address: %1").arg(rec.address), CClientUIInterface::MSG_ERROR);
                    return;
                }
            }
        }

    }
}

void SendWidget::processSendCoinsReturn(const WalletModel::SendCoinsReturn& sendCoinsReturn, const QString& msgArg, bool fPrepare)
{
    bool fAskForUnlock = false;

    QPair<QString, CClientUIInterface::MessageBoxFlags> msgParams;
    // Default to a warning message, override if error message is needed
    msgParams.second = CClientUIInterface::MSG_WARNING;

    // This comment is specific to SendCoinsDialog usage of WalletModel::SendCoinsReturn.
    // WalletModel::TransactionCommitFailed is used only in WalletModel::sendCoins()
    // all others are used only in WalletModel::prepareTransaction()
    switch (sendCoinsReturn.status) {
        case WalletModel::InvalidAddress:
            msgParams.first = tr("The recipient address is not valid, please recheck.");
            break;
        case WalletModel::InvalidAmount:
            msgParams.first = tr("The amount to pay must be larger than 0.");
            break;
        case WalletModel::AmountExceedsBalance:
            msgParams.first = tr("The amount exceeds your balance.");
            break;
        case WalletModel::AmountWithFeeExceedsBalance:
            msgParams.first = tr("The total exceeds your balance when the %1 transaction fee is included.").arg(msgArg);
            break;
        case WalletModel::DuplicateAddress:
            msgParams.first = tr("Duplicate address found, can only send to each address once per send operation.");
            break;
        case WalletModel::TransactionCreationFailed:
            msgParams.first = tr("Transaction creation failed!");
            msgParams.second = CClientUIInterface::MSG_ERROR;
            break;
        case WalletModel::TransactionCommitFailed:
            msgParams.first = tr("The transaction was rejected! This might happen if some of the coins in your wallet were already spent, such as if you used a copy of wallet.dat and coins were spent in the copy but not marked as spent here.");
            msgParams.second = CClientUIInterface::MSG_ERROR;
            break;
        case WalletModel::AnonymizeOnlyUnlocked:
            // Unlock is only need when the coins are send
            if(!fPrepare)
                fAskForUnlock = true;
            else
                msgParams.first = tr("Error: The wallet was unlocked only to anonymize coins.");
            break;

        case WalletModel::InsaneFee:
            msgParams.first = tr("A fee %1 times higher than %2 per kB is considered an insanely high fee.").arg(10000).arg(BitcoinUnits::formatWithUnit(walletModel->getOptionsModel()->getDisplayUnit(), ::minRelayTxFee.GetFeePerK()));
            break;
            // included to prevent a compiler warning.
        case WalletModel::OK:
        default:
            return;
    }

    // Unlock wallet if it wasn't fully unlocked already
    if(fAskForUnlock) {
        walletModel->requestUnlock(AskPassphraseDialog::Context::Unlock_Full, false);
        if(walletModel->getEncryptionStatus () != WalletModel::Unlocked) {
            msgParams.first = tr("Error: The wallet was unlocked only to anonymize coins. Unlock canceled.");
        }
        else {
            // Wallet unlocked
            return;
        }
    }

    emit message(tr("Send Coins"), msgParams.first, msgParams.second);
}


void SendWidget::onChangeAddressClicked(){
    showHideOp(true);
    SendChangeAddressDialog* dialog = new SendChangeAddressDialog(window);
    if(!boost::get<CNoDestination>(&CoinControlDialog::coinControl->destChange)){
        dialog->setAddress(QString::fromStdString(CBitcoinAddress(CoinControlDialog::coinControl->destChange).ToString()));
    }
    if(openDialogWithOpaqueBackgroundY(dialog, window, 3, 5)) {
        if(dialog->selected) {
            QString ret;
            if (dialog->getAddress(walletModel, &ret)) {
                CoinControlDialog::coinControl->destChange = CBitcoinAddress(ret.toStdString()).Get();
                ui->btnChangeAddress->setActive(true);
            }else{
                inform(tr("Invalid change address"));
                ui->btnChangeAddress->setActive(false);
            }
        }
    }
    dialog->deleteLater();
}

void SendWidget::onOpenUriClicked(){
    showHideOp(true);
    OpenURIDialog *dlg = new OpenURIDialog(window);
    if (openDialogWithOpaqueBackgroundY(dlg, window, 3, 5)) {
        emit receivedURI(dlg->getURI());
    }
    dlg->deleteLater();
}

void SendWidget::onChangeCustomFeeClicked(){
    showHideOp(true);
    SendCustomFeeDialog* dialog = new SendCustomFeeDialog(window);
    openDialogWithOpaqueBackgroundY(dialog, window, 3, 5);
    dialog->deleteLater();
}

void SendWidget::onCoinControlClicked()
{
    if(isPIV){
        if (walletModel->getBalance() > 0) {
            if (!coinControlDialog) {
                coinControlDialog = new CoinControlDialog();
                coinControlDialog->setModel(walletModel);
            }
            coinControlDialog->exec();
            ui->btnCoinControl->setActive(CoinControlDialog::coinControl->HasSelected());
        } else {
            inform(tr("You don't have any PIV to select."));
        }
    }else{
        if (walletModel->getZerocoinBalance() > 0) {
            ZVitControlDialog* zVitControl = new ZVitControlDialog(this);
            zVitControl->setModel(walletModel);
            zVitControl->exec();
            ui->btnCoinControl->setActive(!ZVitControlDialog::setSelectedMints.empty());
            zVitControl->deleteLater();
        } else {
            inform(tr("You don't have any zPIV in your balance to select."));
        }
    }
}

void SendWidget::onValueChanged() {
    refreshAmounts();
}

void SendWidget::onPIVSelected(bool _isPIV){
    isPIV = _isPIV;
    coinIcon->setProperty("cssClass", _isPIV ? "coin-icon-piv" : "coin-icon-zpiv");
    refreshView();
    updateStyle(coinIcon);
}

void SendWidget::onContactsClicked(SendMultiRow* entry){
    focusedEntry = entry;
    if(menu && menu->isVisible()){
        menu->hide();
    }

    int contactsSize = walletModel->getAddressTableModel()->sizeSend();
    if(contactsSize == 0) {
        inform(tr("No contacts available, you can go to the contacts screen and add some there!"));
        return;
    }

    int height = (contactsSize <= 2) ? entry->getEditHeight() * ( 2 * (contactsSize + 1 )) : entry->getEditHeight() * 4;
    int width = entry->getEditWidth();

    if(!menuContacts){
        menuContacts = new ContactsDropdown(
                    width,
                    height,
                    this
        );
        menuContacts->setWalletModel(walletModel, AddressTableModel::Send);
        connect(menuContacts, &ContactsDropdown::contactSelected, [this](QString address, QString label){
            if(focusedEntry){
                focusedEntry->setLabel(label);
                focusedEntry->setAddress(address);
            }
        });

    }

    if(menuContacts->isVisible()){
        menuContacts->hide();
        return;
    }

    menuContacts->resizeList(width, height);
    menuContacts->setStyleSheet(this->styleSheet());
    menuContacts->adjustSize();

    QPoint pos;
    if (entries.size() > 1){
        pos = entry->pos();
        pos.setY((pos.y() + (focusedEntry->getEditHeight() - 12) * 4));
    } else {
        pos = focusedEntry->getEditLineRect().bottomLeft();
        pos.setY((pos.y() + (focusedEntry->getEditHeight() - 12) * 3));
    }
    pos.setX(pos.x() + 20);
    menuContacts->move(pos);
    menuContacts->show();
}

void SendWidget::onMenuClicked(SendMultiRow* entry){
    focusedEntry = entry;
    if(menuContacts && menuContacts->isVisible()){
        menuContacts->hide();
    }
    QPoint pos = entry->pos();
    pos.setX(pos.x() + (entry->width() - entry->getMenuBtnWidth()));
    pos.setY(pos.y() + entry->height() + (entry->getMenuBtnWidth()));

    if(!this->menu){
        this->menu = new TooltipMenu(window, this);
        this->menu->setCopyBtnVisible(false);
        this->menu->setEditBtnText(tr("Save contact"));
        this->menu->setMinimumSize(this->menu->width() + 30,this->menu->height());
        connect(this->menu, &TooltipMenu::message, this, &AddressesWidget::message);
        connect(this->menu, SIGNAL(onEditClicked()), this, SLOT(onContactMultiClicked()));
        connect(this->menu, SIGNAL(onDeleteClicked()), this, SLOT(onDeleteClicked()));
    }else {
        this->menu->hide();
    }
    menu->move(pos);
    menu->show();
}

void SendWidget::onContactMultiClicked(){
    if(focusedEntry) {
        QString address = focusedEntry->getAddress();
        if (address.isEmpty()) {
            inform(tr("Address field is empty"));
            return;
        }
        if (!walletModel->validateAddress(address)) {
            inform(tr("Invalid address"));
            return;
        }
        CBitcoinAddress pivAdd = CBitcoinAddress(address.toStdString());
        if (walletModel->isMine(pivAdd)) {
            inform(tr("Cannot store your own address as contact"));
            return;
        }

        showHideOp(true);
        AddNewContactDialog *dialog = new AddNewContactDialog(window);
        QString label = walletModel->getAddressTableModel()->labelForAddress(address);
        if (!label.isNull()){
            dialog->setTexts(tr("Update Contact"), "Edit label for the selected address:\n%1");
            dialog->setData(address, label);
        } else {
            dialog->setTexts(tr("Create New Contact"), "Save label for the selected address:\n%1");
            dialog->setData(address, "");
        }
        openDialogWithOpaqueBackgroundY(dialog, window, 3, 5);
        if (dialog->res) {
            if (label == dialog->getLabel()) {
                return;
            }
            if (walletModel->updateAddressBookLabels(pivAdd.Get(), dialog->getLabel().toStdString(), "send")) {
                inform(tr("New Contact Stored"));
            } else {
                inform(tr("Error Storing Contact"));
            }
        }
        dialog->deleteLater();
    }

}

void SendWidget::onDeleteClicked(){
    if (focusedEntry) {
        focusedEntry->hide();
        focusedEntry->deleteLater();
        int entryNumber = focusedEntry->getNumber();

        // Refresh amount total + rest of rows numbers.
        QMutableListIterator<SendMultiRow*> it(entries);
        while (it.hasNext()) {
            SendMultiRow* entry = it.next();
            if (focusedEntry == entry){
                it.remove();
            } else if (focusedEntry && entry->getNumber() > entryNumber){
                entry->setNumber(entry->getNumber() - 1);
            }
        }

        if (entries.size() == 1) {
            SendMultiRow* sendMultiRow = QMutableListIterator<SendMultiRow*>(entries).next();
            sendMultiRow->setNumber(entries.length());
            sendMultiRow->showLabels();
        }

        focusedEntry = nullptr;
    }
}

void SendWidget::resizeMenu(){
    if(menuContacts && menuContacts->isVisible() && focusedEntry){
        int width = focusedEntry->getEditWidth();
        menuContacts->resizeList(width, menuContacts->height());
        menuContacts->resize(width, menuContacts->height());
        QPoint pos = focusedEntry->getEditLineRect().bottomLeft();
        // TODO: Change this position..
        pos.setX(pos.x() + 20);
        pos.setY(pos.y() + ((focusedEntry->getEditHeight() - 12)  * 3));
        menuContacts->move(pos);
    }
}

SendWidget::~SendWidget()
{
    delete ui;
}
