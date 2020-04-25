#ifndef SEND_H
#define SEND_H

#include <QWidget>
#include <QPushButton>

#include "qt/vitae/contactsdropdown.h"
#include "qt/vitae/sendmultirow.h"


class VITAEGUI;

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
private:
    Ui::send *ui;
    VITAEGUI* window;
    QPushButton *coinIcon;
    QPushButton *btnContacts;

    ContactsDropdown *menuContacts = nullptr;
    SendMultiRow *sendMultiRow;

    bool isPIV = true;
    void resizeMenu();
};

#endif // SEND_H
