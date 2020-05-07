#ifndef RECEIVEWIDGET_H
#define RECEIVEWIDGET_H

#include "qt/vitae/pwidget.h"
#include "addresstablemodel.h"
#include "qt/vitae/furabstractlistitemdelegate.h"
#include "qt/vitae/AddressFilterProxyModel.h"

#include <QSpacerItem>
#include <QWidget>
#include <QPixmap>

class VITAEGUI;
class WalletModel;
class SendCoinsRecipient;

namespace Ui {
class ReceiveWidget;
}

QT_BEGIN_NAMESPACE
class QModelIndex;
QT_END_NAMESPACE

class ReceiveWidget : public PWidget
{
    Q_OBJECT

public:
    explicit ReceiveWidget(VITAEGUI* _window, QWidget *parent = nullptr);
    ~ReceiveWidget();

    void loadWalletModel() override;

public slots:
    void onRequestClicked();
    void onMyAddressesClicked();
    void onNewAddressClicked();
    
private slots:
    void changeTheme(bool isLightTheme, QString &theme) override ;
    void onLabelClicked();
    void onCopyClicked();
    void refreshView(QString refreshAddress = QString());
    void handleAddressClicked(const QModelIndex &index);
private:
    Ui::ReceiveWidget *ui;

    FurAbstractListItemDelegate *delegate;
    AddressTableModel* addressTableModel = nullptr;
    AddressFilterProxyModel *filter = nullptr;

    QSpacerItem *spacer = nullptr;

    // Cached last address
    SendCoinsRecipient *info = nullptr;
    // Cached qr
    QPixmap *qrImage = nullptr;

    void updateQr(QString address);
    void updateLabel();

};

#endif // RECEIVEWIDGET_H
