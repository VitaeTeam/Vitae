#ifndef PWIDGET_H
#define PWIDGET_H

#include <QObject>
#include <QWidget>

class VITAEGUI;
class ClientModel;

namespace Ui {
class PWidget;
}

class PWidget : public QWidget
{
    Q_OBJECT
public:
    explicit PWidget(VITAEGUI* _window = nullptr, QWidget *parent = nullptr);

    void setClientModel(ClientModel* model);

signals:
    void message(QString &message);

protected slots:
    void changeTheme(bool isLightTheme, QString &theme);

protected:
    ClientModel* clientModel;

    virtual void loadClientModel();

private:
    VITAEGUI* window;

};

#endif // PWIDGET_H
