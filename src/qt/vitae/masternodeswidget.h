#ifndef MASTERNODESWIDGET_H
#define MASTERNODESWIDGET_H

#include <QWidget>
#include "qt/vitae/pwidget.h"
#include "qt/vitae/furabstractlistitemdelegate.h"
#include "qt/vitae/mnmodel.h"
#include "qt/vitae/tooltipmenu.h"

class VITAEGUI;

namespace Ui {
class MasterNodesWidget;
}

QT_BEGIN_NAMESPACE
class QModelIndex;
QT_END_NAMESPACE

class MasterNodesWidget : public PWidget
{
    Q_OBJECT

public:

    explicit MasterNodesWidget(VITAEGUI *parent = nullptr);
    ~MasterNodesWidget();

    void loadWalletModel();
private slots:
    void onCreateMNClicked();
    void changeTheme(bool isLightTheme, QString &theme);
    void onMNClicked(const QModelIndex &index);
    void onEditMNClicked();
    void onDeleteMNClicked();

private:
    Ui::MasterNodesWidget *ui;
    FurAbstractListItemDelegate *delegate;
    MNModel *mnModel = nullptr;
    TooltipMenu* menu = nullptr;
    QModelIndex index;


    void startAlias(QString strAlias);
    void updateListState();
};

#endif // MASTERNODESWIDGET_H
