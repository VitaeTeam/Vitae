//
// Created by Kolby on 6/19/2019.
//


#include <startoptions.h>
#include <ui_startoptions.h>




StartOptions::StartOptions(QWidget *parent)
        : QWidget(parent), ui(new Ui::StartOptions)
        {
    ui->setupUi(this);


}

int StartOptions::getRows(){
    rows = 4;
    return rows;
};

StartOptions::~StartOptions() {
    delete ui;
}
