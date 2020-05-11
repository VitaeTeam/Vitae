#include "qt/vitae/masternodeswidget.h"
#include "qt/vitae/forms/ui_masternodeswidget.h"
#include "qt/vitae/qtutils.h"
#include "qt/vitae/mnrow.h"

#include "qt/vitae/masternodewizarddialog.h"

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
#include <boost/filesystem.hpp>
#include <iostream>
#include <fstream>

#include <QMessageBox>
#include <QTimer>

#include <iostream>

#define DECORATION_SIZE 65
#define NUM_ITEMS 3

class MNHolder : public FurListRow<QWidget*>
{
public:
    MNHolder();

    explicit MNHolder(bool _isLightTheme) : FurListRow(), isLightTheme(_isLightTheme){}

    MNRow* createHolder(int pos) override{
        return new MNRow();
    }

    void init(QWidget* holder,const QModelIndex &index, bool isHovered, bool isSelected) const override{
        MNRow* row = static_cast<MNRow*>(holder);
        QString label = index.data(Qt::DisplayRole).toString();
        QString address = index.sibling(index.row(), MNModel::ADDRESS).data(Qt::DisplayRole).toString();
        QString status = index.sibling(index.row(), MNModel::STATUS).data(Qt::DisplayRole).toString();
        row->updateView("Address: " + address, label, status);
    }

    QColor rectColor(bool isHovered, bool isSelected) override{
        return getRowColor(isLightTheme, isHovered, isSelected);
    }

    ~MNHolder() override{}

    bool isLightTheme;
};

MasterNodesWidget::MasterNodesWidget(VITAEGUI *parent) :
    PWidget(parent),
    ui(new Ui::MasterNodesWidget)
{
    ui->setupUi(this);

    delegate = new FurAbstractListItemDelegate(
            DECORATION_SIZE,
            new MNHolder(isLightTheme()),
            this
    );
    mnModel = new MNModel(this);

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
    ui->labelTitle->setText(tr("Master Node"));
    ui->labelTitle->setProperty("cssClass", "text-title-screen");
    ui->labelTitle->setFont(fontLight);

    /* Subtitle */
    ui->labelSubtitle1->setText(tr("Full nodes that incentivize node operators to perform the core consensus functions\nand vote on the treasury system receiving a periodic reward."));
    ui->labelSubtitle1->setProperty("cssClass", "text-subtitle");

    /* Buttons */
    ui->pushButtonSave->setText(tr("Create Master Node"));
    setCssBtnPrimary(ui->pushButtonSave);

    /* Options */
    ui->btnAbout->setTitleClassAndText("btn-title-grey", "What is a Master Node?");
    ui->btnAbout->setSubTitleClassAndText("text-subtitle", "FAQ explaining what Master Nodes are");

    setCssProperty(ui->listMn, "container");
    ui->listMn->setItemDelegate(delegate);
    ui->listMn->setIconSize(QSize(DECORATION_SIZE, DECORATION_SIZE));
    ui->listMn->setMinimumHeight(NUM_ITEMS * (DECORATION_SIZE + 2));
    ui->listMn->setAttribute(Qt::WA_MacShowFocusRect, false);
    ui->listMn->setSelectionBehavior(QAbstractItemView::SelectRows);

    ui->emptyContainer->setVisible(false);
    setCssProperty(ui->pushImgEmpty, "img-empty-master");
    ui->labelEmpty->setText(tr("No active Masternode yet"));
    setCssProperty(ui->labelEmpty, "text-empty");

    connect(ui->pushButtonSave, SIGNAL(clicked()), this, SLOT(onCreateMNClicked()));
    connect(ui->listMn, SIGNAL(clicked(QModelIndex)), this, SLOT(onMNClicked(QModelIndex)));
}

void MasterNodesWidget::loadWalletModel(){
    if(walletModel) {
        ui->listMn->setModel(mnModel);
        ui->listMn->setModelColumn(AddressTableModel::Label);
        updateListState();
    }
}

void MasterNodesWidget::updateListState() {
    if (mnModel->rowCount() > 0) {
        ui->listMn->setVisible(true);
        ui->emptyContainer->setVisible(false);
    } else {
        ui->listMn->setVisible(false);
        ui->emptyContainer->setVisible(true);
    }
}

void MasterNodesWidget::onMNClicked(const QModelIndex &index){
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
}

void MasterNodesWidget::onEditMNClicked(){
    if(walletModel) {
        // Start MN
        QString strAlias = this->index.data(Qt::DisplayRole).toString();
        bool ret;
        ask(tr("Start Master Node"), tr("Are you sure you want to start masternode %1?").arg(strAlias), &ret);

        if (ret) {
            if(!verifyWalletUnlocked()) return;
            startAlias(strAlias);
        }
    }
}

void MasterNodesWidget::startAlias(QString strAlias)
{
    QString strStatusHtml;
    strStatusHtml += "<center>Alias: " + strAlias;

    for (CFundamentalnodeConfig::CFundamentalnodeEntry mne : fundamentalnodeConfig.getEntries()) {
        if (mne.getAlias() == strAlias.toStdString()) {
            std::string strError;
            CFundamentalnodeBroadcast mnb;

            bool fSuccess = CFundamentalnodeBroadcast::Create(mne.getIp(), mne.getPrivKey(), mne.getTxHash(), mne.getOutputIndex(), strError, mnb);

            if (fSuccess) {
                strStatusHtml += "<br>Successfully started masternode.";
                mnodeman.UpdateFundamentalnodeList(mnb);
                mnb.Relay();
            } else {
                strStatusHtml += "<br>Failed to start masternode.<br>Error: " + QString::fromStdString(strError);
            }
            break;
        }
    }
    strStatusHtml += "</center>";

    inform(strStatusHtml);

    // TODO: Update MN list.
}

void MasterNodesWidget::onInfoMNClicked(){

}

void MasterNodesWidget::onDeleteMNClicked(){
    QString qAliasString = index.data(Qt::DisplayRole).toString();
    std::string aliasToRemove = qAliasString.toStdString();

    bool ret = false;
    ask(tr("Delete Master Node"), tr("You are just about to delete Master Node:\n%1\n\nAre you sure?").arg(qAliasString), &ret);

    if (!ret)
        return;

    std::string strConfFile = "masternode.conf";
    std::string strDataDir = GetDataDir().string();
    if (strConfFile != boost::filesystem::basename(strConfFile) + boost::filesystem::extension(strConfFile)){
        throw std::runtime_error(strprintf(_("masternode.conf %s resides outside data directory %s"), strConfFile, strDataDir));
    }

    filesystem::path pathBootstrap = GetDataDir() / strConfFile;
    if (filesystem::exists(pathBootstrap)) {
        boost::filesystem::path pathMasternodeConfigFile = GetMasternodeConfigFile();
        boost::filesystem::ifstream streamConfig(pathMasternodeConfigFile);

        if (!streamConfig.good()) {
            inform(tr("Invalid masternode.conf file"));
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
                    inform(tr("Error parsing masternode.conf file"));
                    return;
                }
            }

            if (aliasToRemove == alias) {
                lineNumToRemove = linenumber;
            } else
                lineCopy += line + "\n";

        }

        if (lineCopy.size() == 0) {
            lineCopy = "# Masternode config file\n"
                                    "# Format: alias IP:port masternodeprivkey collateral_output_txid collateral_output_index\n"
                                    "# Example: mn1 127.0.0.2:51472 93HaYBVUCYjEMeeH1Y4sBGLALQZE1Yc1K64xiqgX37tGBDQL8Xg 2bcd3c84c84f87eaa86e4e56834c92927a07f9e18718810b92e0d0324456a67c 0\n";
        }

        streamConfig.close();

        if (lineNumToRemove != -1) {
            boost::filesystem::path pathConfigFile("masternode_temp.conf");
            if (!pathConfigFile.is_complete()) pathConfigFile = GetDataDir() / pathConfigFile;
            FILE* configFile = fopen(pathConfigFile.string().c_str(), "w");
            fwrite(lineCopy.c_str(), std::strlen(lineCopy.c_str()), 1, configFile);
            fclose(configFile);

            boost::filesystem::path pathOldConfFile("old_masternode.conf");
            if (!pathOldConfFile.is_complete()) pathOldConfFile = GetDataDir() / pathOldConfFile;
            if (filesystem::exists(pathOldConfFile)) {
                filesystem::remove(pathOldConfFile);
            }
            rename(pathMasternodeConfigFile, pathOldConfFile);

            boost::filesystem::path pathNewConfFile("masternode.conf");
            if (!pathNewConfFile.is_complete()) pathNewConfFile = GetDataDir() / pathNewConfFile;
            rename(pathConfigFile, pathNewConfFile);

            // Remove alias
            masternodeConfig.remove(aliasToRemove);
            // Update list
            mnModel->removeMn(index);
            updateListState();
        }
    } else{
        inform(tr("masternode.conf file doesn't exists"));
    }
}

void MasterNodesWidget::onCreateMNClicked(){
    if(verifyWalletUnlocked()) {
        if(walletModel->getBalance() <= (COIN * 10000)){
            inform(tr("No enough balance to create a master node, 10,000 PIV required."));
            return;
        }
        showHideOp(true);
        MasterNodeWizardDialog *dialog = new MasterNodeWizardDialog(walletModel, window);
        if(openDialogWithOpaqueBackgroundY(dialog, window, 5, 7)) {
            if (dialog->isOk) {
                // Update list
                mnModel->addMn(dialog->mnEntry);
                updateListState();
                // add mn
                inform(dialog->returnStr);
            } else {
                warn(tr("Error creating master node"), dialog->returnStr);
            }
        }
        dialog->deleteLater();
    }
}

void MasterNodesWidget::changeTheme(bool isLightTheme, QString& theme){
    static_cast<MNHolder*>(this->delegate->getRowFactory())->isLightTheme = isLightTheme;
}

MasterNodesWidget::~MasterNodesWidget()
{
    delete ui;
}
