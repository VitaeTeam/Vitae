#ifndef PRIVACYWIDGET_H
#define PRIVACYWIDGET_H

#include "qt/vitae/furabstractlistitemdelegate.h"
#include "transactiontablemodel.h"

#include <QLabel>
#include <QWidget>

class VITAEGUI;
class WalletModel;

namespace Ui {
class PrivacyWidget;
}

QT_BEGIN_NAMESPACE
class QModelIndex;
QT_END_NAMESPACE

class PrivacyWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PrivacyWidget(VITAEGUI* _window, QWidget *parent = nullptr);
    ~PrivacyWidget();

    void setWalletModel(WalletModel *_model);
private slots:
    void changeTheme(bool isLightTheme, QString &theme);
    void onCoinControlClicked();
    void onDenomClicked();
    void onRescanMintsClicked();
    void onResetZeroClicked();
    void onTotalZpivClicked();
private:
    Ui::PrivacyWidget *ui;
    VITAEGUI* window;
    FurAbstractListItemDelegate *delegate = nullptr;
    TransactionTableModel* model = nullptr;
};

#endif // PRIVACYWIDGET_H
