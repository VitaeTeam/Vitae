// Copyright (c) 2019-2020 The PIVX developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "qt/vitae/fundamentalnodeswidget.h"
#include "qt/vitae/forms/ui_fundamentalnodeswidget.h"
#include "qt/vitae/qtutils.h"
#include "qt/vitae/mnrow.h"
#include "qt/vitae/mninfodialog.h"

#include "qt/vitae/fundamentalnodewizarddialog.h"

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
#include "askpassphrasedialog.h"
#include "util.h"
#include "qt/vitae/optionbutton.h"
#include <boost/filesystem.hpp>
#include <iostream>
#include <fstream>

#define DECORATION_SIZE 65
#define NUM_ITEMS 3
#define REQUEST_START_ALL 1
#define REQUEST_START_MISSING 2

class MNHolder : public FurListRow<QWidget*>
{
public:
    MNHolder();

    explicit MNHolder(bool _isLightTheme) : FurListRow(), isLightTheme(_isLightTheme){}

    MNRow* createHolder(int pos) override{
        if(!cachedRow) cachedRow = new MNRow();
        return cachedRow;
    }

    void init(QWidget* holder,const QModelIndex &index, bool isHovered, bool isSelected) const override{
        MNRow* row = static_cast<MNRow*>(holder);
        QString label = index.data(Qt::DisplayRole).toString();
        QString address = index.sibling(index.row(), FNModel::ADDRESS).data(Qt::DisplayRole).toString();
        QString status = index.sibling(index.row(), FNModel::STATUS).data(Qt::DisplayRole).toString();
        bool wasCollateralAccepted = index.sibling(index.row(), FNModel::WAS_COLLATERAL_ACCEPTED).data(Qt::DisplayRole).toBool();
        row->updateView("Address: " + address, label, status, wasCollateralAccepted);
    }

    QColor rectColor(bool isHovered, bool isSelected) override{
        return getRowColor(isLightTheme, isHovered, isSelected);
    }

    ~MNHolder() override{}

    bool isLightTheme;
    MNRow* cachedRow = nullptr;
};

FundamentalNodesWidget::FundamentalNodesWidget(VITAEGUI *parent) :
    PWidget(parent),
    ui(new Ui::FundamentalNodesWidget),
    isLoading(false)
{
    ui->setupUi(this);

    delegate = new FurAbstractListItemDelegate(
            DECORATION_SIZE,
            new MNHolder(isLightTheme()),
            this
    );
    fnModel = new FNModel(this);

    this->setStyleSheet(parent->styleSheet());

    /* Containers */
    setCssProperty(ui->left, "container");
    ui->left->setContentsMargins(0,20,0,20);
    setCssProperty(ui->right, "container-right");
    ui->right->setContentsMargins(20,20,20,20);

    /* Light Font */
    QFont fontLight;
    fontLight.setWeight(QFont::Light);

    /* Title */
    ui->labelTitle->setText(tr("Fundamentalnodes"));
    setCssTitleScreen(ui->labelTitle);
    ui->labelTitle->setFont(fontLight);

    ui->labelSubtitle1->setText(tr("Full nodes that incentivize node operators to perform the core consensus functions\nand vote on the treasury system receiving a periodic reward."));
    setCssSubtitleScreen(ui->labelSubtitle1);

    /* Buttons */
    ui->pushButtonSave->setText(tr("Create Fundamentalnode Controller"));
    setCssBtnPrimary(ui->pushButtonSave);
    setCssBtnPrimary(ui->pushButtonStartAll);
    setCssBtnPrimary(ui->pushButtonStartMissing);

    /* Options */
    ui->btnAbout->setTitleClassAndText("btn-title-grey", "What is a Fundamentalnode?");
    ui->btnAbout->setSubTitleClassAndText("text-subtitle", "FAQ explaining what Fundamentalnodes are");
    ui->btnAboutController->setTitleClassAndText("btn-title-grey", "What is a Controller?");
    ui->btnAboutController->setSubTitleClassAndText("text-subtitle", "FAQ explaining what is a Fundamentalnode Controller");

    setCssProperty(ui->listMn, "container");
    ui->listMn->setItemDelegate(delegate);
    ui->listMn->setIconSize(QSize(DECORATION_SIZE, DECORATION_SIZE));
    ui->listMn->setMinimumHeight(NUM_ITEMS * (DECORATION_SIZE + 2));
    ui->listMn->setAttribute(Qt::WA_MacShowFocusRect, false);
    ui->listMn->setSelectionBehavior(QAbstractItemView::SelectRows);

    ui->emptyContainer->setVisible(false);
    setCssProperty(ui->pushImgEmpty, "img-empty-fundamental");
    ui->labelEmpty->setText(tr("No active Fundamentalnode yet"));
    setCssProperty(ui->labelEmpty, "text-empty");

    connect(ui->pushButtonSave, SIGNAL(clicked()), this, SLOT(onCreateMNClicked()));
    connect(ui->pushButtonStartAll, &QPushButton::clicked, [this]() {
        onStartAllClicked(REQUEST_START_ALL);
    });
    connect(ui->pushButtonStartMissing, &QPushButton::clicked, [this]() {
        onStartAllClicked(REQUEST_START_MISSING);
    });
    connect(ui->listMn, SIGNAL(clicked(QModelIndex)), this, SLOT(onMNClicked(QModelIndex)));
    connect(ui->btnAbout, &OptionButton::clicked, [this](){window->openFAQ(9);});
    connect(ui->btnAboutController, &OptionButton::clicked, [this](){window->openFAQ(10);});
}

void FundamentalNodesWidget::showEvent(QShowEvent *event){
    if (fnModel) fnModel->updateMNList();
    if(!timer) {
        timer = new QTimer(this);
        connect(timer, &QTimer::timeout, [this]() {fnModel->updateMNList();});
    }
    timer->start(30000);
}

void FundamentalNodesWidget::hideEvent(QHideEvent *event){
    if(timer) timer->stop();
}

void FundamentalNodesWidget::loadWalletModel(){
    if(walletModel) {
        ui->listMn->setModel(fnModel);
        ui->listMn->setModelColumn(AddressTableModel::Label);
        updateListState();
    }
}

void FundamentalNodesWidget::updateListState() {
    bool show = fnModel->rowCount() > 0;
    ui->listMn->setVisible(show);
    ui->emptyContainer->setVisible(!show);
    ui->pushButtonStartAll->setVisible(show);
}

void FundamentalNodesWidget::onMNClicked(const QModelIndex &index){
    ui->listMn->setCurrentIndex(index);
    QRect rect = ui->listMn->visualRect(index);
    QPoint pos = rect.topRight();
    pos.setX(pos.x() - (DECORATION_SIZE * 2));
    pos.setY(pos.y() + (DECORATION_SIZE * 1.5));
    if(!this->menu){
        this->menu = new TooltipMenu(window, this);
        this->menu->setEditBtnText(tr("Start"));
        this->menu->setDeleteBtnText(tr("Delete"));
        this->menu->setCopyBtnText(tr("Info"));
        connect(this->menu, &TooltipMenu::message, this, &AddressesWidget::message);
        connect(this->menu, SIGNAL(onEditClicked()), this, SLOT(onEditMNClicked()));
        connect(this->menu, SIGNAL(onDeleteClicked()), this, SLOT(onDeleteMNClicked()));
        connect(this->menu, SIGNAL(onCopyClicked()), this, SLOT(onInfoMNClicked()));
        this->menu->adjustSize();
    }else {
        this->menu->hide();
    }
    this->index = index;
    menu->move(pos);
    menu->show();

    // Back to regular status
    ui->listMn->scrollTo(index);
    ui->listMn->clearSelection();
    ui->listMn->setFocus();
}

bool FundamentalNodesWidget::checkMNsNetwork() {
    bool isTierTwoSync = fnModel->isMNsNetworkSynced();
    if (!isTierTwoSync) inform(tr("Please wait until the node is fully synced"));
    return isTierTwoSync;
}

void FundamentalNodesWidget::onEditMNClicked(){
    if(walletModel) {
        if (!checkMNsNetwork()) return;
        if (index.sibling(index.row(), FNModel::WAS_COLLATERAL_ACCEPTED).data(Qt::DisplayRole).toBool()) {
            // Start MN
            QString strAlias = this->index.data(Qt::DisplayRole).toString();
            if (ask(tr("Start Fundamentalnode"), tr("Are you sure you want to start fundamentalnode %1?\n").arg(strAlias))) {
                if (!verifyWalletUnlocked()) return;
                startAlias(strAlias);
            }
        }else {
            inform(tr("Cannot start fundamentalnode, the collateral transaction has not been accepted by the network.\nPlease wait few more minutes."));
        }
    }
}

void FundamentalNodesWidget::startAlias(QString strAlias) {
    QString strStatusHtml;
    strStatusHtml += "Alias: " + strAlias + " ";

    for (CFundamentalnodeConfig::CFundamentalnodeEntry mne : fundamentalnodeConfig.getEntries()) {
        if (mne.getAlias() == strAlias.toStdString()) {
            std::string strError;
            strStatusHtml += (!startMN(mne, strError)) ? ("failed to start.\nError: " + QString::fromStdString(strError)) : "successfully started.";
            break;
        }
    }
    // update UI and notify
    updateModelAndInform(strStatusHtml);
}

void FundamentalNodesWidget::updateModelAndInform(QString informText) {
    fnModel->updateMNList();
    inform(informText);
}

bool FundamentalNodesWidget::startMN(CFundamentalnodeConfig::CFundamentalnodeEntry mne, std::string& strError) {
    CFundamentalnodeBroadcast mnb;
    if (! CFundamentalnodeBroadcast::Create(mne.getIp(), mne.getPrivKey(), mne.getTxHash(), mne.getOutputIndex(), strError, mnb))
        return false;
    mnodeman.UpdateFundamentalnodeList(mnb);
    mnb.Relay();
    fnModel->updateMNList();
    return true;
}

void FundamentalNodesWidget::onStartAllClicked(int type) {
    if (!verifyWalletUnlocked()) return;
    if (!checkMNsNetwork()) return;
    if (isLoading) {
        inform(tr("Background task is being executed, please wait"));
    } else {
        isLoading = true;
        if (!execute(type)) {
            isLoading = false;
            inform(tr("Cannot perform Mastenodes start"));
        }
    }
}

bool FundamentalNodesWidget::startAll(QString& failText, bool onlyMissing) {
    int amountOfMnFailed = 0;
    int amountOfMnStarted = 0;
    for (CFundamentalnodeConfig::CFundamentalnodeEntry mne : fundamentalnodeConfig.getEntries()) {
        // Check for missing only
        QString mnAlias = QString::fromStdString(mne.getAlias());
        if (onlyMissing && !fnModel->isMNInactive(mnAlias)) {
            if (!fnModel->isMNActive(mnAlias))
                amountOfMnFailed++;
            continue;
        }

        std::string strError;
        if (!startMN(mne, strError)) {
            amountOfMnFailed++;
        } else {
            amountOfMnStarted++;
        }
    }
    if (amountOfMnFailed > 0) {
        failText = tr("%1 Fundamentalnodes failed to start, %2 started").arg(amountOfMnFailed).arg(amountOfMnStarted);
        return false;
    }
    return true;
}

void FundamentalNodesWidget::run(int type) {
    bool isStartMissing = type == REQUEST_START_MISSING;
    if (type == REQUEST_START_ALL || isStartMissing) {
        QString failText;
        QString inform = startAll(failText, isStartMissing) ? tr("All Fundamentalnodes started!") : failText;
        QMetaObject::invokeMethod(this, "updateModelAndInform", Qt::QueuedConnection,
                                  Q_ARG(QString, inform));
    }

    isLoading = false;
}

void FundamentalNodesWidget::onError(QString error, int type) {
    if (type == REQUEST_START_ALL) {
        QMetaObject::invokeMethod(this, "inform", Qt::QueuedConnection,
                                  Q_ARG(QString, "Error starting all Fundamentalnodes"));
    }
}

void FundamentalNodesWidget::onInfoMNClicked() {
    if(!verifyWalletUnlocked()) return;
    showHideOp(true);
    MnInfoDialog* dialog = new MnInfoDialog(window);
    QString label = index.data(Qt::DisplayRole).toString();
    QString address = index.sibling(index.row(), FNModel::ADDRESS).data(Qt::DisplayRole).toString();
    QString status = index.sibling(index.row(), FNModel::STATUS).data(Qt::DisplayRole).toString();
    QString txId = index.sibling(index.row(), FNModel::COLLATERAL_ID).data(Qt::DisplayRole).toString();
    QString outIndex = index.sibling(index.row(), FNModel::COLLATERAL_OUT_INDEX).data(Qt::DisplayRole).toString();
    QString pubKey = index.sibling(index.row(), FNModel::PUB_KEY).data(Qt::DisplayRole).toString();
    dialog->setData(pubKey, label, address, txId, outIndex, status);
    dialog->adjustSize();
    showDialog(dialog, 3, 17);
    if (dialog->exportMN){
        if (ask(tr("Remote Fundamentalnode Data"),
                tr("You are just about to export the required data to run a Fundamentalnode\non a remote server to your clipboard.\n\n\n"
                   "You will only have to paste the data in the vitae.conf file\nof your remote server and start it, "
                   "then start the Fundamentalnode using\nthis controller wallet (select the Fundamentalnode in the list and press \"start\").\n"
                ))) {
            // export data
            QString exportedMN = "fundamentalnode=1\n"
                                 "externalip=" + address.left(address.lastIndexOf(":")) + "\n" +
                                 "fundamentalnodeaddr=" + address + + "\n" +
                                 "fundamentalnodeprivkey=" + index.sibling(index.row(), FNModel::PRIV_KEY).data(Qt::DisplayRole).toString() + "\n";
            GUIUtil::setClipboard(exportedMN);
            inform(tr("Fundamentalnode exported!, check your clipboard"));
        }
    }

    dialog->deleteLater();
}

void FundamentalNodesWidget::onDeleteMNClicked(){
    QString qAliasString = index.data(Qt::DisplayRole).toString();
    std::string aliasToRemove = qAliasString.toStdString();

    if (!ask(tr("Delete Fundamentalnode"), tr("You are just about to delete Fundamentalnode:\n%1\n\nAre you sure?").arg(qAliasString)))
        return;

    std::string strConfFile = "fundamentalnode.conf";
    std::string strDataDir = GetDataDir().string();
    if (strConfFile != boost::filesystem::basename(strConfFile) + boost::filesystem::extension(strConfFile)){
        throw std::runtime_error(strprintf(_("fundamentalnode.conf %s resides outside data directory %s"), strConfFile, strDataDir));
    }

    boost::filesystem::path pathBootstrap = GetDataDir() / strConfFile;
    if (boost::filesystem::exists(pathBootstrap)) {
        boost::filesystem::path pathFundamentalnodeConfigFile = GetFundamentalnodeConfigFile();
        boost::filesystem::ifstream streamConfig(pathFundamentalnodeConfigFile);

        if (!streamConfig.good()) {
            inform(tr("Invalid fundamentalnode.conf file"));
            return;
        }

        int lineNumToRemove = -1;
        int linenumber = 1;
        std::string lineCopy = "";
        for (std::string line; std::getline(streamConfig, line); linenumber++) {
            if (line.empty()) continue;

            std::istringstream iss(line);
            std::string comment, alias, ip, privKey, txHash, outputIndex;

            if (iss >> comment) {
                if (comment.at(0) == '#') continue;
                iss.str(line);
                iss.clear();
            }

            if (!(iss >> alias >> ip >> privKey >> txHash >> outputIndex)) {
                iss.str(line);
                iss.clear();
                if (!(iss >> alias >> ip >> privKey >> txHash >> outputIndex)) {
                    streamConfig.close();
                    inform(tr("Error parsing fundamentalnode.conf file"));
                    return;
                }
            }

            if (aliasToRemove == alias) {
                lineNumToRemove = linenumber;
            } else
                lineCopy += line + "\n";

        }

        if (lineCopy.size() == 0) {
            lineCopy = "# Fundamentalnode config file\n"
                                    "# Format: alias IP:port fundamentalnodeprivkey collateral_output_txid collateral_output_index\n"
                                    "# Example: mn1 127.0.0.2:51472 93HaYBVUCYjEMeeH1Y4sBGLALQZE1Yc1K64xiqgX37tGBDQL8Xg 2bcd3c84c84f87eaa86e4e56834c92927a07f9e18718810b92e0d0324456a67c 0\n";
        }

        streamConfig.close();

        if (lineNumToRemove != -1) {
            boost::filesystem::path pathConfigFile("fundamentalnode_temp.conf");
            if (!pathConfigFile.is_complete()) pathConfigFile = GetDataDir() / pathConfigFile;
            FILE* configFile = fopen(pathConfigFile.string().c_str(), "w");
            fwrite(lineCopy.c_str(), std::strlen(lineCopy.c_str()), 1, configFile);
            fclose(configFile);

            boost::filesystem::path pathOldConfFile("old_fundamentalnode.conf");
            if (!pathOldConfFile.is_complete()) pathOldConfFile = GetDataDir() / pathOldConfFile;
            if (boost::filesystem::exists(pathOldConfFile)) {
                boost::filesystem::remove(pathOldConfFile);
            }
            rename(pathFundamentalnodeConfigFile, pathOldConfFile);

            boost::filesystem::path pathNewConfFile("fundamentalnode.conf");
            if (!pathNewConfFile.is_complete()) pathNewConfFile = GetDataDir() / pathNewConfFile;
            rename(pathConfigFile, pathNewConfFile);

            // Remove alias
            fundamentalnodeConfig.remove(aliasToRemove);
            // Update list
            fnModel->removeMn(index);
            updateListState();
        }
    } else{
        inform(tr("fundamentalnode.conf file doesn't exists"));
    }
}

void FundamentalNodesWidget::onCreateMNClicked(){
    if(verifyWalletUnlocked()) {
        if(walletModel->getBalance() <= (COIN * 10000)){
            inform(tr("Not enough balance to create a fundamentalnode, 10,000 VIT required."));
            return;
        }
        showHideOp(true);
        FundamentalNodeWizardDialog *dialog = new FundamentalNodeWizardDialog(walletModel, window);
        if(openDialogWithOpaqueBackgroundY(dialog, window, 5, 7)) {
            if (dialog->isOk) {
                // Update list
                fnModel->addMn(dialog->mnEntry);
                updateListState();
                // add mn
                inform(dialog->returnStr);
            } else {
                warn(tr("Error creating fundamentalnode"), dialog->returnStr);
            }
        }
        dialog->deleteLater();
    }
}

void FundamentalNodesWidget::changeTheme(bool isLightTheme, QString& theme){
    static_cast<MNHolder*>(this->delegate->getRowFactory())->isLightTheme = isLightTheme;
}

FundamentalNodesWidget::~FundamentalNodesWidget()
{
    delete ui;
}
