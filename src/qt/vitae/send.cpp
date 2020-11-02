// Copyright (c) 2019-2020 The PIVX developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "qt/vitae/send.h"
#include "qt/vitae/forms/ui_send.h"
#include "qt/vitae/addnewcontactdialog.h"
#include "qt/vitae/qtutils.h"
#include "qt/vitae/sendchangeaddressdialog.h"
#include "qt/vitae/optionbutton.h"
#include "qt/vitae/sendconfirmdialog.h"
#include "qt/vitae/myaddressrow.h"
#include "qt/vitae/guitransactionsutils.h"
#include "clientmodel.h"
#include "optionsmodel.h"
#include "addresstablemodel.h"
#include "coincontrol.h"
#include "script/standard.h"
#include "zvit/deterministicmint.h"
#include "openuridialog.h"
#include "qt/zVitcontroldialog.h"

SendWidget::SendWidget(VITAEGUI* parent) :
    PWidget(parent),
    ui(new Ui::send),
    coinIcon(new QPushButton()),
    btnContacts(new QPushButton())
{
    ui->setupUi(this);

    this->setStyleSheet(parent->styleSheet());

    /* Containers */
    setCssProperty(ui->left, "container");
    ui->left->setContentsMargins(0,20,0,20);
    setCssProperty(ui->right, "container-right");
    ui->right->setContentsMargins(20,10,20,20);

    /* Light Font */
    QFont fontLight;
    fontLight.setWeight(QFont::Light);

    /* Title */
    ui->labelTitle->setText(tr("Send"));
    setCssProperty(ui->labelTitle, "text-title-screen");
    ui->labelTitle->setFont(fontLight);

    /* Button Group */
    ui->pushLeft->setText("VIT");
    setCssProperty(ui->pushLeft, "btn-check-left");
    ui->pushLeft->setChecked(true);
    ui->pushRight->setText("zVIT");
    setCssProperty(ui->pushRight, "btn-check-right");

    /* Subtitle */
    ui->labelSubtitle1->setText(tr("You can transfer public coins (VIT) or private coins (zVIT)"));
    setCssProperty(ui->labelSubtitle1, "text-subtitle");

    ui->labelSubtitle2->setText(tr("Select coin type to spend"));
    setCssProperty(ui->labelSubtitle2, "text-subtitle");

    /* Address */
    ui->labelSubtitleAddress->setText(tr("VITAE address or contact label"));
    setCssProperty(ui->labelSubtitleAddress, "text-title");


    /* Amount */
    ui->labelSubtitleAmount->setText(tr("Amount"));
    setCssProperty(ui->labelSubtitleAmount, "text-title");

    /* Buttons */
    ui->pushButtonFee->setText(tr("Customize fee"));
    setCssBtnSecondary(ui->pushButtonFee);

    ui->pushButtonClear->setText(tr("Clear all"));
    setCssProperty(ui->pushButtonClear, "btn-secundary-clear");

    ui->pushButtonAddRecipient->setText(tr("Add recipient"));
    setCssProperty(ui->pushButtonAddRecipient, "btn-secundary-add");

    setCssBtnPrimary(ui->pushButtonSave);
    ui->pushButtonReset->setText(tr("Reset to default"));
    setCssBtnSecondary(ui->pushButtonReset);

    // Coin control
    ui->btnCoinControl->setTitleClassAndText("btn-title-grey", "Coin Control");
    ui->btnCoinControl->setSubTitleClassAndText("text-subtitle", "Select the source of the coins.");

    // Change address option
    ui->btnChangeAddress->setTitleClassAndText("btn-title-grey", "Change Address");
    ui->btnChangeAddress->setSubTitleClassAndText("text-subtitle", "Customize the change address.");

    // Uri
    ui->btnUri->setTitleClassAndText("btn-title-grey", "Open URI");
    ui->btnUri->setSubTitleClassAndText("text-subtitle", "Parse a payment request.");

    connect(ui->pushButtonFee, &QPushButton::clicked, this, &SendWidget::onChangeCustomFeeClicked);
    connect(ui->btnCoinControl, &OptionButton::clicked, this, &SendWidget::onCoinControlClicked);
    connect(ui->btnChangeAddress, &OptionButton::clicked, this, &SendWidget::onChangeAddressClicked);
    connect(ui->btnUri, &OptionButton::clicked, this, &SendWidget::onOpenUriClicked);
    connect(ui->pushButtonReset, &QPushButton::clicked, [this](){ onResetCustomOptions(true); });

    setCssProperty(ui->coinWidget, "container-coin-type");
    setCssProperty(ui->labelLine, "container-divider");


    // Total Send
    ui->labelTitleTotalSend->setText(tr("Total to send"));
    setCssProperty(ui->labelTitleTotalSend, "text-title");

    ui->labelAmountSend->setText("0.00 VIT");
    setCssProperty(ui->labelAmountSend, "text-body1");

    // Total Remaining
    setCssProperty(ui->labelTitleTotalRemaining, "text-title");

    setCssProperty(ui->labelAmountRemaining, "text-body1");

    // Icon Send
    ui->stackedWidget->addWidget(coinIcon);
    coinIcon->show();
    coinIcon->raise();

    setCssProperty(coinIcon, "coin-icon-vit");

    QSize BUTTON_SIZE = QSize(24, 24);
    coinIcon->setMinimumSize(BUTTON_SIZE);
    coinIcon->setMaximumSize(BUTTON_SIZE);

    int posX = 0;
    int posY = 20;
    coinIcon->move(posX, posY);

    // Entry
    addEntry();

    // Init custom fee false (updated in loadWalletModel)
    setCustomFeeSelected(false);

    // Connect
    connect(ui->pushLeft, &QPushButton::clicked, [this](){onVITSelected(true);});
    connect(ui->pushRight,  &QPushButton::clicked, [this](){onVITSelected(false);});
    connect(ui->pushButtonSave, &QPushButton::clicked, this, &SendWidget::onSendClicked);
    connect(ui->pushButtonAddRecipient, &QPushButton::clicked, this, &SendWidget::onAddEntryClicked);
    connect(ui->pushButtonClear, &QPushButton::clicked, [this](){clearAll(true);});
}

void SendWidget::refreshView()
{
    QString btnText;
    if (ui->pushLeft->isChecked()) {
        btnText = tr("Send VIT");
        ui->pushButtonAddRecipient->setVisible(true);
    } else {
        btnText = tr("Send zVIT");
        ui->pushButtonAddRecipient->setVisible(false);
    }
    ui->pushButtonSave->setText(btnText);

    refreshAmounts();
}

void SendWidget::refreshAmounts()
{
    CAmount total = 0;
    QMutableListIterator<SendMultiRow*> it(entries);
    while (it.hasNext()) {
        SendMultiRow* entry = it.next();
        CAmount amount = entry->getAmountValue();
        if (amount > 0)
            total += amount;
    }

    bool isZvit = ui->pushRight->isChecked();
    nDisplayUnit = walletModel->getOptionsModel()->getDisplayUnit();

    ui->labelAmountSend->setText(GUIUtil::formatBalance(total, nDisplayUnit, isZvit));

    CAmount totalAmount = 0;
    if (CoinControlDialog::coinControl->HasSelected()) {
        // Set remaining balance to the sum of the coinControl selected inputs
        totalAmount = walletModel->getBalance(CoinControlDialog::coinControl) - total;
        ui->labelTitleTotalRemaining->setText(tr("Total remaining from the selected UTXO"));
    } else {
        // Wallet's balance
        totalAmount = (isZvit ? walletModel->getZerocoinBalance() : walletModel->getBalance()) - total;
        ui->labelTitleTotalRemaining->setText(tr("Total remaining"));
    }
    ui->labelAmountRemaining->setText(
            GUIUtil::formatBalance(
                    totalAmount,
                    nDisplayUnit,
                    isZvit
                    )
    );
}

void SendWidget::loadClientModel()
{
    if (clientModel) {
        connect(clientModel, &ClientModel::numBlocksChanged, [this](){
            if (customFeeDialog) customFeeDialog->updateFee();
        });
    }
}

void SendWidget::loadWalletModel()
{
    if (walletModel && walletModel->getOptionsModel()) {
        // display unit
        nDisplayUnit = walletModel->getOptionsModel()->getDisplayUnit();

        for (SendMultiRow *entry : entries) {
            if (entry) {
                entry->setWalletModel(walletModel);
            }
        }

        // Restore custom fee from wallet Settings
        CAmount nCustomFee;
        if (walletModel->getWalletCustomFee(nCustomFee)) {
            setCustomFeeSelected(true, nCustomFee);
        }

        // Refresh view
        refreshView();

        // TODO: This only happen when the coin control features are modified in other screen, check before do this if the wallet has another screen modifying it.
        // Coin Control
        //connect(walletModel->getOptionsModel(), &OptionsModel::coinControlFeaturesChanged, [this](){});
        //ui->frameCoinControl->setVisible(model->getOptionsModel()->getCoinControlFeatures());
        //coinControlUpdateLabels();
    }
}

void SendWidget::clearAll(bool fClearSettings)
{
    onResetCustomOptions(false);
    if (fClearSettings) onResetSettings();
    clearEntries();
    refreshAmounts();
}

void SendWidget::onResetSettings()
{
    if (customFeeDialog) customFeeDialog->clear();
    setCustomFeeSelected(false);
    if (walletModel) walletModel->setWalletCustomFee(false, DEFAULT_TRANSACTION_FEE);
}

void SendWidget::onResetCustomOptions(bool fRefreshAmounts)
{
    CoinControlDialog::coinControl->SetNull();
    ui->btnChangeAddress->setActive(false);
    ui->btnCoinControl->setActive(false);
    if (fRefreshAmounts) {
        refreshAmounts();
    }
}

void SendWidget::clearEntries()
{
    int num = entries.length();
    for (int i = 0; i < num; ++i) {
        ui->scrollAreaWidgetContents->layout()->takeAt(0)->widget()->deleteLater();
    }
    entries.clear();

    addEntry();
}

void SendWidget::addEntry()
{
    if (entries.isEmpty()) {
        createEntry();
    } else {
        if (entries.length() == 1) {
            SendMultiRow *entry = entries.at(0);
            entry->hideLabels();
            entry->setNumber(1);
        } else if (entries.length() == MAX_SEND_POPUP_ENTRIES) {
            inform(tr("Maximum amount of outputs reached"));
            return;
        }

        SendMultiRow *sendMultiRow = createEntry();
        sendMultiRow->setNumber(entries.length());
        sendMultiRow->hideLabels();
    }
    setFocusOnLastEntry();
}

SendMultiRow* SendWidget::createEntry()
{
    SendMultiRow *sendMultiRow = new SendMultiRow(this);
    if (this->walletModel) sendMultiRow->setWalletModel(this->walletModel);
    entries.append(sendMultiRow);
    ui->scrollAreaWidgetContents->layout()->addWidget(sendMultiRow);
    connect(sendMultiRow, &SendMultiRow::onContactsClicked, this, &SendWidget::onContactsClicked);
    connect(sendMultiRow, &SendMultiRow::onMenuClicked, this, &SendWidget::onMenuClicked);
    connect(sendMultiRow, &SendMultiRow::onValueChanged, this, &SendWidget::onValueChanged);
    return sendMultiRow;
}

void SendWidget::onAddEntryClicked()
{
    // Check prev valid entries before add a new one.
    for (SendMultiRow* entry : entries) {
        if (!entry || !entry->validate()) {
            inform(tr("Invalid entry, previous entries must be valid before add a new one"));
            return;
        }
    }
    addEntry();
}

void SendWidget::resizeEvent(QResizeEvent *event)
{
    resizeMenu();
    QWidget::resizeEvent(event);
}

void SendWidget::showEvent(QShowEvent *event)
{
    // Set focus on last recipient address when Send-window is displayed
    setFocusOnLastEntry();
}

void SendWidget::setFocusOnLastEntry()
{
    if (!entries.isEmpty()) entries.last()->setFocus();
}

void SendWidget::onSendClicked()
{
    if (!walletModel || !walletModel->getOptionsModel())
        return;

    QList<SendCoinsRecipient> recipients;

    for (SendMultiRow* entry : entries) {
        // TODO: Check UTXO splitter here..
        // Validate send..
        if (entry && entry->validate()) {
            recipients.append(entry->getValue());
        } else {
            inform(tr("Invalid entry"));
            return;
        }
    }

    if (recipients.isEmpty()) {
        inform(tr("No set recipients"));
        return;
    }

    bool sendVit = ui->pushLeft->isChecked();

    bool sendPiv = ui->pushLeft->isChecked();

    WalletModel::UnlockContext ctx(walletModel->requestUnlock());
    if (!ctx.isValid()) {
        // Unlock wallet was cancelled
        inform(tr("Cannot send, wallet locked"));
        return;
    }

    if ((sendVit) ? send(recipients) : sendZvit(recipients)) {
        updateEntryLabels(recipients);
    }
    setFocusOnLastEntry();
}

bool SendWidget::send(QList<SendCoinsRecipient> recipients)
{
    // prepare transaction for getting txFee earlier
    WalletModelTransaction currentTransaction(recipients);
    WalletModel::SendCoinsReturn prepareStatus;

    prepareStatus = walletModel->prepareTransaction(currentTransaction, CoinControlDialog::coinControl);

    // process prepareStatus and on error generate message shown to user
    GuiTransactionsUtils::ProcessSendCoinsReturnAndInform(
            this,
            prepareStatus,
            walletModel,
            BitcoinUnits::formatWithUnit(walletModel->getOptionsModel()->getDisplayUnit(),
                                         currentTransaction.getTransactionFee()),
            true
    );

    if (prepareStatus.status != WalletModel::OK) {
        inform(tr("Cannot create transaction."));
        return false;
    }

    showHideOp(true);
    QString warningStr = QString();
    if (currentTransaction.getTransaction()->fStakeDelegationVoided)
        warningStr = tr("WARNING:\nTransaction spends a cold-stake delegation, voiding it.\n"
                     "These coins will no longer be cold-staked.");
    TxDetailDialog* dialog = new TxDetailDialog(window, true, warningStr);
    dialog->setDisplayUnit(walletModel->getOptionsModel()->getDisplayUnit());
    dialog->setData(walletModel, currentTransaction);
    dialog->adjustSize();
    openDialogWithOpaqueBackgroundY(dialog, window, 3, 5);

    if (dialog->isConfirm()) {
        // now send the prepared transaction
        WalletModel::SendCoinsReturn sendStatus = dialog->getStatus();
        // process sendStatus and on error generate message shown to user
        GuiTransactionsUtils::ProcessSendCoinsReturnAndInform(
                this,
                sendStatus,
                walletModel
        );

        if (sendStatus.status == WalletModel::OK) {
            clearAll(false);
            inform(tr("Transaction sent"));
            dialog->deleteLater();
            return true;
        }
    }

    dialog->deleteLater();
    return false;
}

bool SendWidget::sendZvit(QList<SendCoinsRecipient> recipients)
{
    if (!walletModel || !walletModel->getOptionsModel())
        return false;

    if (sporkManager.IsSporkActive(SPORK_20_ZEROCOIN_MAINTENANCE_MODE)) {
        Q_EMIT message(tr("Spend Zerocoin"), tr("zVIT is currently undergoing maintenance."), CClientUIInterface::MSG_ERROR);
        return false;
    }

    std::list<std::pair<CBitcoinAddress*, CAmount>> outputs;
    CAmount total = 0;
    for (SendCoinsRecipient rec : recipients) {
        total += rec.amount;
        outputs.push_back(std::pair<CBitcoinAddress*, CAmount>(new CBitcoinAddress(rec.address.toStdString()),rec.amount));
    }

    // use mints from zVIT selector if applicable
    std::vector<CMintMeta> vMintsToFetch;
    std::vector<CZerocoinMint> vMintsSelected;
    if (!ZVitControlDialog::setSelectedMints.empty()) {
        vMintsToFetch = ZVitControlDialog::GetSelectedMints();

        for (auto& meta : vMintsToFetch) {
            CZerocoinMint mint;
            if (!walletModel->getMint(meta.hashSerial, mint)) {
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
    Q_EMIT message(
            tr("Spend Zerocoin"),
            sendBody,
            CClientUIInterface::MSG_INFORMATION | CClientUIInterface::BTN_MASK | CClientUIInterface::MODAL,
            &ret);

    if (!ret) return false;

    CZerocoinSpendReceipt receipt;

    std::string changeAddress = "";
    if (!boost::get<CNoDestination>(&CoinControlDialog::coinControl->destChange)) {
        changeAddress = CBitcoinAddress(CoinControlDialog::coinControl->destChange).ToString();
    } else {
        changeAddress = walletModel->getAddressTableModel()->getAddressToShow().toStdString();
    }

    if (walletModel->sendZvit(
            vMintsSelected,
            receipt,
            outputs,
            changeAddress
    )
            ) {
        inform(tr("zVIT transaction sent!"));
        ZVitControlDialog::setSelectedMints.clear();
        clearAll(false);
        return true;
    } else {
        QString body;
        if (receipt.GetStatus() == ZVIT_SPEND_V1_SEC_LEVEL) {
            body = tr("Version 1 zVIT require a security level of 100 to successfully spend.");
        } else {
            int nNeededSpends = receipt.GetNeededSpends(); // Number of spends we would need for this transaction
            const int nMaxSpends = Params().GetConsensus().ZC_MaxSpendsPerTx; // Maximum possible spends for one zVIT transaction
            if (nNeededSpends > nMaxSpends) {
                body = tr("Too much inputs (") + QString::number(nNeededSpends, 10) +
                       tr(") needed.\nMaximum allowed: ") + QString::number(nMaxSpends, 10);
                body += tr(
                        "\nEither mint higher denominations (so fewer inputs are needed) or reduce the amount to spend.");
            } else {
                body = QString::fromStdString(receipt.GetStatusMessage());
            }
        }
        Q_EMIT message("zVIT transaction failed", body, CClientUIInterface::MSG_ERROR);
        return false;
    }
}

QString SendWidget::recipientsToString(QList<SendCoinsRecipient> recipients)
{
    QString s = "";
    for (SendCoinsRecipient rec : recipients) {
        s += rec.address + " -> " + BitcoinUnits::formatWithUnit(walletModel->getOptionsModel()->getDisplayUnit(), rec.amount, false, BitcoinUnits::separatorAlways) + "\n";
    }
    return s;
}

void SendWidget::updateEntryLabels(QList<SendCoinsRecipient> recipients)
{
    for (SendCoinsRecipient rec : recipients) {
        QString label = rec.label;
        if (!label.isNull()) {
            QString labelOld = walletModel->getAddressTableModel()->labelForAddress(rec.address);
            if (label.compare(labelOld) != 0) {
                CTxDestination dest = CBitcoinAddress(rec.address.toStdString()).Get();
                if (!walletModel->updateAddressBookLabels(dest, label.toStdString(),
                                                          this->walletModel->isMine(dest) ?
                                                                  AddressBook::AddressBookPurpose::RECEIVE :
                                                                  AddressBook::AddressBookPurpose::SEND)) {
                    // Label update failed
                    Q_EMIT message("", tr("Address label update failed for address: %1").arg(rec.address), CClientUIInterface::MSG_ERROR);
                    return;
                }
            }
        }

    }
}


void SendWidget::onChangeAddressClicked()
{
    showHideOp(true);
    SendChangeAddressDialog* dialog = new SendChangeAddressDialog(window);
    if (!boost::get<CNoDestination>(&CoinControlDialog::coinControl->destChange)) {
        dialog->setAddress(QString::fromStdString(CBitcoinAddress(CoinControlDialog::coinControl->destChange).ToString()));
    }
    if (openDialogWithOpaqueBackgroundY(dialog, window, 3, 5)) {
        if (dialog->selected) {
            QString ret;
            if (dialog->getAddress(walletModel, &ret)) {
                CoinControlDialog::coinControl->destChange = CBitcoinAddress(ret.toStdString()).Get();
                ui->btnChangeAddress->setActive(true);
            } else {
                inform(tr("Invalid change address"));
                ui->btnChangeAddress->setActive(false);
            }
        }
    }
    dialog->deleteLater();
}

void SendWidget::onOpenUriClicked()
{
    showHideOp(true);
    OpenURIDialog *dlg = new OpenURIDialog(window);
    if (openDialogWithOpaqueBackgroundY(dlg, window, 3, 5)) {

        SendCoinsRecipient rcp;
        if (!GUIUtil::parseBitcoinURI(dlg->getURI(), &rcp)) {
            inform(tr("Invalid URI"));
            return;
        }
        if (!walletModel->validateAddress(rcp.address)) {
            inform(tr("Invalid address in URI"));
            return;
        }

        int listSize = entries.size();
        if (listSize == 1) {
            SendMultiRow *entry = entries[0];
            entry->setAddressAndLabelOrDescription(rcp.address, rcp.message);
            entry->setAmount(BitcoinUnits::format(nDisplayUnit, rcp.amount, false));
        } else {
            // Use the last one if it's invalid or add a new one
            SendMultiRow *entry = entries[listSize - 1];
            if (!entry->validate()) {
                addEntry();
                entry = entries[listSize];
            }
            entry->setAddressAndLabelOrDescription(rcp.address, rcp.message);
            entry->setAmount(BitcoinUnits::format(nDisplayUnit, rcp.amount, false));
        }
        Q_EMIT receivedURI(dlg->getURI());
    }
    dlg->deleteLater();
}

void SendWidget::onChangeCustomFeeClicked()
{
    showHideOp(true);
    if (!customFeeDialog) {
        customFeeDialog = new SendCustomFeeDialog(window);
        customFeeDialog->setWalletModel(walletModel);
    }
    if (openDialogWithOpaqueBackgroundY(customFeeDialog, window, 3, 5)) {
        const CAmount& nFeePerKb = customFeeDialog->getFeeRate().GetFeePerK();
        setCustomFeeSelected(customFeeDialog->isCustomFeeChecked(), nFeePerKb);
    }
}

void SendWidget::onCoinControlClicked()
{
    if (isVIT) {
        if (walletModel->getBalance() > 0) {
            if (!coinControlDialog) {
                coinControlDialog = new CoinControlDialog();
                coinControlDialog->setModel(walletModel);
            } else {
                coinControlDialog->refreshDialog();
            }
            coinControlDialog->exec();
            ui->btnCoinControl->setActive(CoinControlDialog::coinControl->HasSelected());
            refreshAmounts();
        } else {
            inform(tr("You don't have any VIT to select."));
        }
    } else {
        if (walletModel->getZerocoinBalance() > 0) {
            ZVitControlDialog* zVitControl = new ZVitControlDialog(this);
            zVitControl->setModel(walletModel);
            zVitControl->exec();
            ui->btnCoinControl->setActive(!ZVitControlDialog::setSelectedMints.empty());
            zVitControl->deleteLater();
        } else {
            inform(tr("You don't have any zVIT in your balance to select."));
        }
    }
}

void SendWidget::onValueChanged()
{
    refreshAmounts();
}

void SendWidget::onVITSelected(bool _isVIT)
{
    isVIT = _isVIT;
    setCssProperty(coinIcon, _isVIT ? "coin-icon-vit" : "coin-icon-zvit");
    refreshView();
    updateStyle(coinIcon);
}

void SendWidget::onContactsClicked(SendMultiRow* entry)
{
    focusedEntry = entry;
    if (menu && menu->isVisible()) {
        menu->hide();
    }

    int contactsSize = walletModel->getAddressTableModel()->sizeSend();
    if (contactsSize == 0) {
        inform(tr("No contacts available, you can go to the contacts screen and add some there!"));
        return;
    }

    int height = (contactsSize <= 2) ? entry->getEditHeight() * ( 2 * (contactsSize + 1 )) : entry->getEditHeight() * 4;
    int width = entry->getEditWidth();

    if (!menuContacts) {
        menuContacts = new ContactsDropdown(
                    width,
                    height,
                    this
        );
        menuContacts->setWalletModel(walletModel, AddressTableModel::Send);
        connect(menuContacts, &ContactsDropdown::contactSelected, [this](QString address, QString label) {
            if (focusedEntry) {
                focusedEntry->setLabel(label);
                focusedEntry->setAddress(address);
            }
        });

    }

    if (menuContacts->isVisible()) {
        menuContacts->hide();
        return;
    }

    menuContacts->resizeList(width, height);
    menuContacts->setStyleSheet(this->styleSheet());
    menuContacts->adjustSize();

    QPoint pos;
    if (entries.size() > 1) {
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

void SendWidget::onMenuClicked(SendMultiRow* entry)
{
    focusedEntry = entry;
    if (menuContacts && menuContacts->isVisible()) {
        menuContacts->hide();
    }
    QPoint pos = entry->pos();
    pos.setX(pos.x() + (entry->width() - entry->getMenuBtnWidth()));
    pos.setY(pos.y() + entry->height() + (entry->getMenuBtnWidth()));

    if (!this->menu) {
        this->menu = new TooltipMenu(window, this);
        this->menu->setCopyBtnVisible(false);
        this->menu->setEditBtnText(tr("Save contact"));
        this->menu->setMinimumSize(this->menu->width() + 30,this->menu->height());
        connect(this->menu, &TooltipMenu::message, this, &AddressesWidget::message);
        connect(this->menu, &TooltipMenu::onEditClicked, this, &SendWidget::onContactMultiClicked);
        connect(this->menu, &TooltipMenu::onDeleteClicked, this, &SendWidget::onDeleteClicked);
    } else {
        this->menu->hide();
    }
    menu->move(pos);
    menu->show();
}

void SendWidget::onContactMultiClicked()
{
    if (focusedEntry) {
        QString address = focusedEntry->getAddress();
        if (address.isEmpty()) {
            inform(tr("Address field is empty"));
            return;
        }
        if (!walletModel->validateAddress(address)) {
            inform(tr("Invalid address"));
            return;
        }
        CBitcoinAddress vitAdd = CBitcoinAddress(address.toStdString());
        if (walletModel->isMine(vitAdd)) {
            inform(tr("Cannot store your own address as contact"));
            return;
        }

        showHideOp(true);
        AddNewContactDialog *dialog = new AddNewContactDialog(window);
        QString label = walletModel->getAddressTableModel()->labelForAddress(address);
        if (!label.isNull()) {
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
            if (walletModel->updateAddressBookLabels(vitAdd.Get(), dialog->getLabel().toStdString(),
                    AddressBook::AddressBookPurpose::SEND)) {
                inform(tr("New Contact Stored"));
            } else {
                inform(tr("Error Storing Contact"));
            }
        }
        dialog->deleteLater();
    }

}

void SendWidget::onDeleteClicked()
{
    if (focusedEntry) {
        focusedEntry->hide();
        focusedEntry->deleteLater();
        int entryNumber = focusedEntry->getNumber();

        // remove selected entry and update row number for the others
        QMutableListIterator<SendMultiRow*> it(entries);
        while (it.hasNext()) {
            SendMultiRow* entry = it.next();
            if (focusedEntry == entry) {
                it.remove();
            } else if (focusedEntry && entry->getNumber() > entryNumber) {
                entry->setNumber(entry->getNumber() - 1);
            }
        }

        if (entries.size() == 1) {
            SendMultiRow* sendMultiRow = QMutableListIterator<SendMultiRow*>(entries).next();
            sendMultiRow->setNumber(entries.length());
            sendMultiRow->showLabels();
        }

        focusedEntry = nullptr;

        // Update total amounts
        refreshAmounts();
        setFocusOnLastEntry();
    }
}

void SendWidget::resizeMenu()
{
    if (menuContacts && menuContacts->isVisible() && focusedEntry) {
        int width = focusedEntry->getEditWidth();
        menuContacts->resizeList(width, menuContacts->height());
        menuContacts->resize(width, menuContacts->height());
        QPoint pos = focusedEntry->getEditLineRect().bottomLeft();
        pos.setX(pos.x() + 20);
        pos.setY(pos.y() + ((focusedEntry->getEditHeight() - 12)  * 3));
        menuContacts->move(pos);
    }
}

void SendWidget::setCustomFeeSelected(bool isSelected, const CAmount& customFee)
{
    isCustomFeeSelected = isSelected;
    ui->pushButtonFee->setText(isCustomFeeSelected ?
                    tr("Custom Fee %1").arg(BitcoinUnits::formatWithUnit(nDisplayUnit, customFee) + "/kB") :
                    tr("Customize Fee"));
    if (walletModel)
        walletModel->setWalletDefaultFee(customFee);
}

void SendWidget::changeTheme(bool isLightTheme, QString& theme)
{
    if (coinControlDialog) coinControlDialog->setStyleSheet(theme);
}

SendWidget::~SendWidget()
{
    delete ui;
}
