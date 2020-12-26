// Copyright (c) 2019-2020 The PIVX developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef FUNDAMENTALNODESWIDGET_H
#define FUNDAMENTALNODESWIDGET_H

#include <QWidget>
#include "qt/vitae/pwidget.h"
#include "qt/vitae/furabstractlistitemdelegate.h"
#include "qt/vitae/fnmodel.h"
#include "qt/vitae/tooltipmenu.h"
#include <QTimer>
#include <atomic>

class VITAEGUI;

namespace Ui {
class FundamentalNodesWidget;
}

QT_BEGIN_NAMESPACE
class QModelIndex;
QT_END_NAMESPACE

class FundamentalNodesWidget : public PWidget
{
    Q_OBJECT

public:

    explicit FundamentalNodesWidget(VITAEGUI *parent = nullptr);
    ~FundamentalNodesWidget();

    void loadWalletModel() override;

    void run(int type) override;
    void onError(QString error, int type) override;

    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;

private Q_SLOTS:
    void onCreateMNClicked();
    void onStartAllClicked(int type);
    void changeTheme(bool isLightTheme, QString &theme) override;
    void onMNClicked(const QModelIndex &index);
    void onEditMNClicked();
    void onDeleteMNClicked();
    void onInfoMNClicked();
    void updateListState();
    void updateModelAndInform(QString informText);

private:
    Ui::FundamentalNodesWidget *ui;
    FurAbstractListItemDelegate *delegate;
    FNModel *fnModel = nullptr;
    TooltipMenu* menu = nullptr;
    QModelIndex index;
    QTimer *timer = nullptr;

    std::atomic<bool> isLoading;

    bool checkMNsNetwork();
    void startAlias(QString strAlias);
    bool startAll(QString& failedMN, bool onlyMissing);
    bool startMN(CFundamentalnodeConfig::CFundamentalnodeEntry mne, std::string& strError);
};

#endif // FUNDAMENTALNODESWIDGET_H
