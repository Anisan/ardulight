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

#include <QtGui>
#include <QtCore/QStringList>
#include <QtCore/qmath.h>
 #include <QScreen>
#include "window.h"
#include "serialdeviceenumerator.h"
#include <abstractserial.h>
//#include <qxtglobalshortcut.h>
#include "globalshortcutmanager.h"

#include "delegate.h"
#include "version.h"


//! [0]

#define COUNT_AREA 14

Window::Window()
{
    qsrand(QTime(0,0,0).secsTo(QTime::currentTime()));

    // 1.0.6 onTop ���� ��������
    this->setWindowFlags(Qt::WindowStaysOnTopHint | Qt::Window);

    port = new AbstractSerial();
    settings = new QSettings("settings.ini",QSettings::IniFormat);
    qDebug() << "Settings file: " << settings->fileName();
    tabWidget = new QTabWidget;

    createHardwareGroupBox();
    createColorGroupBox();
    createAdvancedGroupBox();
    createTrayIcon();

   QGroupBox *previewGroupBox = new QGroupBox(tr("Preview"));
    screenshotLabel = new QLabel;
    screenshotLabel->setSizePolicy(QSizePolicy::Expanding,
                                   QSizePolicy::Expanding);
    screenshotLabel->setAlignment(Qt::AlignCenter);
    screenshotLabel->setMinimumSize(240, 160);

    QHBoxLayout *aLayout = new QHBoxLayout;
    aLayout->addWidget(screenshotLabel);
    previewGroupBox->setLayout(aLayout);

    QGroupBox *modeGroupBox = new QGroupBox(tr("Mode"));
    modeComboBox = new QComboBox;
    modeComboBox->addItem(QIcon(":/images/ambilight_icon.png"),tr("Ambilight"));
    modeComboBox->addItem(QIcon(":/images/backlight.png"),tr("Backlight"));
    modeComboBox->addItem(QIcon(":/images/moodlamp.png"),tr("Mood lamp"));

    QHBoxLayout *mLayout = new QHBoxLayout;
    mLayout->addWidget(modeComboBox);
    modeGroupBox->setLayout(mLayout);



    QWidget *colorTabWidget = new QWidget;
    QVBoxLayout *colorLayout = new QVBoxLayout(colorTabWidget);
    colorLayout->addWidget(previewGroupBox);
    colorLayout->addWidget(modeGroupBox);
    colorLayout->addWidget(colorGroupBox);

     QWidget *hardTabWidget = new QWidget;
     QVBoxLayout *hardLayout = new QVBoxLayout(hardTabWidget);
     hardLayout->addWidget(comGroupBox);
     hardLayout->addWidget(advGroupBox);
     saveAdv = new QPushButton(tr("Apply"));
     hardLayout->addWidget(saveAdv);
     QString about = QString("%1 v.%2").arg(tr("Ambilight - 2011 - Eraser Soft")).arg(VERSION_STR);
     hardLayout->addWidget(new QLabel(about),0,Qt::AlignCenter);

     zoneTabWidget = new QWidget;
     createZoneTab();

    tabWidget->addTab(colorTabWidget, tr("Main"));
    tabWidget->addTab(zoneTabWidget, tr("Area"));
    tabWidget->addTab(hardTabWidget, tr("Advanced"));

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(tabWidget);

    setLayout(mainLayout);

    setIcon(1);


    setWindowTitle(tr("Ambilight"));
    setWindowIcon(QIcon(":/images/ambilight_icon.ico"));
    resize(400, 300);

    readSettings();


    //v 1.0.7  global shirtcuts on/off
   QString SHORTCUT_AMB = hotkeyLine->text();
   //v 1.1.2  global shirtcuts mode
   QString SHORTCUT_MODE = hotkeyLineMode->text();
    hotkeyAmbilight->connect(QKeySequence(SHORTCUT_AMB),this,SLOT(switchAmbilight()));
    hotkeyMode->connect(QKeySequence(SHORTCUT_MODE),this,SLOT(switchMode()));

    onAction->setEnabled(!onAmbiligth);
    offAction->setEnabled(onAmbiligth);
    if (onAmbiligth)
    {
        setIcon(2);
        openPort();
    }
    else
        setIcon(1);

    connect(saveZone, SIGNAL(clicked()), this, SLOT(zoneSaveSettings()));

    connect(brightnessSlider,SIGNAL(sliderReleased()),this,SLOT(saveColorSettings()));
    connect(saturationSlider,SIGNAL(sliderReleased()),this,SLOT(saveColorSettings()));
    connect(redSlider,SIGNAL(sliderReleased()),this,SLOT(saveColorSettings()));
    connect(greenSlider,SIGNAL(sliderReleased()),this,SLOT(saveColorSettings()));
    connect(blueSlider,SIGNAL(sliderReleased()),this,SLOT(saveColorSettings()));
    connect(thresholdSlider,SIGNAL(sliderReleased()),this,SLOT(saveColorSettings()));
    connect(modeComboBox,SIGNAL(currentIndexChanged(int)),this,SLOT(saveColorSettingsFF()));

    connect(modeComboBox,SIGNAL(currentIndexChanged(int)),this,SLOT(changeMode()));


    connect(saveAdv, SIGNAL(clicked()), this, SLOT(saveSettings()));

    connect(screenComboBox,SIGNAL(currentIndexChanged(int)),this,SLOT(zoneRecalc()));

    connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
            this, SLOT(iconActivated(QSystemTrayIcon::ActivationReason)));


     trayIcon->show();


      originalPixmap = QPixmap();
     shootScreen();
}
//! [0]

void Window::readSettings()
{
    qDebug() << " load settings ";
   // QSettings settings("EraserSoft", "Ambiligth");

    modeComboBox->setCurrentIndex( settings->value("WorkMode",0).toInt());
     //v 1.1.2 auto start mode
    modeStartComboBox->setCurrentIndex( settings->value("StartMode",0).toInt());
    if (modeStartComboBox->currentIndex()<3)
       modeComboBox->setCurrentIndex(modeStartComboBox->currentIndex());
    changeMode();

   brightnessSlider->setValue(settings->value("Brightness", 192).toInt());
   saturationSlider->setValue(settings->value("Saturation", 96).toInt());
   redSlider->setValue(settings->value("Red", 128).toInt());
   greenSlider->setValue(settings->value("Green", 128).toInt());
   blueSlider->setValue(settings->value("Blue", 128).toInt());

   thresholdSlider->setValue(settings->value("Threshold", 0).toInt());

   onAmbiligth = settings->value("onAmbilight",false).toBool();
   onAmbiligthCheckBox->setChecked(settings->value("onAmbilight",false).toBool());
   delaySpinBox->setValue(settings->value("Delay",40).toInt());
   channelSpinBox->setValue(settings->value("Channels",24).toInt());
   viewZoneCheckBox->setChecked(settings->value("ViewZoneOnPreview",false).toBool());

   viewGridCheckBox->setChecked(settings->value("ViewGridOnPreview",false).toBool());
   stepSpinBox->setValue(settings->value("StepGrid",5).toInt());

   hotkeyLine->setText(settings->value("Hotkey","Ctrl+Alt+F").toString());
   hotkeyLineMode->setText(settings->value("HotkeyMode","Ctrl+Alt+M").toString());

   ba = new QByteArray;
   ba->clear();

   for (int row = 0; row < COUNT_AREA; ++row) {
       for (int column = 0; column < 4; ++column) {
           QModelIndex index = model->index(row, column, QModelIndex());
           model->setData(index, settings->value(QString("Zone_%1_%2").arg(row).arg(column),0).toInt() );
       }
       QModelIndex index = model->index(row, 4, QModelIndex());
       model->setData(index, settings->value(QString("Zone_%1_Brightness").arg(row),100).toInt() );
   }

   int ind = comComboBox->findText(settings->value("ComPort",comComboBox->currentText()).toString());
   comComboBox->setCurrentIndex(ind);

   ind = brComboBox->findText(settings->value("BaudRate",brComboBox->currentText()).toString());
   brComboBox->setCurrentIndex(ind);
   ind = dbComboBox->findText(settings->value("DataBits",dbComboBox->currentText()).toString());
   dbComboBox->setCurrentIndex(ind);
   ind = sbComboBox->findText(settings->value("StopBits",sbComboBox->currentText()).toString());
   sbComboBox->setCurrentIndex(ind);
   ind = pComboBox->findText(settings->value("Parity",pComboBox->currentText()).toString());
   pComboBox->setCurrentIndex(ind);
   ind = fcComboBox->findText(settings->value("FlowControl",fcComboBox->currentText()).toString());
   fcComboBox->setCurrentIndex(ind);

   ind = screenComboBox->findText(settings->value("Screen",screenComboBox->currentText()).toString());
   screenComboBox->setCurrentIndex(ind);

   dtrCheckBox->setChecked(settings->value("DTR",false).toBool());
   rtsCheckBox->setChecked(settings->value("RTS",false).toBool());


    zoneRecalc();

 qDebug() << "end load settings ";
}

QRect screenresOld;
void Window::zoneRecalc()
{
      int screen = screenComboBox->currentIndex();
    QRect screenres = QApplication::desktop()->screenGeometry(screen);
    screenresOld = screenres ;
    float W = screenres.width() / 100.0;
    float H =screenres.height() / 100.0;

    R = QRect(W*getData(0,0),H*getData(0,1),W*getData(0,2),H*getData(0,3));
    L = QRect(W*getData(1,0),H*getData(1,1),W*getData(1,2),H*getData(1,3));
    T = QRect(W*getData(2,0),H*getData(2,1),W*getData(2,2),H*getData(2,3));
    B = QRect(W*getData(3,0),H*getData(3,1),W*getData(3,2),H*getData(3,3));
    R1 = QRect(W*getData(4,0),H*getData(4,1),W*getData(4,2),H*getData(4,3));
    L1 = QRect(W*getData(5,0),H*getData(5,1),W*getData(5,2),H*getData(5,3));
    T1 = QRect(W*getData(6,0),H*getData(6,1),W*getData(6,2),H*getData(6,3));
    B1 = QRect(W*getData(7,0),H*getData(7,1),W*getData(7,2),H*getData(7,3));
    R2 = QRect(W*getData(8,0),H*getData(8,1),W*getData(8,2),H*getData(8,3));
    L2 = QRect(W*getData(9,0),H*getData(9,1),W*getData(9,2),H*getData(9,3));
    T2 = QRect(W*getData(10,0),H*getData(10,1),W*getData(10,2),H*getData(10,3));
    B2 = QRect(W*getData(11,0),H*getData(11,1),W*getData(11,2),H*getData(11,3));
    R3 = QRect(W*getData(12,0),H*getData(12,1),W*getData(12,2),H*getData(12,3));
    L3 = QRect(W*getData(13,0),H*getData(13,1),W*getData(13,2),H*getData(13,3));
}

int Window::getData(int x, int y)
{
    QModelIndex index = model->index(x, y, QModelIndex());
    return index.data().toInt();
}

void Window::zoneSaveSettings()
{
     qDebug() << " save zone settings ";

    for (int row = 0; row < COUNT_AREA; ++row) {
        for (int column = 0; column < 4; ++column) {
            QModelIndex index = model->index(row, column, QModelIndex());
            settings->setValue(QString("Zone_%1_%2").arg(row).arg(column), index.data());
        }
        QModelIndex index = model->index(row, 4, QModelIndex());
        settings->setValue(QString("Zone_%1_Brightness").arg(row), index.data());
    }
    settings->setValue("ViewZoneOnPreview", viewZoneCheckBox->isChecked());
    settings->setValue("ViewGridOnPreview", viewGridCheckBox->isChecked());
    settings->setValue("StepGrid",stepSpinBox->value());

    zoneRecalc();
}


void Window::saveSettings()
{
     qDebug() << " save settings";
    settings->setValue("onAmbilight", onAmbiligthCheckBox->isChecked());
    settings->setValue("StartMode", modeStartComboBox->currentIndex());
    settings->setValue("Screen",screenComboBox->currentText());
    settings->setValue("Delay",delaySpinBox->value());
    settings->setValue("Channels",channelSpinBox->value());

    settings->setValue("Hotkey",hotkeyLine->text());
    settings->setValue("HotkeyMode",hotkeyLineMode->text());

    settings->setValue("ComPort",comComboBox->currentText());
    settings->setValue("BaudRate",brComboBox->currentText());
    settings->setValue("DataBits",dbComboBox->currentText());
    settings->setValue("StopBits",sbComboBox->currentText());
    settings->setValue("Parity",pComboBox->currentText());
    settings->setValue("FlowControl",fcComboBox->currentText());
    settings->setValue("DTR", dtrCheckBox->isChecked());
    settings->setValue("RTS", rtsCheckBox->isChecked());


}

void Window::saveColorSettings()
{
    qDebug() << " save color settings";
    settings->setValue("WorkMode", modeComboBox->currentIndex());
    settings->setValue("Brightness", brightnessSlider->value());
    settings->setValue("Saturation", saturationSlider->value());
    settings->setValue("Red", redSlider->value());
    settings->setValue("Green", greenSlider->value());
    settings->setValue("Blue", blueSlider->value());
    settings->setValue("Threshold", thresholdSlider->value());
}
//*******************************************************************
void Window::openPort()
{
        if (port->isOpen())
        {
            return;
        }

port->setDeviceName(comComboBox->currentText());

if (port->open(AbstractSerial::WriteOnly)) {

        qDebug() << "Serial device " << port->deviceName() << " open in " << port->openMode();

        qDebug() << "= Default parameters =";
        qDebug() << "Device name            : " << port->deviceName();
        qDebug() << "Baud rate              : " << port->baudRate();
        qDebug() << "Data bits              : " << port->dataBits();
        qDebug() << "Parity                 : " << port->parity();
        qDebug() << "Stop bits              : " << port->stopBits();
        qDebug() << "Flow                   : " << port->flowControl();
        qDebug() << "Char timeout, msec     : " << port->charIntervalTimeout();

        AbstractSerial::BaudRate baud;
        switch (brComboBox->currentIndex())
        {
            case 0: baud = AbstractSerial::BaudRate300; break;
            case 1: baud = AbstractSerial::BaudRate600; break;
            case 2: baud = AbstractSerial::BaudRate1200; break;
            case 3: baud = AbstractSerial::BaudRate2400; break;
            case 4: baud = AbstractSerial::BaudRate4800; break;
            case 5: baud = AbstractSerial::BaudRate9600; break;
            case 6: baud = AbstractSerial::BaudRate19200; break;
            case 7: baud = AbstractSerial::BaudRate38400; break;
            case 8: baud = AbstractSerial::BaudRate57600; break;
            case 9: baud = AbstractSerial::BaudRate115200; break;
            case 10: baud = AbstractSerial::BaudRate230400; break;
            case 11: baud = AbstractSerial::BaudRate460800; break;
            case 12: baud = AbstractSerial::BaudRate921600; break;
        }
        if (!port->setBaudRate(baud)) {
            qDebug() << "Set baud rate " <<  baud << " error.";
        };

        AbstractSerial::DataBits dataBits;
                    switch (dbComboBox->currentIndex())
                    {
                    case 0: dataBits = AbstractSerial::DataBits7; break;
                    case 1: dataBits = AbstractSerial::DataBits8; break;
                        }
        if (!port->setDataBits(dataBits)) {
            qDebug() << "Set data bits " << dataBits << " error.";
        }

        AbstractSerial::Parity par;
        switch (pComboBox->currentIndex())
        {
        case 0: par = AbstractSerial::ParityNone; break;
        case 1: par = AbstractSerial::ParityOdd; break;
        case 2: par = AbstractSerial::ParityEven; break;
        case 3: par = AbstractSerial::ParityMark; break;
        case 4: par = AbstractSerial::ParitySpace; break;
            }
        if (!port->setParity(par)) {
            qDebug() << "Set parity " << par << " error.";
        }

        AbstractSerial::StopBits stopBits;
                    switch (sbComboBox->currentIndex())
                    {
                    case 0: stopBits = AbstractSerial::StopBits1; break;
                    case 1: stopBits = AbstractSerial::StopBits2; break;
                        }
        if (!port->setStopBits(stopBits)) {
            qDebug() << "Set stop bits " <<  stopBits << " error.";
        }

        QString flowControl;
                    switch (fcComboBox->currentIndex())
                    {
                    case 0: flowControl = AbstractSerial::FlowControlOff; break;
                    case 1: flowControl = AbstractSerial::FlowControlHardware; break;
                    case 2: flowControl = AbstractSerial::FlowControlXonXoff; break;
                        }
        if (!port->setFlowControl(flowControl)) {
            qDebug() << "Set flow " <<  flowControl << " error.";
        }

        port->setDtr(dtrCheckBox->isChecked());
        port->setRts(rtsCheckBox->isChecked());


        qDebug() << "= New parameters =";
        qDebug() << "Device name            : " << port->deviceName();
        qDebug() << "Baud rate              : " << port->baudRate();
        qDebug() << "Data bits              : " << port->dataBits();
        qDebug() << "Parity                 : " << port->parity();
        qDebug() << "Stop bits              : " << port->stopBits();
        qDebug() << "Flow                   : " << port->flowControl();
        qDebug() << "Char timeout, msec     : " << port->charIntervalTimeout();

        //v1.0.4 �������� ��� ������������ NANO
        //������������ Nano ���������� ������ ��� ��� ����������� � ���������
       QEventLoop loop;
       QTimer::singleShot(1000, &loop, SLOT(quit())); loop.exec();


    }
    else {
        qDebug() << "Error opened serial device " << port->deviceName();
        setIcon(3);
    }
}

void Window::closePort()
{
        if (!port->isOpen())
        {
            port->close();
        }
}

//*******************************************************************
int Window::IntToByte(int i)
{
   if (i > 255) return 255;
     else
         if (i < thresholdSlider->value()) return 0;
         else
         return i;
}

void Window::GetPix(QRect rect, int brightness)
{
  int Red=0;
  int Green=0;
  int Blue=0;

  //int screen = screenComboBox->currentIndex();
 // QImage img = QPixmap::grabWindow(QApplication::desktop()->screen(screen) ->winId(),rect.x(),rect.y(),rect.x()+rect.width(),rect.y()+rect.height()).toImage();
//  QImage img = originalPixmap.copy(rect).toImage();

//  for (int y=0; y< rect.height();y++){
//      for (int x=0; x<rect.width(); x++){
//          QColor pix = img.pixel(x,y);
//          Red += pix.red();
//          Green += pix.green();         //��������� ��� �������
//          Blue += pix.blue();
//       }
//  }
//  int z =rect.width()*rect.height();
//  if (z==0)
//  {
//      ba->append(char(0));
//      ba->append(char(0)); //��������� � �����
//      ba->append(char(0));
//      return;
//  }
//  Red=Red/z;
//  Green=Green/ z;   //�������� ������� �������� ��� ������� ������
//  Blue=Blue/z;

  // �� ����� ������� �������

   QPixmap img = originalPixmap.copy(rect);
   QPixmap scaledPix = img.scaled(1,1, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
   QImage im = scaledPix.toImage();
   QColor pix = im.pixel(0,0);
   Red = pix.red();
   Green = pix.green();
   Blue = pix.blue();

   int Average_Pix= qFloor((Red+Green+Blue)/ 3.0); //�������
   int TrackBar_Sol = saturationSlider->value();
   Red=(Red+((Red / 128.0) * TrackBar_Sol));
   Green=(Green+((Green / 128.0) * TrackBar_Sol));  // ������������
   Blue=(Blue+((Blue / 128.0) * TrackBar_Sol));
   int  Aver=qFloor((Red + Green + Blue)/ 3.0)- Average_Pix; //������� � ������ ����� ��������������

   int TrackBar_Red = redSlider->value();
   int TrackBar_Green = greenSlider->value();
   int TrackBar_Blue = blueSlider->value();

   Red=(Red*( TrackBar_Red / 128.0))-Aver;
   Green=(Green*( TrackBar_Green / 128.0))-Aver;
   Blue=(Blue*( TrackBar_Blue / 128.0))-Aver;

   // v1.0.6 ������� �� �����
  int TrackBar_Light =255-brightnessSlider->value()/100.0*brightness;
  Red=IntToByte(Red-(qFloor(Red*TrackBar_Light)/ 255.0));
  Green=IntToByte(Green-(qFloor(Green*TrackBar_Light)/ 255.0));//�������
  Blue=IntToByte(Blue-(qFloor(Blue*TrackBar_Light)/ 255.0));
  ba->append(Red);
  ba->append(Green); //��������� � �����
  ba->append(Blue);

  // ������ ����
     if (!this->isHidden())
          if (viewZoneCheckBox->isChecked())
              {
                  QPainter p(&originalPixmap);
                  p.fillRect(rect,QColor(Red,Green,Blue));
              }
}


////********************************************************
void Window::shootScreen()
{
    //todo ����� ��� ��������
 //  qDebug() << QApplication::desktop()->screenCount();
 //  qDebug() << QApplication::desktop()->screenNumber(this);
 //   qDebug() << QApplication::desktop()->primaryScreen();
 //   qDebug() << QApplication::desktop()->isVirtualDesktop();

    QTime t;
    t.start();

    //���������� ��� �����
   // originalPixmap = QPixmap(); // clear image for low memory situations on embedded devices.

    //����� �����
    int screen = screenComboBox->currentIndex();
    QRect screenres = QApplication::desktop()->screenGeometry(screen);
    // v1.0.5 fix multimonitor
    originalPixmap = QPixmap::grabWindow(QApplication::desktop()->screen(screen) ->winId(),
                                         screenres.x(), //!
                                         screenres.y(), //!
                                         screenres.width(),
                                         screenres.height());

    // v1.0.8 �������� �������� ��� ��� ����� ����������
    if (screenresOld!=screenres)
          zoneRecalc();


    int channels = channelSpinBox->value();
    ba->clear();
    ba->append(255); // �������
              GetPix(R,getData(0,4));
          if (channels>3) GetPix(L,getData(1,4));
          if (channels>6) GetPix(T,getData(2,4));
          if (channels>9) GetPix(B,getData(3,4));

          if (channels>12) GetPix(R1,getData(4,4));
          if (channels>15) GetPix(L1,getData(5,4));
          if (channels>18) GetPix(T1,getData(6,4));
          if (channels>21) GetPix(B1,getData(7,4));

          if (channels>24) GetPix(R2,getData(8,4));
          if (channels>27) GetPix(L2,getData(9,4));
          if (channels>30) GetPix(T2,getData(10,4));
          if (channels>33) GetPix(B2,getData(11,4));

          if (channels>36) GetPix(R3,getData(12,4));
          if (channels>39) GetPix(L3,getData(13,4));

        writePort();

   refreshAmbilightEvaluated(t.elapsed());

   updateScreenshotLabel();
   nextWork();

}

////********************************************************
void Window::backLight()
{
    QTime t;
    t.start();


    int Red = redSlider->value();
    int Green =greenSlider->value();
    int Blue =blueSlider->value();
    int TrackBar_Sol = saturationSlider->value();
    Red=(Red+((Red / 128.0) * TrackBar_Sol));
    Green=(Green+((Green / 128.0) * TrackBar_Sol));  // ������������
    Blue=(Blue+((Blue / 128.0) * TrackBar_Sol));

    int TrackBar_Light =255-brightnessSlider->value();
   Red=IntToByte(Red-(qFloor(Red*TrackBar_Light)/ 255.0));
   Green=IntToByte(Green-(qFloor(Green*TrackBar_Light)/ 255.0));//�������
   Blue=IntToByte(Blue-(qFloor(Blue*TrackBar_Light)/ 255.0));

    int channels = channelSpinBox->value();
    ba->clear();
    ba->append(255); // �������
    for (int i=0;i<channels;i=i+3)
    {
        ba->append(Red);
        ba->append(Green);
        ba->append(Blue);
    }

   writePort();

   refreshAmbilightEvaluated(t.elapsed());

   if (!this->isHidden())
   {
       originalPixmap = QPixmap(10,10);
       QPainter p(&originalPixmap);
       p.fillRect(QRect(0,0,10,10),QColor(Red,Green,Blue));
       screenshotLabel->setPixmap(originalPixmap.scaled(screenshotLabel->size(),
                                                  Qt::KeepAspectRatio,
                                                  Qt::SmoothTransformation));
  }
   nextWork();

}
int random(int val)
{
    return qrand()%val;
}

////********************************************************
int newRed=0;
int newGreen=0;
int newBlue=0;
int Red=0;
int Green=0;
int Blue=0;

void Window::moodLamp()
{
    QTime t;
    t.start();

  //   qDebug() <<  Red <<" " << Green  <<" "<< Blue;
 //  qDebug() << "new " << newRed <<" " << newGreen  <<" "<< newBlue;


    if ((Red==newRed) && (Green==newGreen) && (Blue==newBlue))
    {
        newRed = random(255);
        newGreen =random(255);
        newBlue =random(255);
        int sw = random(8);
        switch (sw)
        {
            case 0:             newRed=0;            break;
            case 1:            newGreen=0;            break;
            case 2:            newBlue=0;            break;
            case 3:             newBlue=0;            newRed=0;            break;
            case 4:            newGreen=0;            newRed=0;            break;
            case 5:            newGreen=0;            newBlue=0;            break;
        }
    }

    if(newRed!=Red) { if (Red>newRed) --Red; else ++Red;}
    if(newGreen!=Green)  {if (Green>newGreen) --Green; else ++Green;}
    if(newBlue!=Blue)  {if (Blue>newBlue) --Blue; else ++Blue;}

    int TrackBar_Sol = saturationSlider->value();
   int prRed=(Red+((Red / 128.0) * TrackBar_Sol));
   int prGreen=(Green+((Green / 128.0) * TrackBar_Sol));  // ������������
   int prBlue=(Blue+((Blue / 128.0) * TrackBar_Sol));

    int TrackBar_Light =255-brightnessSlider->value();
   prRed=IntToByte(prRed-(qFloor(prRed*TrackBar_Light)/ 255.0));
   prGreen=IntToByte(prGreen-(qFloor(prGreen*TrackBar_Light)/ 255.0));//�������
   prBlue=IntToByte(prBlue-(qFloor(prBlue*TrackBar_Light)/ 255.0));

    int channels = channelSpinBox->value();
    ba->clear();
    ba->append(255); // �������
    for (int i=0;i<channels;i=i+3)
    {
        ba->append(prRed);
        ba->append(prGreen);
        ba->append(prBlue);
    }

   writePort();

   refreshAmbilightEvaluated(t.elapsed());

   if (!this->isHidden())
   {
       originalPixmap = QPixmap(10,10);
       QPainter p(&originalPixmap);
       p.fillRect(QRect(0,0,10,10),QColor(prRed,prGreen,prBlue));
       screenshotLabel->setPixmap(originalPixmap.scaled(screenshotLabel->size(),
                                                  Qt::KeepAspectRatio,
                                                  Qt::SmoothTransformation));
  }
   nextWork();

}

void Window::nextWork()
{
    if (!onAmbiligth)
    {
        closePort();
        return;
    }

    switch (modeComboBox->currentIndex())
    {
    case 0:
        QTimer::singleShot(delaySpinBox->value(), this, SLOT(shootScreen()));
        break;
    case 1:
        QTimer::singleShot(delaySpinBox->value(), this, SLOT(backLight()));
        break;
    case 2:
        QTimer::singleShot(delaySpinBox->value(), this, SLOT(moodLamp()));
        break;
    }

}

void Window::writePort()
{
        QByteArray data;
        data.append(*ba);
        // ��������
        if (!port->isOpen())
        {
            onAction->setEnabled(true);
            offAction->setEnabled(false);
            if (onAmbiligth)
              setIcon(3);
            onAmbiligth = false;
        }
        else
          // ����� � ����
          port->write(data);
 }

 //v1.0.4 ������� ����������
void Window::refreshAmbilightEvaluated(double updateResultMs)
{
    int usbTimerDelayMs = delaySpinBox->value();
    double secs = (updateResultMs + usbTimerDelayMs) / 1000;
    double hz = 0;

    if(secs != 0){
        hz = 1 / secs;
    }

    refreshAmbiliht->setText( QString::number(hz,'f', 4) /* ms to hz */ );
}

void Window::updateScreenshotLabel()
{
    // todo ������ ���������
    if (!this->isHidden())
    {
        //v.1.0.6 �����
        if (viewGridCheckBox->isChecked())
            {
                QPainter p(&originalPixmap);
                int stepx=originalPixmap.width()/100.0 * stepSpinBox->value();
                int stepy=originalPixmap.height()/100.0 * stepSpinBox->value();
                for (int x=0;x<originalPixmap.width();x=x+stepx)
                          p.drawLine(x,0,x,originalPixmap.height());
                for (int y=0;y<originalPixmap.height();y=y+stepy)
                          p.drawLine(0,y,originalPixmap.width(),y);
            }

        screenshotLabel->setPixmap(originalPixmap.scaled(screenshotLabel->size(),
                                                   Qt::KeepAspectRatio,
                                                   Qt::SmoothTransformation));
   }
}

//! [1]
void Window::setVisible(bool visible)
{
    //minimizeAction->setEnabled(visible);
    //maximizeAction->setEnabled(!isMaximized());
    //restoreAction->setEnabled(isMaximized() || !visible);
    QDialog::setVisible(visible);
}
//! [1]

//! [2]
void Window::closeEvent(QCloseEvent *event)
{
   if (trayIcon->isVisible()) {
        hide();
        event->ignore();
    }
}
//! [2]

//! [3]
void Window::setIcon(int index)
{
    QIcon icon;
    QString text;
    switch (index)
    {
      case 1:
            icon = QIcon(":/images/off.png");
            text = tr( "Off");
            break;
      case 2:
            icon = QIcon(":/images/on.png");
            text = tr("On");
            break;
      case 3:
            icon = QIcon(":/images/error.png");
            text= tr("Error");
            break;
     }
    trayIcon->setIcon(icon);
    trayIcon->setToolTip(text);
}
//! [3]

//! [4]
void Window::iconActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason) {
    case QSystemTrayIcon::Trigger:
        break;
    case QSystemTrayIcon::DoubleClick:
        if (onAmbiligth)
            off_ambiligth();
        else
            on_ambiligth();
        break;
    case QSystemTrayIcon::MiddleClick:
        //showMessage();
        break;
    default:
        ;
    }
}
//! [4]

void Window::switchMode()
{
    if (modeComboBox->currentIndex()==modeComboBox->count()-1)
           modeComboBox->setCurrentIndex(0);
        else
    {
            int i = modeComboBox->currentIndex();
           modeComboBox->setCurrentIndex(++i);
     }
}

void Window::switchAmbilight()
{
    if (onAmbiligth)
        off_ambiligth();
    else
        on_ambiligth();
}


void Window::createHardwareGroupBox()
{
    comGroupBox = new QGroupBox(tr("Hardware settings"));
    comLabel = new QLabel(tr("Port:"));
    comComboBox = new QComboBox;

    this->m_sde = new SerialDeviceEnumerator(this);
    connect(this->m_sde, SIGNAL(hasChanged(QStringList)),
            this, SLOT(slotPrintAllDevices(QStringList)));
    this->m_sde->setEnabled(true);


    QLabel * brLaber= new QLabel(tr("Baud Rate"));
    QLabel * dbLaber= new QLabel(tr("Data Bits"));
    QLabel * sbLaber= new QLabel(tr("Stop Bits"));
    QLabel * pLaber= new QLabel(tr("Parity"));

    brComboBox = new QComboBox();
    brComboBox->addItem(tr("300"),AbstractSerial::BaudRate300);
    brComboBox->addItem(tr("600"),AbstractSerial::BaudRate600);
    brComboBox->addItem(tr("1.200"),AbstractSerial::BaudRate1200);
    brComboBox->addItem(tr("2.400"),AbstractSerial::BaudRate2400);
    brComboBox->addItem(tr("4.800"),AbstractSerial::BaudRate4800);
    brComboBox->addItem(tr("9.600"),AbstractSerial::BaudRate9600);
    brComboBox->addItem(tr("19.200"),AbstractSerial::BaudRate19200);
    brComboBox->addItem(tr("38.400"),AbstractSerial::BaudRate38400);
    brComboBox->addItem(tr("57.600"),AbstractSerial::BaudRate57600);
    brComboBox->addItem(tr("115.200"),AbstractSerial::BaudRate115200);
    brComboBox->addItem(tr("230.400"),AbstractSerial::BaudRate230400);
    brComboBox->addItem(tr("460.800"),AbstractSerial::BaudRate460800);
    brComboBox->addItem(tr("921.600"),AbstractSerial::BaudRate921600);

    dbComboBox = new QComboBox();
    dbComboBox->addItem(tr("7 bits"),AbstractSerial::DataBits7);
    dbComboBox->addItem(tr("8 bits"),AbstractSerial::DataBits8);

    sbComboBox = new QComboBox();
    sbComboBox->addItem(tr("1 bit"),AbstractSerial::StopBits1);
    sbComboBox->addItem(tr("2 bits"),AbstractSerial::StopBits2);

    pComboBox = new QComboBox();
    pComboBox->addItem(tr("None"),AbstractSerial::ParityNone);
    pComboBox->addItem(tr("Odd"),AbstractSerial::ParityOdd);
    pComboBox->addItem(tr("Even"),AbstractSerial::ParityEven);
    pComboBox->addItem(tr("Mark"),AbstractSerial::ParityMark);
    pComboBox->addItem(tr("Space"),AbstractSerial::ParitySpace);

    QLabel * fcLaber= new QLabel(tr("Flow Control"));
      QLabel * rtsLaber= new QLabel(tr("RTS"));
         QLabel * dtrLaber= new QLabel(tr("DTR"));

         fcComboBox = new QComboBox();
         fcComboBox->addItem(tr("None"),AbstractSerial::FlowControlOff);
         fcComboBox->addItem(tr("Hardware"),AbstractSerial::FlowControlHardware);
         fcComboBox->addItem(tr("X-On/X-Off"),AbstractSerial::FlowControlXonXoff);


       rtsCheckBox = new QCheckBox();
       dtrCheckBox = new QCheckBox();


    QGridLayout *hardLayout = new QGridLayout;
    hardLayout->addWidget(comLabel,0,0);
    hardLayout->addWidget(comComboBox,0,1,1,3);
    hardLayout->addWidget(brLaber,1,0);
    hardLayout->addWidget(brComboBox,1,1);
    hardLayout->addWidget(dbLaber,2,0);
    hardLayout->addWidget(dbComboBox,2,1);
    hardLayout->addWidget(sbLaber,3,0);
    hardLayout->addWidget(sbComboBox,3,1);
    hardLayout->addWidget(pLaber,4,0);
    hardLayout->addWidget(pComboBox,4,1);

    hardLayout->addWidget(fcLaber,1,2,Qt::AlignRight);
    hardLayout->addWidget(fcComboBox,1,3);

    hardLayout->addWidget(rtsLaber,2,2,Qt::AlignRight);
    hardLayout->addWidget(rtsCheckBox,2,3);
    hardLayout->addWidget(dtrLaber,3,2,Qt::AlignRight);
    hardLayout->addWidget(dtrCheckBox,3,3);


    comGroupBox->setLayout(hardLayout);
}

void Window::createAdvancedGroupBox()
{
    advGroupBox = new QGroupBox(tr("Advanced settings"));

    onAmbiligthCheckBox = new QCheckBox(tr("On ambilight with start"));

    modeStartComboBox = new QComboBox;
    modeStartComboBox->addItem(QIcon(":/images/ambilight_icon.png"),tr("Ambilight"));
    modeStartComboBox->addItem(QIcon(":/images/backlight.png"),tr("Backlight"));
    modeStartComboBox->addItem(QIcon(":/images/moodlamp.png"),tr("Mood lamp"));
    modeStartComboBox->addItem(QIcon(":/images/settings.png"),tr("Last mode"));


    QLabel *screenLabel = new QLabel(tr("Screen"));
    QLabel *delayLabel = new QLabel(tr("Refresh ambilight delay"));
    QLabel *delayLabel2 = new QLabel(tr("ms"));
    QLabel *channelLabel = new QLabel(tr("Channel:"));
    QLabel *refreshLabel = new QLabel(tr("Refresh ambilight"));
    QLabel *refreshLabel2 = new QLabel(tr("Hz"));
    QLabel *hotkeyLabel = new QLabel(tr("Hotkey on/off"));
    QLabel *hotkeyLabel2 = new QLabel(tr("(need restart)"));
    QLabel *hotkey2Label = new QLabel(tr("Hotkey switch mode"));



    refreshAmbiliht = new QLabel();

    delaySpinBox = new  QSpinBox();
    delaySpinBox->setMinimum(1);
    delaySpinBox->setMaximum(2000);
    delaySpinBox->setSingleStep(1);

    channelSpinBox = new  QSpinBox();
    channelSpinBox->setMinimum(1);
    channelSpinBox->setMaximum(42);

    screenComboBox = new QComboBox();
    int count =  QApplication::desktop()->screenCount();
    for (int i=0;i<count;++i)
        screenComboBox->addItem(QIcon(":/images/tv.png"),QString("%1").arg(i));

    QGridLayout *aLayout = new QGridLayout;

    aLayout->addWidget(onAmbiligthCheckBox,0,0);
    aLayout->addWidget(modeStartComboBox,0,1);
    aLayout->addWidget(screenLabel,1,0);
    aLayout->addWidget(screenComboBox,1,1);
    aLayout->addWidget(delayLabel,2,0);
    aLayout->addWidget(delaySpinBox,2,1);
    aLayout->addWidget(delayLabel2,2,2);
    aLayout->addWidget(channelLabel,3,0);
    aLayout->addWidget(channelSpinBox,3,1);
    aLayout->addWidget(refreshLabel,4,0);
    aLayout->addWidget(refreshAmbiliht,4,1);
    aLayout->addWidget(refreshLabel2,4,2);

    aLayout->addWidget(hotkeyLabel,5,0);
    hotkeyLine = new QLineEdit();
    aLayout->addWidget(hotkeyLine,5,1);
    aLayout->addWidget(hotkeyLabel2,5,2,2,1);

    aLayout->addWidget(hotkey2Label,6,0);
    hotkeyLineMode = new QLineEdit();
    aLayout->addWidget(hotkeyLineMode,6,1);

    advGroupBox->setLayout(aLayout);
}

void Window::slotPrintAllDevices(const QStringList &list)
{
    qDebug() << "\n ===> All devices: " << list;

    foreach (QString s, list) {
        this->m_sde->setDeviceName(s);

        comComboBox->addItem(QIcon(":/images/cpu.png"),this->m_sde->name());

        qDebug() << "\n <<< info about: " << this->m_sde->name() << " >>>";
        qDebug() << "-> description  : " << this->m_sde->description();
        qDebug() << "-> driver       : " << this->m_sde->driver();
        qDebug() << "-> friendlyName : " << this->m_sde->friendlyName();
        qDebug() << "-> hardwareID   : " << this->m_sde->hardwareID();
        qDebug() << "-> locationInfo : " << this->m_sde->locationInfo();
        qDebug() << "-> manufacturer : " << this->m_sde->manufacturer();
        qDebug() << "-> productID    : " << this->m_sde->productID();
        qDebug() << "-> service      : " << this->m_sde->service();
        qDebug() << "-> shortName    : " << this->m_sde->shortName();
        qDebug() << "-> subSystem    : " << this->m_sde->subSystem();
        qDebug() << "-> systemPath   : " << this->m_sde->systemPath();
        qDebug() << "-> vendorID     : " << this->m_sde->vendorID();
        qDebug() << "-> revision     : " << this->m_sde->revision();
        qDebug() << "-> bus          : " << this->m_sde->bus();
        qDebug() << "-> is exists    : " << this->m_sde->isExists();
        qDebug() << "-> is busy      : " << this->m_sde->isBusy();
    }
}

void Window::createColorGroupBox()
{
    colorGroupBox = new QGroupBox(tr("Colors settings"));


    brightnessLabel = new QLabel("0");
    saturationLabel = new QLabel("0");
    redLabel = new QLabel("0");
    greenLabel = new QLabel("0");
    blueLabel = new QLabel("0");
    thresholdLabel = new QLabel("0");

    brightnessSlider = new QSlider(Qt::Horizontal);
    brightnessSlider->setRange(0,255);
    saturationSlider = new QSlider(Qt::Horizontal);
    saturationSlider->setRange(0,255);
    redSlider = new QSlider(Qt::Horizontal);
    redSlider->setRange(0,255);
    greenSlider = new QSlider(Qt::Horizontal);
    greenSlider->setRange(0,255);
    blueSlider = new QSlider(Qt::Horizontal);
    blueSlider->setRange(0,255);

    thresholdSlider = new QSlider(Qt::Horizontal);
    thresholdSlider->setRange(0,255);

    QGridLayout *messageLayout = new QGridLayout;

    messageLayout->addWidget(new QLabel(tr("Brightness")), 0, 0);
    messageLayout->addWidget(brightnessSlider, 0, 1);
    messageLayout->addWidget(brightnessLabel, 0, 2);
    messageLayout->addWidget(new QLabel(tr("Saturation")), 1, 0);
    messageLayout->addWidget(saturationSlider, 1, 1);
    messageLayout->addWidget(saturationLabel, 1, 2);
    messageLayout->addWidget(new QLabel(tr("Hue red")), 2, 0);
    messageLayout->addWidget(redSlider, 2, 1);
    messageLayout->addWidget(redLabel, 2, 2);
    messageLayout->addWidget(new QLabel(tr("Hue green")), 3, 0);
    messageLayout->addWidget(greenSlider, 3, 1);
    messageLayout->addWidget(greenLabel, 3, 2);
    messageLayout->addWidget(new QLabel(tr("Hue blue")), 4, 0);
    messageLayout->addWidget(blueSlider, 4, 1);
    messageLayout->addWidget(blueLabel, 4, 2);
    messageLayout->addWidget(new QLabel(tr("Threshold")), 6, 0);
    messageLayout->addWidget(thresholdSlider, 6, 1);
    messageLayout->addWidget(thresholdLabel, 6, 2);

    messageLayout->setColumnMinimumWidth(2,20);

    connect(brightnessSlider, SIGNAL(valueChanged(int)), brightnessLabel,  SLOT(setNum(int)));
    connect(saturationSlider, SIGNAL(valueChanged(int)), saturationLabel,  SLOT(setNum(int)));
    connect(redSlider, SIGNAL(valueChanged(int)), redLabel,  SLOT(setNum(int)));
    connect(greenSlider, SIGNAL(valueChanged(int)), greenLabel,  SLOT(setNum(int)));
    connect(blueSlider, SIGNAL(valueChanged(int)), blueLabel,  SLOT(setNum(int)));
    connect(thresholdSlider, SIGNAL(valueChanged(int)), thresholdLabel,  SLOT(setNum(int)));

    //messageLayout->setColumnStretch(2,2);
   // messageLayout->setRowStretch(5,1);
    colorGroupBox->setLayout(messageLayout);
}

void  Window::on_ambiligth()
{
    setIcon(2);
    onAction->setEnabled(false);
    offAction->setEnabled(true);
    onAmbiligth = true;
    openPort();
    switch (modeComboBox->currentIndex())
    {
    case 0:
         shootScreen();
        break;
    case 1:
       backLight();
        break;
    case 2:
        moodLamp();
        break;
    }

}

void  Window::off_ambiligth()
{
     setIcon(1);
     onAction->setEnabled(true);
     offAction->setEnabled(false);

     onAmbiligth = false;
}

void Window::changeMode()
{
    switch (modeComboBox->currentIndex())
    {
    case 0:
        thresholdSlider->setEnabled(true);
        brightnessSlider->setEnabled(true);
        saturationSlider->setEnabled(true);
        redSlider->setEnabled(true);
        greenSlider->setEnabled(true);
        blueSlider->setEnabled(true);
        break;
    case 1:
        thresholdSlider->setEnabled(false);
        brightnessSlider->setEnabled(true);
        saturationSlider->setEnabled(true);
        redSlider->setEnabled(true);
        greenSlider->setEnabled(true);
        blueSlider->setEnabled(true);
        break;
    case 2:
        thresholdSlider->setEnabled(false);
        brightnessSlider->setEnabled(true);
        saturationSlider->setEnabled(true);
        redSlider->setEnabled(false);
        greenSlider->setEnabled(false);
        blueSlider->setEnabled(false);
        break;
    }
}

void Window::createActions()
{
    onAction = new QAction(QIcon(":/images/on.png"),tr("On ambilight"), this);
    connect(onAction, SIGNAL(triggered()), this, SLOT(on_ambiligth()));

    offAction = new QAction(QIcon(":/images/off.png"),tr("Off ambilight"), this);
    connect(offAction, SIGNAL(triggered()), this, SLOT(off_ambiligth()));

    settingsAction = new QAction(QIcon(":/images/settings.png"),tr("&Settings"), this);
    connect(settingsAction, SIGNAL(triggered()), this, SLOT(showNormal()));

    quitAction = new QAction(QIcon(":/images/error.png"),tr("&Quit"), this);
    connect(quitAction, SIGNAL(triggered()), qApp, SLOT(quit()));
}

void Window::createTrayIcon()
{
    createActions();

    trayIconMenu = new QMenu(this);
    trayIconMenu->addAction(onAction);
    trayIconMenu->addAction(offAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(settingsAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(quitAction);

    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setContextMenu(trayIconMenu);
}

void Window::createZoneTab()
{
    QGridLayout *zoneLayout = new QGridLayout(zoneTabWidget);

   model = new QStandardItemModel(COUNT_AREA, 5);
    QTableView *tableView = new QTableView();

    tableView->setModel(model);

    zoneDesktopCheckBox = new QCheckBox(tr("View zone on desktop"));

    zoneLayout->addWidget(tableView,0,0,1,3);
    // todo ������� ���, ����� �� ��� ���
//    zoneLayout->addWidget(zoneDesktopCheckBox);

    viewZoneCheckBox = new QCheckBox(tr("View area on preview"));
    zoneLayout->addWidget(viewZoneCheckBox,1,0,1,3);

    viewGridCheckBox = new QCheckBox(tr("View grid on preview. Step "));
    zoneLayout->addWidget(viewGridCheckBox,2,0);
    stepSpinBox = new  QSpinBox();
    stepSpinBox->setMinimum(1);
    stepSpinBox->setMaximum(100);
    stepSpinBox->setSingleStep(1);
    zoneLayout->addWidget(stepSpinBox,2,1);
    zoneLayout->addWidget(new QLabel("%"),2,2);



    saveZone = new QPushButton(tr("Apply"));
    zoneLayout->addWidget(saveZone,3,0,1,3);

    SpinBoxDelegate *delegate = new SpinBoxDelegate;
    tableView->setItemDelegate(delegate);
    QStringList headerLabels;
    headerLabels << tr("Left") <<tr("Top") << tr("Width") <<tr( "Heigth")<<tr("Brightness");
    model->setHorizontalHeaderLabels(headerLabels);
    QStringList verticalLabels;
    for (int i=0;i<COUNT_AREA;++i)
       verticalLabels.append(QString("%1 %2").arg(tr("Area ")).arg(i+1));
    model->setVerticalHeaderLabels(verticalLabels);
    tableView->horizontalHeader()->setResizeMode( QHeaderView::Stretch);


return;
}

