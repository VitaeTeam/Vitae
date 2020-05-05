#include "qt/vitae/optionbutton.h"
#include "qt/vitae/forms/ui_optionbutton.h"

#include <QMouseEvent>

OptionButton::OptionButton(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::OptionButton)
{
    ui->setupUi(this);

    ui->labelArrow3->setProperty("cssClass", "ic-arrow");

    ui->layoutOptions2->setProperty("cssClass", "container-options");
    ui->layoutOptions2->setContentsMargins(0,10,10,10);

    ui->labelCircle->setProperty("cssClass", "btn-options-indicator");
    connect(ui->labelArrow3, &QPushButton::clicked, [this](){setChecked(!ui->labelArrow3->isChecked());});
    setActive(false);
}

OptionButton::~OptionButton()
{
    delete ui;
}

void OptionButton::setTitleClassAndText(QString className, QString text){
    ui->labelTitleChange->setText(text);
    ui->labelTitleChange->setProperty("cssClass", className);
}

void OptionButton::setTitleText(QString text){
    ui->labelTitleChange->setText(text);
}

void OptionButton::setSubTitleClassAndText(QString className, QString text){
    ui->labelSubtitleChange->setText(text);
    ui->labelSubtitleChange->setProperty("cssClass", className);
}

void OptionButton::setRightIconClass(QString className){
    ui->labelArrow3->setProperty("cssClass", className);
}

void OptionButton::setRightIcon(QPixmap icon){
    //ui->labelArrow3->setPixmap(icon);
}

void OptionButton::setActive(bool isActive){
    if (isActive) {
        ui->layoutCircle->setVisible(true);
        ui->labelTitleChange->setProperty("cssClass", "btn-title-purple");
    } else {
        ui->layoutCircle->setVisible(false);
        ui->labelTitleChange->setProperty("cssClass", "btn-title-grey");
    }
}

void OptionButton::setChecked(bool checked){
    ui->labelArrow3->setChecked(checked);
    emit clicked();
}

void OptionButton::mousePressEvent(QMouseEvent *qevent){
    if (qevent->button() == Qt::LeftButton){
        setChecked(!ui->labelArrow3->isChecked());
    }
}
