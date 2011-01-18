/****************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Nokia Corporation and its Subsidiary(-ies) nor
**     the names of its contributors may be used to endorse or promote
**     products derived from this software without specific prior written
**     permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef WINDOW_H
#define WINDOW_H

#include <QSystemTrayIcon>
#include <QDialog>
#include <QPixmap>
#include <QtCore/QSignalMapper>

QT_BEGIN_NAMESPACE
class QAction;
class QCheckBox;
class QComboBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class QMenu;
class QPushButton;
class QSpinBox;
class QTextEdit;
class QSlider;
class QTabWidget;
class QStandardItemModel;
class QPushButton;
class QSettings;

class SerialDeviceEnumerator;
class AbstractSerial;
class GlobalShortcutManager;

QT_END_NAMESPACE

//! [0]
class Window : public QDialog
{
    Q_OBJECT

public:
    Window();

    void setVisible(bool visible);

protected:
    void closeEvent(QCloseEvent *event);

private slots:
    void setIcon(int index);
    void iconActivated(QSystemTrayIcon::ActivationReason reason);
    void on_ambiligth();
    void off_ambiligth();
    void saveSettings();
    void saveColorSettings();
    void shootScreen();
    void backLight();
    void moodLamp();
    void changeMode();
    void slotPrintAllDevices(const QStringList &list);
    void zoneSaveSettings();
    void zoneRecalc();
    void switchAmbilight();

private:
    GlobalShortcutManager *hotkeyAmbilight;

    SerialDeviceEnumerator *m_sde;
    AbstractSerial *port;
    QByteArray *ba; //data to send

    bool onAmbiligth;

    QRect L,L1,L2,L3;
    QRect R,R1,R2,R3;
    QRect T,T1,T2;
    QRect B,B1,B2;

    QSettings *settings;



    int getData(int x, int y);

    void openPort();
    void closePort();

    void readSettings();
    void createHardwareGroupBox();
    void createColorGroupBox();
    void createAdvancedGroupBox();
    void createActions();
    void createTrayIcon();
    void createZoneTab();
    void updateScreenshotLabel();
    void GetPix(QRect rect,int brightness);
    int IntToByte(int i);
    void refreshAmbilightEvaluated(double updateResultMs);
    void writePort();
    void nextWork();


    QComboBox *modeComboBox;


    QWidget *zoneTabWidget;
    QStandardItemModel *model;
    QPushButton *saveZone;
    QPushButton *saveAdv;

    QTabWidget *tabWidget;

    QPixmap originalPixmap;

    QLabel *screenshotLabel;
    QLabel *refreshAmbiliht;

    QGroupBox *comGroupBox;
    QLabel *comLabel;
    QComboBox *comComboBox;


    QComboBox * brComboBox;
    QComboBox * dbComboBox;
    QComboBox * sbComboBox;
    QComboBox * pComboBox;
    QComboBox * fcComboBox;
    QCheckBox *rtsCheckBox;
    QCheckBox *dtrCheckBox;

    QGroupBox *advGroupBox;
    QComboBox *screenComboBox;
    QCheckBox *onAmbiligthCheckBox;
    QSpinBox *delaySpinBox;
    QSpinBox *channelSpinBox;
    QSpinBox *stepSpinBox;
    QCheckBox *viewZoneCheckBox;
    QCheckBox *viewGridCheckBox;
    QCheckBox *zoneDesktopCheckBox;

    QLabel *thresholdLabel;
    QLabel *brightnessLabel;
    QLabel *saturationLabel;
    QLabel *redLabel;
    QLabel *greenLabel;
    QLabel *blueLabel;

    QSlider *thresholdSlider;
    QSlider *brightnessSlider;
    QSlider *saturationSlider;
    QSlider *redSlider;
    QSlider *greenSlider;
    QSlider *blueSlider;

    QLineEdit *hotkeyLine;

    QGroupBox *colorGroupBox;

    QAction *onAction;
    QAction *offAction;
    QAction *settingsAction;
    QAction *quitAction;

    QSystemTrayIcon *trayIcon;
    QMenu *trayIconMenu;
};
//! [0]

#endif
