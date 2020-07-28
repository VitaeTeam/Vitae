// Copyright (c) 2019 The PIVX developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SNACKBAR_H
#define SNACKBAR_H

#include <QDialog>

class VITAEGUI;

namespace Ui {
class SnackBar;
}

class SnackBar : public QDialog
{
    Q_OBJECT

public:
    explicit SnackBar(VITAEGUI* _window = nullptr, QWidget *parent = nullptr);
    ~SnackBar();

    virtual void showEvent(QShowEvent *event) override;
    void sizeTo(QWidget *widget);
    void setText(QString text);
private Q_SLOTS:
    void hideAnim();
    void windowResizeEvent(QResizeEvent *event);
private:
    Ui::SnackBar *ui;
    VITAEGUI* window = nullptr;
};

#endif // SNACKBAR_H
