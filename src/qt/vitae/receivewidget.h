#ifndef RECEIVEWIDGET_H
#define RECEIVEWIDGET_H

#include "addresstablemodel.h"
#include "qt/vitae/furabstractlistitemdelegate.h"

#include <QSpacerItem>
#include <QWidget>

class VITAEGUI;
class WalletModel;

namespace Ui {
class ReceiveWidget;
}

QT_BEGIN_NAMESPACE
class QModelIndex;
QT_END_NAMESPACE

class ReceiveWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ReceiveWidget(VITAEGUI* _window, QWidget *parent = nullptr);
    ~ReceiveWidget();

    void setWalletModel(WalletModel* model);

public slots:
    void onRequestClicked();
    void onMyAddressesClicked();
    
private slots:
    void changeTheme(bool isLightTheme, QString &theme);
    void onLabelClicked();
private:
    Ui::ReceiveWidget *ui;
    VITAEGUI* window;

    FurAbstractListItemDelegate *delegate;
    AddressTableModel* addressTableModel;

    QSpacerItem *spacer = nullptr;

};

#endif // RECEIVEWIDGET_H
