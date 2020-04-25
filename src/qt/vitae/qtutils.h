#ifndef QTUTILS_H
#define QTUTILS_H

#include <QWidget>
#include <QDialog>
#include <QPropertyAnimation>
#include <QAbstractAnimation>
#include <QPoint>
#include <QString>
#include <QColor>
#include <QSettings>
#include <QStandardPaths>
#include "qt/vitae/VITAEGUI.h"


bool openDialog(QDialog *widget, VITAEGUI *gui);
void closeDialog(QDialog *widget, VITAEGUI *gui);
void openDialogFullScreen(QWidget *parent, QWidget * dialog);
bool openDialogWithOpaqueBackgroundY(QDialog *widget, VITAEGUI *gui, double posX = 3, int posY = 5);
bool openDialogWithOpaqueBackground(QDialog *widget, VITAEGUI *gui, double posX = 3);
bool openDialogWithOpaqueBackgroundFullScreen(QDialog *widget, VITAEGUI *gui);

inline void openSnackbar(QWidget *parent, VITAEGUI *gui, QString text){
    // TODO:Complete me..
    return;
}


// Helpers
void updateStyle(QWidget* widget);
QColor getRowColor(bool isLightTheme, bool isHovered, bool isSelected);

// Settings
QSettings* getSettings();
void setupSettings(QSettings *settings);

// Theme
QString getLightTheme();
QString getDarkTheme();

bool isLightTheme();
void setTheme(bool isLight);

#endif // QTUTILS_H