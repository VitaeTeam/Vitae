#include "fundamentalnodelist.h"
#include "ui_fundamentalnodelist.h"

#include "activefundamentalnode.h"
#include "clientmodel.h"
#include "guiutil.h"
#include "init.h"
#include "fundamentalnode-sync.h"
#include "fundamentalnodeconfig.h"
#include "fundamentalnodeman.h"
#include "sync.h"
#include "wallet.h"
#include "walletmodel.h"

#include <QMessageBox>
#include <QTimer>

CCriticalSection cs_fundamentalnodes;

FundamentalnodeList::FundamentalnodeList(QWidget* parent) : QWidget(parent),
                                                  ui(new Ui::FundamentalnodeList),
                                                  clientModel(0),
                                                  walletModel(0)
{
    ui->setupUi(this);

    ui->startButton->setEnabled(false);

    int columnAliasWidth = 100;
    int columnAddressWidth = 200;
    int columnProtocolWidth = 60;
    int columnStatusWidth = 80;
    int columnActiveWidth = 130;
    int columnLastSeenWidth = 130;

    ui->tableWidgetMyFundamentalnodes->setAlternatingRowColors(true);
    ui->tableWidgetMyFundamentalnodes->setColumnWidth(0, columnAliasWidth);
    ui->tableWidgetMyFundamentalnodes->setColumnWidth(1, columnAddressWidth);
    ui->tableWidgetMyFundamentalnodes->setColumnWidth(2, columnProtocolWidth);
    ui->tableWidgetMyFundamentalnodes->setColumnWidth(3, columnStatusWidth);
    ui->tableWidgetMyFundamentalnodes->setColumnWidth(4, columnActiveWidth);
    ui->tableWidgetMyFundamentalnodes->setColumnWidth(5, columnLastSeenWidth);

    ui->tableWidgetMyFundamentalnodes->setContextMenuPolicy(Qt::CustomContextMenu);

    QAction* startAliasAction = new QAction(tr("Start alias"), this);
    contextMenu = new QMenu();
    contextMenu->addAction(startAliasAction);
    connect(ui->tableWidgetMyFundamentalnodes, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(showContextMenu(const QPoint&)));
    connect(startAliasAction, SIGNAL(triggered()), this, SLOT(on_startButton_clicked()));

    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(updateMyNodeList()));
    timer->start(1000);

    // Fill MN list
    fFilterUpdated = true;
    nTimeFilterUpdated = GetTime();
}

FundamentalnodeList::~FundamentalnodeList()
{
    delete ui;
}

void FundamentalnodeList::setClientModel(ClientModel* model)
{
    this->clientModel = model;
}

void FundamentalnodeList::setWalletModel(WalletModel* model)
{
    this->walletModel = model;
}

void FundamentalnodeList::showContextMenu(const QPoint& point)
{
    QTableWidgetItem* item = ui->tableWidgetMyFundamentalnodes->itemAt(point);
    if (item) contextMenu->exec(QCursor::pos());
}

void FundamentalnodeList::StartAlias(std::string strAlias)
{
    std::string strStatusHtml;
    strStatusHtml += "<center>Alias: " + strAlias;

    BOOST_FOREACH (CFundamentalnodeConfig::CFundamentalnodeEntry mne, fundamentalnodeConfig.getEntries()) {
        if (mne.getAlias() == strAlias) {
            std::string strError;
            CFundamentalnodeBroadcast mnb;

            bool fSuccess = CFundamentalnodeBroadcast::Create(mne.getIp(), mne.getPrivKey(), mne.getTxHash(), mne.getOutputIndex(), strError, mnb);

            if (fSuccess) {
                strStatusHtml += "<br>Successfully started fundamentalnode.";
                mnodeman.UpdateFundamentalnodeList(mnb);
                mnb.Relay();
            } else {
                strStatusHtml += "<br>Failed to start fundamentalnode.<br>Error: " + strError;
            }
            break;
        }
    }
    strStatusHtml += "</center>";

    QMessageBox msg;
    msg.setText(QString::fromStdString(strStatusHtml));
    msg.exec();

    updateMyNodeList(true);
}

void FundamentalnodeList::StartAll(std::string strCommand)
{
    int nCountSuccessful = 0;
    int nCountFailed = 0;
    std::string strFailedHtml;

    BOOST_FOREACH (CFundamentalnodeConfig::CFundamentalnodeEntry mne, fundamentalnodeConfig.getEntries()) {
        std::string strError;
        CFundamentalnodeBroadcast mnb;

        int nIndex;
        if(!mne.castOutputIndex(nIndex))
            continue;

        CTxIn txin = CTxIn(uint256S(mne.getTxHash()), uint32_t(nIndex));
        CFundamentalnode* pmn = mnodeman.Find(txin);

        if (strCommand == "start-missing" && pmn) continue;

        bool fSuccess = CFundamentalnodeBroadcast::Create(mne.getIp(), mne.getPrivKey(), mne.getTxHash(), mne.getOutputIndex(), strError, mnb);

        if (fSuccess) {
            nCountSuccessful++;
            mnodeman.UpdateFundamentalnodeList(mnb);
            mnb.Relay();
        } else {
            nCountFailed++;
            strFailedHtml += "\nFailed to start " + mne.getAlias() + ". Error: " + strError;
        }
    }
    pwalletMain->Lock();

    std::string returnObj;
    returnObj = strprintf("Successfully started %d fundamentalnodes, failed to start %d, total %d", nCountSuccessful, nCountFailed, nCountFailed + nCountSuccessful);
    if (nCountFailed > 0) {
        returnObj += strFailedHtml;
    }

    QMessageBox msg;
    msg.setText(QString::fromStdString(returnObj));
    msg.exec();

    updateMyNodeList(true);
}

void FundamentalnodeList::updateMyFundamentalnodeInfo(QString strAlias, QString strAddr, CFundamentalnode* pmn)
{
    LOCK(cs_mnlistupdate);
    bool fOldRowFound = false;
    int nNewRow = 0;

    for (int i = 0; i < ui->tableWidgetMyFundamentalnodes->rowCount(); i++) {
        if (ui->tableWidgetMyFundamentalnodes->item(i, 0)->text() == strAlias) {
            fOldRowFound = true;
            nNewRow = i;
            break;
        }
    }

    if (nNewRow == 0 && !fOldRowFound) {
        nNewRow = ui->tableWidgetMyFundamentalnodes->rowCount();
        ui->tableWidgetMyFundamentalnodes->insertRow(nNewRow);
    }

    QTableWidgetItem* aliasItem = new QTableWidgetItem(strAlias);
    QTableWidgetItem* addrItem = new QTableWidgetItem(pmn ? QString::fromStdString(pmn->addr.ToString()) : strAddr);
    QTableWidgetItem* protocolItem = new QTableWidgetItem(QString::number(pmn ? pmn->protocolVersion : -1));
    QTableWidgetItem* statusItem = new QTableWidgetItem(QString::fromStdString(pmn ? pmn->GetStatus() : "MISSING"));
    GUIUtil::DHMSTableWidgetItem* activeSecondsItem = new GUIUtil::DHMSTableWidgetItem(pmn ? (pmn->lastPing.sigTime - pmn->sigTime) : 0);
    QTableWidgetItem* lastSeenItem = new QTableWidgetItem(QString::fromStdString(DateTimeStrFormat("%Y-%m-%d %H:%M", pmn ? pmn->lastPing.sigTime : 0)));
    QTableWidgetItem* pubkeyItem = new QTableWidgetItem(QString::fromStdString(pmn ? CBitcoinAddress(pmn->pubKeyCollateralAddress.GetID()).ToString() : ""));

    ui->tableWidgetMyFundamentalnodes->setItem(nNewRow, 0, aliasItem);
    ui->tableWidgetMyFundamentalnodes->setItem(nNewRow, 1, addrItem);
    ui->tableWidgetMyFundamentalnodes->setItem(nNewRow, 2, protocolItem);
    ui->tableWidgetMyFundamentalnodes->setItem(nNewRow, 3, statusItem);
    ui->tableWidgetMyFundamentalnodes->setItem(nNewRow, 4, activeSecondsItem);
    ui->tableWidgetMyFundamentalnodes->setItem(nNewRow, 5, lastSeenItem);
    ui->tableWidgetMyFundamentalnodes->setItem(nNewRow, 6, pubkeyItem);
}

void FundamentalnodeList::updateMyNodeList(bool fForce)
{
    static int64_t nTimeMyListUpdated = 0;

    // automatically update my fundamentalnode list only once in MY_FUNDAMENTALNODELIST_UPDATE_SECONDS seconds,
    // this update still can be triggered manually at any time via button click
    int64_t nSecondsTillUpdate = nTimeMyListUpdated + MY_FUNDAMENTALNODELIST_UPDATE_SECONDS - GetTime();
    ui->secondsLabel->setText(QString::number(nSecondsTillUpdate));

    if (nSecondsTillUpdate > 0 && !fForce) return;
    nTimeMyListUpdated = GetTime();

    ui->tableWidgetMyFundamentalnodes->setSortingEnabled(false);
    BOOST_FOREACH (CFundamentalnodeConfig::CFundamentalnodeEntry mne, fundamentalnodeConfig.getEntries()) {
        int nIndex;
        if(!mne.castOutputIndex(nIndex))
            continue;

        CTxIn txin = CTxIn(uint256S(mne.getTxHash()), uint32_t(nIndex));
        CFundamentalnode* pmn = mnodeman.Find(txin);
        updateMyFundamentalnodeInfo(QString::fromStdString(mne.getAlias()), QString::fromStdString(mne.getIp()), pmn);
    }
    ui->tableWidgetMyFundamentalnodes->setSortingEnabled(true);

    // reset "timer"
    ui->secondsLabel->setText("0");
}

void FundamentalnodeList::on_startButton_clicked()
{
    // Find selected node alias
    QItemSelectionModel* selectionModel = ui->tableWidgetMyFundamentalnodes->selectionModel();
    QModelIndexList selected = selectionModel->selectedRows();

    if (selected.count() == 0) return;

    QModelIndex index = selected.at(0);
    int nSelectedRow = index.row();
    std::string strAlias = ui->tableWidgetMyFundamentalnodes->item(nSelectedRow, 0)->text().toStdString();

    // Display message box
    QMessageBox::StandardButton retval = QMessageBox::question(this, tr("Confirm fundamentalnode start"),
        tr("Are you sure you want to start fundamentalnode %1?").arg(QString::fromStdString(strAlias)),
        QMessageBox::Yes | QMessageBox::Cancel,
        QMessageBox::Cancel);

    if (retval != QMessageBox::Yes) return;

    WalletModel::EncryptionStatus encStatus = walletModel->getEncryptionStatus();

    if (encStatus == walletModel->Locked || encStatus == walletModel->UnlockedForAnonymizationOnly) {
        WalletModel::UnlockContext ctx(walletModel->requestUnlock());

        if (!ctx.isValid()) return; // Unlock wallet was cancelled

        StartAlias(strAlias);
        return;
    }

    StartAlias(strAlias);
}

void FundamentalnodeList::on_startAllButton_clicked()
{
    // Display message box
    QMessageBox::StandardButton retval = QMessageBox::question(this, tr("Confirm all fundamentalnodes start"),
        tr("Are you sure you want to start ALL fundamentalnodes?"),
        QMessageBox::Yes | QMessageBox::Cancel,
        QMessageBox::Cancel);

    if (retval != QMessageBox::Yes) return;

    WalletModel::EncryptionStatus encStatus = walletModel->getEncryptionStatus();

    if (encStatus == walletModel->Locked || encStatus == walletModel->UnlockedForAnonymizationOnly) {
        WalletModel::UnlockContext ctx(walletModel->requestUnlock());

        if (!ctx.isValid()) return; // Unlock wallet was cancelled

        StartAll();
        return;
    }

    StartAll();
}

void FundamentalnodeList::on_startMissingButton_clicked()
{
    if (!fundamentalnodeSync.IsFundamentalnodeListSynced()) {
        QMessageBox::critical(this, tr("Command is not available right now"),
            tr("You can't use this command until fundamentalnode list is synced"));
        return;
    }

    // Display message box
    QMessageBox::StandardButton retval = QMessageBox::question(this,
        tr("Confirm missing fundamentalnodes start"),
        tr("Are you sure you want to start MISSING fundamentalnodes?"),
        QMessageBox::Yes | QMessageBox::Cancel,
        QMessageBox::Cancel);

    if (retval != QMessageBox::Yes) return;

    WalletModel::EncryptionStatus encStatus = walletModel->getEncryptionStatus();

    if (encStatus == walletModel->Locked || encStatus == walletModel->UnlockedForAnonymizationOnly) {
        WalletModel::UnlockContext ctx(walletModel->requestUnlock());

        if (!ctx.isValid()) return; // Unlock wallet was cancelled

        StartAll("start-missing");
        return;
    }

    StartAll("start-missing");
}

void FundamentalnodeList::on_tableWidgetMyFundamentalnodes_itemSelectionChanged()
{
    if (ui->tableWidgetMyFundamentalnodes->selectedItems().count() > 0) {
        ui->startButton->setEnabled(true);
    }
}

void FundamentalnodeList::on_UpdateButton_clicked()
{
    updateMyNodeList(true);
}
