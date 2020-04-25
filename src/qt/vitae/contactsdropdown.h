#ifndef CONTACTSDROPDOWN_H
#define CONTACTSDROPDOWN_H

#include "addresstablemodel.h"
#include "qt/vitae/contactdropdownrow.h"
#include "qt/vitae/furabstractlistitemdelegate.h"

#include <QListView>
#include <QObject>
#include <QWidget>


class ContactsViewDelegate;
class ContViewHolder;
class WalletModel;


QT_BEGIN_NAMESPACE
class QModelIndex;
QT_END_NAMESPACE

class ContactsDropdown : public QWidget
{
    Q_OBJECT
public:
    explicit ContactsDropdown(int minWidth, int minHeight, QWidget *parent = nullptr);

    void resizeList(int minWidth, int mintHeight);
    void setWalletModel(WalletModel* _model);
private:
    FurAbstractListItemDelegate* delegate;
    AddressTableModel* model;
    QListView *list;
    QFrame *frameList;
private slots:
    void handleClick(const QModelIndex &index);
};

#endif // CONTACTSDROPDOWN_H