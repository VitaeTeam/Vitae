#ifndef SETTINGSBACKUPWALLET_H
#define SETTINGSBACKUPWALLET_H

#include <QWidget>
#include "qt/vitae/pwidget.h"

namespace Ui {
class SettingsBackupWallet;
}

class SettingsBackupWallet : public PWidget
{
    Q_OBJECT

public:
    explicit SettingsBackupWallet(VITAEGUI* _window, QWidget *parent = nullptr);
    ~SettingsBackupWallet();

private slots:
    void backupWallet();
    void selectFileOutput();

private:
    Ui::SettingsBackupWallet *ui;
    QString filename;
};

#endif // SETTINGSBACKUPWALLET_H
