#ifndef SEND_H
#define SEND_H

#include <QWidget>
#include <QPushButton>

#include "qt/vitae/contactsdropdown.h"
#include "qt/vitae/sendmultirow.h"
#include "walletmodel.h"

static const int MAX_SEND_POPUP_ENTRIES = 8;

class VITAEGUI;
class ClientModel;
class WalletModel;

namespace Ui {
class send;
class QPushButton;
}

class SendWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SendWidget(VITAEGUI* _window, QWidget *parent = nullptr);
    ~SendWidget();

    void addEntry();

    void setClientModel(ClientModel* clientModel);
    void setModel(WalletModel* model);

signals:
    void message(const QString& title, const QString& message, unsigned int style);

public slots:
    void onChangeAddressClicked();
    void onChangeCustomFeeClicked();
    void onCoinControlClicked();

protected:
    void resizeEvent(QResizeEvent *event);

private slots:
    void onPIVSelected();
    void onzPIVSelected();
    void onSendClicked();
    void changeTheme(bool isLightTheme, QString& theme);
    void onContactsClicked();
    void onAddEntryClicked();
    void clearEntries();
private:
    Ui::send *ui;
    VITAEGUI* window;
    QPushButton *coinIcon;
    QPushButton *btnContacts;

    ClientModel* clientModel = nullptr;
    WalletModel* walletModel = nullptr;

    QList<SendMultiRow*> entries;

    ContactsDropdown *menuContacts = nullptr;
    SendMultiRow *sendMultiRow;

    bool isPIV = true;
    void resizeMenu();
    SendMultiRow* createEntry();
    void send(QList<SendCoinsRecipient> recipients);
    void sendZpiv(QList<SendCoinsRecipient> recipients);

    // Process WalletModel::SendCoinsReturn and generate a pair consisting
    // of a message and message flags for use in emit message().
    // Additional parameter msgArg can be used via .arg(msgArg).
    void processSendCoinsReturn(const WalletModel::SendCoinsReturn& sendCoinsReturn, const QString& msgArg = QString(), bool fPrepare = false);
};

#endif // SEND_H
