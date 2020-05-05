#include "qt/vitae/receivewidget.h"
#include "qt/vitae/forms/ui_receivewidget.h"
#include "qt/vitae/requestdialog.h"
#include "qt/vitae/addnewcontactdialog.h"
#include "qt/vitae/qtutils.h"

#include "qt/vitae/VITAEGUI.h"
#include "qt/vitae/myaddressrow.h"
#include "qt/vitae/qtutils.h"
#include "qt/vitae/furlistrow.h"
#include "walletmodel.h"
#include "guiutil.h"
#include "base58.h"
#include "script/standard.h"

#include <QAbstractItemDelegate>
#include <QPainter>
#include <QSettings>
#include <QModelIndex>
#include <QFile>
#include <QClipboard>
#include <QColor>
#include <QDateTime>

#include <iostream>

#define DECORATION_SIZE 70
#define NUM_ITEMS 3

class AddressHolder : public FurListRow<QWidget*>
{
public:
    AddressHolder();

    explicit AddressHolder(bool _isLightTheme) : FurListRow(), isLightTheme(_isLightTheme){}

    MyAddressRow* createHolder(int pos) override{
        return new MyAddressRow();
    }

    void init(QWidget* holder,const QModelIndex &index, bool isHovered, bool isSelected) const override{
        MyAddressRow *row = static_cast<MyAddressRow*>(holder);

        QString address = index.data(Qt::DisplayRole).toString();
        QModelIndex sibling = index.sibling(index.row(), AddressTableModel::Label);
        QString label = sibling.data(Qt::DisplayRole).toString();

        row->updateView(address, label);

    }

    QColor rectColor(bool isHovered, bool isSelected) override{
        return getRowColor(isLightTheme, isHovered, isSelected);
    }

    ~AddressHolder() override{}

    bool isLightTheme;
};

ReceiveWidget::ReceiveWidget(VITAEGUI* _window, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ReceiveWidget),
    window(_window)
{
    ui->setupUi(this);

    delegate = new FurAbstractListItemDelegate(
                DECORATION_SIZE,
                new AddressHolder(isLightTheme()),
                this
                );

    // Stylesheet

    this->setStyleSheet(_window->styleSheet());

    // Containers
    ui->left->setProperty("cssClass", "container");
    ui->left->setContentsMargins(20,20,20,20);
    ui->right->setProperty("cssClass", "container-right");
    ui->right->setContentsMargins(0,9,0,0);


    // Title
    ui->labelTitle->setText("Receive");
    ui->labelTitle->setProperty("cssClass", "text-title-screen");

    ui->labelSubtitle1->setText("Scan the QR code or copy the address to receive PIV.");
    ui->labelSubtitle1->setProperty("cssClass", "text-subtitle");


    // Address

    ui->labelAddress->setText("No address ");
    ui->labelAddress->setProperty("cssClass", "label-address-box");

    ui->labelDate->setText("Dec. 19, 2018");
    ui->labelDate->setProperty("cssClass", "text-subtitle");
    ui->labelLabel->setText("");
    ui->labelLabel->setProperty("cssClass", "text-subtitle");

    // Options

    ui->btnMyAddresses->setTitleClassAndText("btn-title-grey", "My Addresses");
    ui->btnMyAddresses->setSubTitleClassAndText("text-subtitle", "List your own addresses.");
    ui->btnMyAddresses->layout()->setMargin(0);
    ui->btnMyAddresses->setRightIconClass("btn-dropdown");
    ui->btnMyAddresses->isActive(false);


    ui->btnRequest->setTitleClassAndText("btn-title-grey", "Create Request");
    ui->btnRequest->setSubTitleClassAndText("text-subtitle", "Request payment with a fixed amount.");
    ui->btnRequest->layout()->setMargin(0);
    ui->btnRequest->isActive(false);


    // Buttons
    connect(ui->btnRequest, SIGNAL(clicked()), this, SLOT(onRequestClicked()));
    connect(ui->btnMyAddresses, SIGNAL(clicked()), this, SLOT(onMyAddressesClicked()));


    ui->pushButtonLabel->setText("Add Label");
    ui->pushButtonLabel->setProperty("cssClass", "btn-secundary-label");

    ui->pushButtonNewAddress->setText("Generate Address");
    ui->pushButtonNewAddress->setProperty("cssClass", "btn-secundary-new-address");

    ui->pushButtonCopy->setText("Copy");
    ui->pushButtonCopy->setProperty("cssClass", "btn-secundary-copy");


    // List Addresses
    ui->listViewAddress->setProperty("cssClass", "container");
    ui->listViewAddress->setItemDelegate(delegate);
    ui->listViewAddress->setIconSize(QSize(DECORATION_SIZE, DECORATION_SIZE));
    ui->listViewAddress->setMinimumHeight(NUM_ITEMS * (DECORATION_SIZE + 2));
    ui->listViewAddress->setAttribute(Qt::WA_MacShowFocusRect, false);
    ui->listViewAddress->setSelectionBehavior(QAbstractItemView::SelectRows);


    spacer = new QSpacerItem(40, 20, QSizePolicy::Maximum, QSizePolicy::Expanding);
    ui->btnMyAddresses->setChecked(true);
    ui->container_right->addItem(spacer);
    ui->listViewAddress->setVisible(false);

    // Connect
    connect(window, SIGNAL(themeChanged(bool, QString&)), this, SLOT(changeTheme(bool, QString&)));
    connect(ui->pushButtonLabel, SIGNAL(clicked()), this, SLOT(onLabelClicked()));
    connect(ui->pushButtonCopy, SIGNAL(clicked()), this, SLOT(onCopyClicked()));
    connect(ui->pushButtonNewAddress, SIGNAL(clicked()), this, SLOT(onNewAddressClicked()));
    connect(ui->listViewAddress, SIGNAL(clicked(QModelIndex)), this, SLOT(handleAddressClicked(QModelIndex)));
}

void ReceiveWidget::setWalletModel(WalletModel* model){
    this->walletModel = model;
    if(walletModel) {
        this->addressTableModel = model->getAddressTableModel();
        this->filter = new AddressFilterProxyModel(AddressTableModel::Receive, this);
        this->filter->setSourceModel(addressTableModel);
        ui->listViewAddress->setModel(this->filter);
        ui->listViewAddress->setModelColumn(AddressTableModel::Address);

        if(!info) info = new SendCoinsRecipient();
        refreshView();

        // data change
        connect(this->addressTableModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)), this, SLOT(refreshView()));
    }
}

void ReceiveWidget::refreshView(QString refreshAddress){
    QString latestAddress = (refreshAddress.isEmpty()) ? this->addressTableModel->getLastUnusedAddress() : refreshAddress;
    if (latestAddress.isEmpty()) // new default address
       latestAddress = QString::fromStdString(walletModel->getNewAddress("Default").ToString());
    ui->labelAddress->setText(latestAddress);
    int64_t time = walletModel->getKeyCreationTime(CBitcoinAddress(latestAddress.toStdString()));
    ui->labelDate->setText(GUIUtil::dateTimeStr(QDateTime::fromTime_t(static_cast<uint>(time))));
    updateQr(latestAddress);
    updateLabel();
}

void ReceiveWidget::updateLabel(){
    if(!info->address.isEmpty()) {
        // Check if address label exists
        QString label = this->addressTableModel->labelForAddress(info->address);
        if (!label.isEmpty()) {
            // TODO: Show label.. complete me..
            ui->labelLabel->setVisible(true);
            ui->labelLabel->setText(label);
            ui->pushButtonLabel->setText(tr("Change Label"));
        }else{
            ui->labelLabel->setVisible(false);
        }
    }
}

void ReceiveWidget::updateQr(QString address){
    info->address = address;
    QString uri = GUIUtil::formatBitcoinURI(*info);
    ui->labelQrImg->setText("");

    QString error;
    QColor qrColor("#382d4d");
    QPixmap pixmap = encodeToQr(uri, error, qrColor);
    if(!pixmap.isNull()){
        qrImage = &pixmap;
        ui->labelQrImg->setPixmap(qrImage->scaled(ui->labelQrImg->width(), ui->labelQrImg->height()));
    }else{
        ui->labelQrImg->setText(!error.isEmpty() ? error : "Error encoding address");
    }
}

void ReceiveWidget::handleAddressClicked(const QModelIndex &index){
    QModelIndex rIndex = filter->mapToSource(index);
    refreshView(rIndex.data(Qt::DisplayRole).toString());
}

void ReceiveWidget::onLabelClicked(){
    if(walletModel) {
        window->showHide(true);
        // TODO: Open this to "update" the label if the address already has it.
        AddNewContactDialog *dialog = new AddNewContactDialog(window);
        if (openDialogWithOpaqueBackgroundY(dialog, window, 3.5, 6)) {
            QString label = dialog->getLabel();
            const CBitcoinAddress address = CBitcoinAddress(info->address.toUtf8().constData());
            if (!label.isEmpty() && walletModel->updateAddressBookLabels(
                    address.Get(),
                    label.toUtf8().constData(),
                    "receive"
            )
                    ) {
                // Show snackbar
                // update label status (icon color)
                updateLabel();
                window->messageInfo(tr("Address label saved"));
            } else {
                // Show snackbar error
                window->messageInfo(tr("Error storing address label"));
            }
        }
    }
}

void ReceiveWidget::onNewAddressClicked(){
    CBitcoinAddress address = walletModel->getNewAddress("");
    updateQr(QString::fromStdString(address.ToString()));
    ui->labelAddress->setText(!info->address.isEmpty() ? info->address : tr("No address"));
    updateLabel();
    window->messageInfo(tr("New address created"));
}

void ReceiveWidget::onCopyClicked(){
    GUIUtil::setClipboard(info->address);
    window->messageInfo(tr("Address copied"));
}


void ReceiveWidget::onRequestClicked(){
    if(walletModel) {
        window->showHide(true);
        RequestDialog *dialog = new RequestDialog(window);
        dialog->setWalletModel(walletModel);
        openDialogWithOpaqueBackgroundY(dialog, window, 3.5, 12);
    }
}

void ReceiveWidget::onMyAddressesClicked(){
    bool isVisible = ui->listViewAddress->isVisible();

    if(!isVisible){
        ui->listViewAddress->setVisible(true);
        ui->container_right->removeItem(spacer);
        ui->listViewAddress->update();
    }else{
        ui->container_right->addItem(spacer);
        ui->listViewAddress->setVisible(false);
    }
}

void ReceiveWidget::changeTheme(bool isLightTheme, QString& theme){
    // Change theme in all of the childs here..
    this->setStyleSheet(theme);
    static_cast<AddressHolder*>(this->delegate->getRowFactory())->isLightTheme = isLightTheme;
    updateStyle(this);
}

ReceiveWidget::~ReceiveWidget()
{
    delete ui;
}
