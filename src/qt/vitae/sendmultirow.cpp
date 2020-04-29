#include "qt/vitae/sendmultirow.h"
#include "qt/vitae/forms/ui_sendmultirow.h"
#include <QGraphicsDropShadowEffect>
#include <QDoubleValidator>

#include "optionsmodel.h"
#include "guiutil.h"
#include "bitcoinunits.h"
#include "qt/vitae/qtutils.h"

SendMultiRow::SendMultiRow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SendMultiRow),
    iconNumber(new QPushButton()),
    btnContact(new QPushButton())
{
    ui->setupUi(this);

    this->setStyleSheet(parent->styleSheet());

    QGraphicsDropShadowEffect* shadowEffect = new QGraphicsDropShadowEffect();
    shadowEffect->setColor(QColor(0, 0, 0, 22));
    shadowEffect->setXOffset(0);
    shadowEffect->setYOffset(3);
    shadowEffect->setBlurRadius(6);

    ui->lineEditAddress->setPlaceholderText("Add address");
    ui->lineEditAddress->setProperty("cssClass", "edit-primary-multi-book");
    ui->lineEditAddress->setAttribute(Qt::WA_MacShowFocusRect, 0);
    ui->stackedAddress->setGraphicsEffect(shadowEffect);

    ui->lineEditAmount->setPlaceholderText("0.00 zPIV ");
    setCssEditLine(ui->lineEditAmount, true, false);
    ui->lineEditAmount->setValidator(new QDoubleValidator(0, 100000000000, 7, this) );
    ui->lineEditAmount->setAttribute(Qt::WA_MacShowFocusRect, 0);
    ui->lineEditAmount->setGraphicsEffect(shadowEffect);

    /* Description */

    ui->labelSubtitleDescription->setText("Label address (optional)");
    ui->labelSubtitleDescription->setProperty("cssClass", "text-title");

    ui->lineEditDescription->setPlaceholderText("Add descripcion ");
    ui->lineEditDescription->setProperty("cssClass", "edit-primary");
    ui->lineEditDescription->setAttribute(Qt::WA_MacShowFocusRect, 0);
    ui->lineEditDescription->setGraphicsEffect(shadowEffect);

    // Button Contact

    btnContact->setProperty("cssClass", "btn-dropdown");
    btnContact->setCheckable(true);

    QSize BUTTON_CONTACT_SIZE = QSize(24, 24);
    btnContact->setMinimumSize(BUTTON_CONTACT_SIZE);
    btnContact->setMaximumSize(BUTTON_CONTACT_SIZE);

    ui->stackedAddress->addWidget(btnContact);

    btnContact->show();
    btnContact->raise();

    int posBtnXX = ui->lineEditAddress->width() - 20;
    int posBtnYY = 12;
    btnContact->move(posBtnXX, posBtnYY);


    // Icon Number

    ui->stackedAddress->addWidget(iconNumber);
    iconNumber->show();
    iconNumber->raise();

    iconNumber->setProperty("cssClass", "ic-multi-number");
    iconNumber->setText("1");
    iconNumber->setVisible(false);

    QSize BUTTON_SIZE = QSize(24, 24);
    iconNumber->setMinimumSize(BUTTON_SIZE);
    iconNumber->setMaximumSize(BUTTON_SIZE);

    int posIconX = 0;
    int posIconY = 14;
    iconNumber->move(posIconX, posIconY);

    // TODO: add validator --> there is a class in the core QValidateLineEdit and some methods..
    connect(ui->lineEditAmount, SIGNAL(textChanged(const QString&)), this, SLOT(amountChanged(const QString&)));
    connect(ui->lineEditAddress, SIGNAL(textChanged(const QString&)), this, SLOT(addressChanged(const QString&)));

}

void SendMultiRow::amountChanged(const QString& amount){
    CAmount value = getAmountValue(amount);
    if(value > 0){
        // BitcoinUnits::format(displayUnit, value, false, BitcoinUnits::separatorAlways);
        ui->lineEditAmount->setText(amount);
    }
}

/**
 * Returns -1 if the value is invalid
 */
CAmount SendMultiRow::getAmountValue(QString amount){
    bool isValid = false;
    CAmount value = GUIUtil::parseValue(amount, displayUnit, &isValid);
    return isValid ? value : -1;
}

bool SendMultiRow::addressChanged(const QString& str){
    QString trimmedStr = str.trimmed();
    bool valid = model->validateAddress(trimmedStr);
    if (!valid){
        ui->lineEditAddress->setProperty("cssClass","edit-primary-multi-book-error");
    }else{
        ui->lineEditAddress->setProperty("cssClass","edit-primary-multi-book");
    }
    updateStyle(ui->lineEditAddress);
    return valid;
}


void SendMultiRow::setModel(WalletModel* model) {
    this->model = model;

    // TODO:Complete me..
    if (model && model->getOptionsModel()) {
        displayUnit = model->getOptionsModel()->getDisplayUnit();
        //connect(model->getOptionsModel(), SIGNAL(displayUnitChanged(int)), this, SLOT(updateDisplayUnit()));
    }

    clear();
}

void SendMultiRow::deleteClicked() {
    emit removeEntry(this);
}

void SendMultiRow::clear() {
    ui->lineEditAddress->clear();
    ui->lineEditAmount->clear();
    ui->lineEditDescription->clear();
}

bool SendMultiRow::validate()
{
    if (!model)
        return false;

    // Check input validity
    bool retval = true;

    // Skip checks for payment request
    if (recipient.paymentRequest.IsInitialized())
        return retval;

    // Check address validity, returns false if it's invalid
    retval = addressChanged(ui->lineEditAddress->text());

    CAmount value = getAmountValue(ui->lineEditAmount->text());
    if(value == -1){
        retval = false;
    }

    // Sending a zero amount is invalid
    if (value <= 0) {
        setCssEditLine(ui->lineEditAmount, false, true);
        retval = false;
    }

    // Reject dust outputs:
    if (retval && GUIUtil::isDust(ui->lineEditAddress->text(), value)) {
        setCssEditLine(ui->lineEditAmount, false, true);
        retval = false;
    }

    return retval;
}

SendCoinsRecipient SendMultiRow::getValue() {
    // Payment request
    if (recipient.paymentRequest.IsInitialized())
        return recipient;

    // Normal payment
    QString addressStr = ui->lineEditAddress->text();
    QString trimmedStr = addressStr.trimmed();
    recipient.address = trimmedStr;
    recipient.label = ui->lineEditDescription->text();

    // TODO: Convert this into a value..
    CAmount value = getAmountValue(ui->lineEditAmount->text());
    if(value == -1){
        // Invalid value..
        // todo: Notificate user..
    }
    recipient.amount = value;
    //recipient.message = ui->messageTextLabel->text();

    return recipient;
}

void SendMultiRow::setAddress(const QString& address) {
    ui->lineEditAddress->setText(address);
    ui->lineEditAmount->setFocus();
}

bool SendMultiRow::isClear()
{
    return ui->lineEditAddress->text().isEmpty();
}

void SendMultiRow::setFocus()
{
    ui->lineEditAddress->setFocus();
}


void SendMultiRow::setNumber(int _number){
    iconNumber->setText(QString::number(_number));
}

void SendMultiRow::hideLabels(){
    ui->layoutLabel->setVisible(false);
    iconNumber->setVisible(true);
}

void SendMultiRow::showLabels(){
    ui->layoutLabel->setVisible(true);
    iconNumber->setVisible(false);
}

void SendMultiRow::resizeEvent(QResizeEvent *event)
 {
    int posXX = ui->lineEditAddress->width() - 20;
    int posYY = 12;
    btnContact->move(posXX, posYY);
    QWidget::resizeEvent(event);
 }

SendMultiRow::~SendMultiRow()
{
    delete ui;
}
