#ifndef TOOLTIPMENU_H
#define TOOLTIPMENU_H

#include "qt/vitae/pwidget.h"
#include <QWidget>
#include <QModelIndex>

class VITAEGUI;
class WalletModel;

namespace Ui {
class TooltipMenu;
}

QT_BEGIN_NAMESPACE
class QModelIndex;
QT_END_NAMESPACE

class TooltipMenu : public PWidget
{
    Q_OBJECT

public:
    explicit TooltipMenu(VITAEGUI* _window, QWidget *parent = nullptr);
    ~TooltipMenu() override;

    void setIndex(const QModelIndex &index);
    virtual void showEvent(QShowEvent *event) override;

    void setEditBtnText(QString btnText);
    void setCopyBtnVisible(bool visible);

signals:
    void onDeleteClicked();
    void onCopyClicked();
    void onEditClicked();

private slots:
    void deleteClicked();
    void copyClicked();
    void editClicked();

private:
    Ui::TooltipMenu *ui;
    QModelIndex index;
};

#endif // TOOLTIPMENU_H
