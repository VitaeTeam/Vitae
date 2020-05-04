#ifndef TOPBAR_H
#define TOPBAR_H

#include <QWidget>
#include "lockunlock.h"
#include "amount.h"
#include <QTimer>

class VITAEGUI;
class WalletModel;
class ClientModel;

namespace Ui {
class TopBar;
}

class TopBar : public QWidget
{
    Q_OBJECT

public:
    explicit TopBar(VITAEGUI* _mainWindow, QWidget *parent = nullptr);
    ~TopBar();

    void showTop();
    void showBottom();
    void showPasswordDialog();

    void setWalletModel(WalletModel *model);
    void setClientModel(ClientModel *model);

    void encryptWallet();
public slots:
    void updateBalances(const CAmount& balance, const CAmount& unconfirmedBalance, const CAmount& immatureBalance,
                        const CAmount& zerocoinBalance, const CAmount& unconfirmedZerocoinBalance, const CAmount& immatureZerocoinBalance,
                        const CAmount& watchOnlyBalance, const CAmount& watchUnconfBalance, const CAmount& watchImmatureBalance);
    void updateDisplayUnit();

    void setNumConnections(int count);
    void setNumBlocks(int count);
    void updateAutoMintStatus();
    void updateStakingStatus();

signals:
    // Fired when a message should be reported to the user
    void message(const QString& title, const QString& message, unsigned int style);

private slots:
    void onBtnReceiveClicked();
    void onThemeClicked();
    void onBtnLockClicked();
    void lockDropdownMouseLeave();
    void lockDropdownClicked(const StateClicked&);
    void refreshStatus();
private:
    Ui::TopBar *ui;
    VITAEGUI* mainWindow;
    LockUnlock *lockUnlockWidget = nullptr;
    WalletModel *walletModel;
    ClientModel *clientModel;

    int nDisplayUnit = -1;
    QTimer* timerStakingIcon = nullptr;

    QString formatBalance(CAmount amount, bool isZpiv = false);
};

#endif // TOPBAR_H
