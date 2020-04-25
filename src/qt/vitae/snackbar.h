#ifndef SNACKBAR_H
#define SNACKBAR_H

#include <QDialog>

class PIVXGUI;

namespace Ui {
class SnackBar;
}

class SnackBar : public QDialog
{
    Q_OBJECT

public:
    explicit SnackBar(PIVXGUI* _window, QWidget *parent = nullptr);
    ~SnackBar();

    virtual void showEvent(QShowEvent *event) override;
    void sizeTo(QWidget *widget);
    void setText(QString text);
private slots:
    void hideAnim();
    void windowResizeEvent(QResizeEvent *event);
private:
    Ui::SnackBar *ui;
    PIVXGUI* window;
};

#endif // SNACKBAR_H
