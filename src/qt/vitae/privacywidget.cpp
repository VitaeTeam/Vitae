#include "qt/pivx/privacywidget.h"
#include "qt/pivx/forms/ui_privacywidget.h"
#include <QFile>
#include "qt/pivx/qtutils.h"
#include "qt/pivx/coincontrolzpivdialog.h"
#include "qt/pivx/denomgenerationdialog.h"
#include <QGraphicsDropShadowEffect>
#include "qt/pivx/defaultdialog.h"
#include "qt/pivx/furlistrow.h"
#include "qt/pivx/txviewholder.h"
#include "walletmodel.h"

#define DECORATION_SIZE 70
#define NUM_ITEMS 3

PrivacyWidget::PrivacyWidget(PIVXGUI* _window, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PrivacyWidget),
    window(_window)
{
    ui->setupUi(this);

    this->setStyleSheet(_window->styleSheet());

    /* Containers */
    ui->left->setProperty("cssClass", "container");
    ui->left->setContentsMargins(0,20,0,20);
    ui->right->setProperty("cssClass", "container-right");
    ui->right->setContentsMargins(20,10,20,20);

    /* Light Font */
    QFont fontLight;
    fontLight.setWeight(QFont::Light);

    /* Title */
    ui->labelTitle->setText("Privacy");
    ui->labelTitle->setProperty("cssClass", "text-title-screen");
    ui->labelTitle->setFont(fontLight);

    /* Button Group */
    ui->pushLeft->setText("Convert");
    ui->pushLeft->setProperty("cssClass", "btn-check-left");
    ui->pushRight->setText("Mint");
    ui->pushRight->setProperty("cssClass", "btn-check-right");

    /* Subtitle */

    ui->labelSubtitle1->setText("Minting zPIV anonymizes your PIV by removing\ntransaction history, making transactions untraceable ");
    ui->labelSubtitle1->setProperty("cssClass", "text-subtitle");

    ui->labelSubtitle2->setText("Mint new zPIV or convert back to PIV");
    ui->labelSubtitle2->setProperty("cssClass", "text-subtitle");
    ui->labelSubtitle2->setContentsMargins(0,2,0,0);
    /* Amount */

    ui->labelSubtitleAmount->setText("Enter amount of PIV to mint into zPIV ");
    ui->labelSubtitleAmount->setProperty("cssClass", "text-title");

    QGraphicsDropShadowEffect* shadowEffect = new QGraphicsDropShadowEffect();
    shadowEffect->setColor(QColor(0, 0, 0, 22));
    shadowEffect->setXOffset(0);
    shadowEffect->setYOffset(3);
    shadowEffect->setBlurRadius(6);

    ui->lineEditAmount->setPlaceholderText("0.00 PIV ");
    ui->lineEditAmount->setProperty("cssClass", "edit-primary");
    ui->lineEditAmount->setAttribute(Qt::WA_MacShowFocusRect, 0);
    ui->lineEditAmount->setGraphicsEffect(shadowEffect);

    /* Denom */

    ui->labelTitleDenom1->setText("Denom. with value 1:");
    ui->labelTitleDenom1->setProperty("cssClass", "text-subtitle");
    ui->labelValueDenom1->setText("0x1 = 0 zPIV");
    ui->labelValueDenom1->setProperty("cssClass", "text-body2");

    ui->labelTitleDenom5->setText("Denom. with value 5:");
    ui->labelTitleDenom5->setProperty("cssClass", "text-subtitle");
    ui->labelValueDenom5->setText("0x5 = 0 zPIV");
    ui->labelValueDenom5->setProperty("cssClass", "text-body2");

    ui->labelTitleDenom10->setText("Denom. with value 10:");
    ui->labelTitleDenom10->setProperty("cssClass", "text-subtitle");
    ui->labelValueDenom10->setText("0x10 = 0 zPIV");
    ui->labelValueDenom10->setProperty("cssClass", "text-body2");

    ui->labelTitleDenom50->setText("Denom. with value 50:");
    ui->labelTitleDenom50->setProperty("cssClass", "text-subtitle");
    ui->labelValueDenom50->setText("0x50 = 0 zPIV");
    ui->labelValueDenom50->setProperty("cssClass", "text-body2");

    ui->labelTitleDenom100->setText("Denom. with value 100:");
    ui->labelTitleDenom100->setProperty("cssClass", "text-subtitle");
    ui->labelValueDenom100->setText("0x100 = 0 zPIV");
    ui->labelValueDenom100->setProperty("cssClass", "text-body2");

    ui->labelTitleDenom500->setText("Denom. with value 500:");
    ui->labelTitleDenom500->setProperty("cssClass", "text-subtitle");
    ui->labelValueDenom500->setText("0x500 = 0 zPIV");
    ui->labelValueDenom500->setProperty("cssClass", "text-body2");

    ui->labelTitleDenom1000->setText("Denom. with value 1000:");
    ui->labelTitleDenom1000->setProperty("cssClass", "text-subtitle");
    ui->labelValueDenom1000->setText("0x1000 = 0 zPIV");
    ui->labelValueDenom1000->setProperty("cssClass", "text-body2");


    ui->labelTitleDenom5000->setText("Denom. with value 5000:");
    ui->labelTitleDenom5000->setProperty("cssClass", "text-subtitle");
    ui->labelValueDenom5000->setText("0x5000 = 0 zPIV");
    ui->labelValueDenom5000->setProperty("cssClass", "text-body2");


    ui->layoutDenom->setVisible(false);



    // List


    ui->labelListHistory->setText("Last Mints");
    ui->labelListHistory->setProperty("cssClass", "text-title");

    ui->listView->setVisible(false);

    //ui->emptyContainer->setVisible(false);
    ui->pushImgEmpty->setProperty("cssClass", "img-empty-privacy");

    ui->labelEmpty->setText("No transactions yet");
    ui->labelEmpty->setProperty("cssClass", "text-empty");

    // Buttons

    ui->pushButtonSave->setText("Mint to zPIV");
    ui->pushButtonSave->setProperty("cssClass", "btn-primary");

    ui->btnTotalzPIV->setTitleClassAndText("btn-title-grey", "Total zPIV 1000");
    ui->btnTotalzPIV->setSubTitleClassAndText("text-subtitle", "Show own coins denominations.");
    ui->btnTotalzPIV->setRightIconClass("btn-dropdown");

    ui->btnCoinControl->setTitleClassAndText("btn-title-grey", "Coin Control");
    ui->btnCoinControl->setSubTitleClassAndText("text-subtitle", "Select PIV outputs to mint into zPIV.");

    ui->btnDenomGeneration->setTitleClassAndText("btn-title-grey", "Denom generation");
    ui->btnDenomGeneration->setSubTitleClassAndText("text-subtitle", "Select the denomination of the coins.");

    ui->btnRescanMints->setTitleClassAndText("btn-title-grey", "Rescan mints");
    ui->btnRescanMints->setSubTitleClassAndText("text-subtitle", "Find mints in the blockchain.");

    ui->btnResetZerocoin->setTitleClassAndText("btn-title-grey", "Reset Zerocoin");
    ui->btnResetZerocoin->setSubTitleClassAndText("text-subtitle", "Reset zerocoin database.");


    connect(ui->btnTotalzPIV, SIGNAL(clicked()), this, SLOT(onTotalZpivClicked()));
    connect(ui->btnCoinControl, SIGNAL(clicked()), this, SLOT(onCoinControlClicked()));
    connect(ui->btnDenomGeneration, SIGNAL(clicked()), this, SLOT(onDenomClicked()));
    connect(ui->btnRescanMints, SIGNAL(clicked()), this, SLOT(onRescanMintsClicked()));
    connect(ui->btnResetZerocoin, SIGNAL(clicked()), this, SLOT(onResetZeroClicked()));

    // Style
    connect(window, SIGNAL(themeChanged(bool, QString&)), this, SLOT(changeTheme(bool, QString&)));


    // List
    ui->listView->setProperty("cssClass", "container");
    delegate = new FurAbstractListItemDelegate(
                DECORATION_SIZE,
                new TxViewHolder(isLightTheme()),
                this
                );

    ui->listView->setItemDelegate(delegate);
    ui->listView->setIconSize(QSize(DECORATION_SIZE, DECORATION_SIZE));
    ui->listView->setMinimumHeight(NUM_ITEMS * (DECORATION_SIZE + 2));
    ui->listView->setAttribute(Qt::WA_MacShowFocusRect, false);
    ui->listView->setSelectionBehavior(QAbstractItemView::SelectRows);

}

void PrivacyWidget::setWalletModel(WalletModel* _model){
    model = _model->getTransactionTableModel();
    ui->listView->setModel(this->model);
}


void PrivacyWidget::onTotalZpivClicked(){

    bool isVisible = ui->layoutDenom->isVisible();

    if(!isVisible){
        ui->layoutDenom->setVisible(true);
    }else{
        ui->layoutDenom->setVisible(false);
    }

}


void PrivacyWidget::onCoinControlClicked(){
    window->showHide(true);
    CoinControlZpivDialog* dialog = new CoinControlZpivDialog(window);
    openDialogWithOpaqueBackgroundY(dialog, window, 4.5, 5);
}

void PrivacyWidget::onDenomClicked(){
    window->showHide(true);
    DenomGenerationDialog* dialog = new DenomGenerationDialog(window);
    openDialogWithOpaqueBackgroundY(dialog, window, 4.5, 5);
}

void PrivacyWidget::onRescanMintsClicked(){
    window->showHide(true);
    DefaultDialog* dialog = new DefaultDialog(window);
    openDialogWithOpaqueBackgroundY(dialog, window, 4.5, 5);
}

void PrivacyWidget::onResetZeroClicked(){
    window->showHide(true);
    DefaultDialog* dialog = new DefaultDialog(window);
    openDialogWithOpaqueBackgroundY(dialog, window, 4.5, 5);
}

void PrivacyWidget::changeTheme(bool isLightTheme, QString& theme){
    // Change theme in all of the childs here..
    this->setStyleSheet(theme);
    static_cast<TxViewHolder*>(this->delegate->getRowFactory())->isLightTheme = isLightTheme;
    ui->listView->update();
    updateStyle(this);
}

PrivacyWidget::~PrivacyWidget()
{
    delete ui;
}
