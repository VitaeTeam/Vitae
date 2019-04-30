#ifndef ADDRESSESWIDGET_H
#define ADDRESSESWIDGET_H

#include "qt/vitae/pwidget.h"
#include "addresstablemodel.h"
#include "tooltipmenu.h"
#include "furabstractlistitemdelegate.h"
#include "qt/vitae/AddressFilterProxyModel.h"

#include <QWidget>

class AddressViewDelegate;
class TooltipMenu;
class VITAEGUI;
class WalletModel;

namespace Ui {
class AddressesWidget;
}

QT_BEGIN_NAMESPACE
class QModelIndex;
QT_END_NAMESPACE

class AddressesWidget : public PWidget
{
    Q_OBJECT

public:
    explicit AddressesWidget(VITAEGUI* _window, QWidget *parent = nullptr);
    ~AddressesWidget();

    void loadWalletModel() override;
    void onNewContactClicked();

private slots:
    void handleAddressClicked(const QModelIndex &index);
    void onStoreContactClicked();
    void onEditClicked();
    void onDeleteClicked();
    void onCopyClicked();

    void changeTheme(bool isLightTheme, QString &theme);
private:
    Ui::AddressesWidget *ui;

    FurAbstractListItemDelegate* delegate = nullptr;
    AddressTableModel* addressTablemodel = nullptr;
    AddressFilterProxyModel *filter = nullptr;

    bool isOnMyAddresses = true;
    TooltipMenu* menu = nullptr;

    // Cached index
    QModelIndex index;
};

#endif // ADDRESSESWIDGET_H
