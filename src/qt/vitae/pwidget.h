#ifndef PWIDGET_H
#define PWIDGET_H

#include <QObject>
#include <QWidget>

class VITAEGUI;

namespace Ui {
class PWidget;
}

class PWidget : public QWidget
{
    Q_OBJECT
public:
    explicit PWidget(VITAEGUI* _window = nullptr, QWidget *parent = nullptr);

protected slots:
    void changeTheme(bool isLightTheme, QString &theme);

private:
    VITAEGUI* window;

};

#endif // PWIDGET_H
