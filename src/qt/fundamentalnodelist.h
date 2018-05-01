#ifndef FUNDAMENTALNODELIST_H
#define FUNDAMENTALNODELIST_H

#include "fundamentalnode.h"
#include "platformstyle.h"
#include "sync.h"
#include "util.h"

#include <QMenu>
#include <QTimer>
#include <QWidget>

#define MY_FUNDAMENTALNODELIST_UPDATE_SECONDS 60
#define FUNDAMENTALNODELIST_UPDATE_SECONDS 15
#define FUNDAMENTALNODELIST_FILTER_COOLDOWN_SECONDS 3

namespace Ui
{
class FundamentalnodeList;
}

class ClientModel;
class WalletModel;

QT_BEGIN_NAMESPACE
class QModelIndex;
QT_END_NAMESPACE

/** fundamentalnode Manager page widget */
class FundamentalnodeList : public QWidget
{
    Q_OBJECT

public:
    explicit FundamentalnodeList(QWidget* parent = 0);
    ~FundamentalnodeList();

    void setClientModel(ClientModel* clientModel);
    void setWalletModel(WalletModel* walletModel);
    void StartAlias(std::string strAlias);
    void StartAll(std::string strCommand = "start-all");

private:
    QMenu* contextMenu;
    int64_t nTimeFilterUpdated;
    bool fFilterUpdated;

public Q_SLOTS:
    void updateMyFundamentalnodeInfo(QString strAlias, QString strAddr, CFundamentalnode* pmn);
    void updateMyNodeList(bool fForce = false);

Q_SIGNALS:

private:
    QTimer* timer;
    Ui::FundamentalnodeList* ui;
    ClientModel* clientModel;
    WalletModel* walletModel;
    CCriticalSection cs_mnlistupdate;
    QString strCurrentFilter;

private Q_SLOTS:
    void showContextMenu(const QPoint&);
    void on_startButton_clicked();
    void on_startAllButton_clicked();
    void on_startMissingButton_clicked();
    void on_tableWidgetMyFundamentalnodes_itemSelectionChanged();
    void on_UpdateButton_clicked();
};
#endif // FUNDAMENTALNODELIST_H
