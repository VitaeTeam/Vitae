#include "qt/vitae/settings/settingsmainoptionswidget.h"
#include "qt/vitae/settings/forms/ui_settingsmainoptionswidget.h"
#include "QListView"

#if defined(HAVE_CONFIG_H)
#include "config/vitae-config.h"
#endif

#include "bitcoinunits.h"
#include "guiutil.h"
#include "optionsmodel.h"
#include "qt/vitae/qtutils.h"

#include "main.h" // for MAX_SCRIPTCHECK_THREADS
#include "netbase.h"
#include "txdb.h" // for -dbcache defaults

#ifdef ENABLE_WALLET
#include "wallet.h" // for CWallet::minTxFee
#endif

#include <boost/thread.hpp>

#include <QDataWidgetMapper>
#include <QIntValidator>
#include <QLocale>
#include <QTimer>


SettingsMainOptionsWidget::SettingsMainOptionsWidget(VITAEGUI* _window, QWidget *parent) :
    PWidget(_window, parent),
    ui(new Ui::SettingsMainOptionsWidget)
{
    ui->setupUi(this);

    this->setStyleSheet(parent->styleSheet());

    // Containers
    ui->left->setProperty("cssClass", "container");
    ui->left->setContentsMargins(10,10,10,10);
    ui->labelDivider->setProperty("cssClass", "container-divider");

    // Title
    ui->labelTitle->setText(tr("Main"));
    ui->labelSubtitle1->setText("Customize the main application options");

    setCssTitleScreen(ui->labelTitle);
    setCssSubtitleScreen(ui->labelSubtitle1);
    setCssTitleScreen(ui->labelTitleDown);
    setCssSubtitleScreen(ui->labelSubtitleDown);

    ui->labelTitleSizeDb->setText(tr("Size of database cache"));
    ui->labelTitleSizeDb->setProperty("cssClass", "text-main-settings");

    ui->labelTitleThreads->setText(tr("Number of script verification threads"));
    ui->labelTitleThreads->setProperty("cssClass", "text-main-settings");

    // Switch
    ui->pushSwitchStart->setText(tr("Start PIVX on system login"));
    ui->pushSwitchStart->setProperty("cssClass", "btn-switch");

    // Combobox
    ui->databaseCache->setProperty("cssClass", "btn-spin-box");
    ui->databaseCache->setAttribute(Qt::WA_MacShowFocusRect, 0);
    setShadow(ui->databaseCache);
    ui->threadsScriptVerif->setProperty("cssClass", "btn-spin-box");
    ui->threadsScriptVerif->setAttribute(Qt::WA_MacShowFocusRect, 0);
    setShadow(ui->threadsScriptVerif);

    // CheckBox
    ui->checkBoxMinTaskbar->setText(tr("Minimize to they tray instead of the taskbar"));
    ui->checkBoxMinClose->setText(tr("Minimize on close"));

    // Buttons
    ui->pushButtonSave->setText(tr("SAVE"));
    ui->pushButtonReset->setText(tr("Reset to default"));
    setCssBtnPrimary(ui->pushButtonSave);
    setCssBtnSecondary(ui->pushButtonReset);

    /* Main elements init */
    ui->databaseCache->setMinimum(nMinDbCache);
    ui->databaseCache->setMaximum(nMaxDbCache);
    ui->threadsScriptVerif->setMinimum(-(int)boost::thread::hardware_concurrency());
    ui->threadsScriptVerif->setMaximum(MAX_SCRIPTCHECK_THREADS);

    connect(ui->pushButtonSave, SIGNAL(clicked()), parent, SLOT(onSaveOptionsClicked()));
}

void SettingsMainOptionsWidget::setMapper(QDataWidgetMapper *mapper){
    mapper->addMapping(ui->pushSwitchStart, OptionsModel::StartAtStartup);
    mapper->addMapping(ui->threadsScriptVerif, OptionsModel::ThreadsScriptVerif);
    mapper->addMapping(ui->databaseCache, OptionsModel::DatabaseCache);
    /* Window */
#ifndef Q_OS_MAC
    mapper->addMapping(ui->checkBoxMinTaskbar, OptionsModel::MinimizeToTray);
    mapper->addMapping(ui->checkBoxMinClose, OptionsModel::MinimizeOnClose);
#endif
}

SettingsMainOptionsWidget::~SettingsMainOptionsWidget()
{
    delete ui;
}
