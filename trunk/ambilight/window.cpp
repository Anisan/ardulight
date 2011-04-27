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

#include "grab\grab_api.h"

#include "delegate.h"
#include "version.h"


//! [0]

int COUNT_AREA=4;

Window::Window()
{


    isWinAPIGrab = false;

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
     QGroupBox *testGroupBox = new QGroupBox(tr("Testing"));
     QHBoxLayout *testLayout = new QHBoxLayout(testGroupBox);
     QPushButton *colorTestButton = new QPushButton(tr("Color test"));
     QPushButton *zoneTestButton = new QPushButton(tr("Zone test"));
     testLayout->addWidget(colorTestButton);
     testLayout->addWidget(colorTestLabel = new QLabel("..."));
     testLayout->addWidget(zoneTestButton);
     testLayout->addWidget(zoneTestLabel = new QLabel("..."));
      hardLayout->addWidget(testGroupBox);

     saveAdv = new QPushButton(tr("Apply"));
     hardLayout->addWidget(saveAdv);
     QString about = QString("<a href=\"http://code.google.com/p/ardulight/\">%1 v.%2</a>").arg("Ambilight - 2011 - Eraser Soft").arg(VERSION_STR);
     QLabel *label= new QLabel;
     label->setTextInteractionFlags(Qt::LinksAccessibleByMouse);
     label->setOpenExternalLinks(true);
     label->setText(about);
     label->show();
     hardLayout->addWidget(label,0,Qt::AlignCenter);

     zoneTabWidget = new QWidget;
     createZoneTab();

    tabWidget->addTab(colorTabWidget, tr("Main"));
    tabWidget->addTab(zoneTabWidget, tr("Area"));
    tabWidget->addTab(hardTabWidget, tr("Advanced"));

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(tabWidget);

    setLayout(mainLayout);

    setIcon(1);


    setWindowTitle(QString("%1 v.%2").arg(tr("Ambilight")).arg(VERSION_STR));
    setWindowIcon(QIcon(":/images/ambilight_icon.ico"));
    resize(400, 300);

    readSettings();

    createZones();

    readZonesSettings();

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

    connect(colorTestButton, SIGNAL(clicked()),this,SLOT(TestColor()));
    connect(zoneTestButton, SIGNAL(clicked()),this,SLOT(TestZone()));

    connect(brightnessSlider,SIGNAL(sliderReleased()),this,SLOT(saveColorSettings()));
    connect(saturationSlider,SIGNAL(sliderReleased()),this,SLOT(saveColorSettings()));
    connect(contrastSlider,SIGNAL(sliderReleased()),this,SLOT(saveColorSettings()));
    connect(gammaSlider,SIGNAL(sliderReleased()),this,SLOT(saveColorSettings()));
    connect(redSlider,SIGNAL(sliderReleased()),this,SLOT(saveColorSettings()));
    connect(greenSlider,SIGNAL(sliderReleased()),this,SLOT(saveColorSettings()));
    connect(blueSlider,SIGNAL(sliderReleased()),this,SLOT(saveColorSettings()));
    connect(thresholdSlider,SIGNAL(sliderReleased()),this,SLOT(saveColorSettings()));
    connect(modeComboBox,SIGNAL(currentIndexChanged(int)),this,SLOT(saveModeSettings()));

    connect(modeComboBox,SIGNAL(currentIndexChanged(int)),this,SLOT(changeMode()));

    connect(saveAdv, SIGNAL(clicked()), this, SLOT(saveSettings()));

    connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
            this, SLOT(iconActivated(QSystemTrayIcon::ActivationReason)));

     trayIcon->show();


      originalPixmap = QPixmap();
     shootScreen();
     checkSDK();
}
//! [0]


// v1.1.6 SDK
void Window::checkSDK()
{

    QFile file("status");
    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QByteArray read = file.readAll();
        file.close();
        QString status = QString::fromUtf8(read.data(), read.size());
        //v.1.0.10 fix
       if (status=="off" && onAmbiligth)  off_ambiligth();
       if (status=="ambilight" && (!onAmbiligth || modeComboBox->currentIndex()!=0))   { modeComboBox->setCurrentIndex(0);on_ambiligth();};
       if (status=="backlight" && (!onAmbiligth ||  modeComboBox->currentIndex()!=1))   { modeComboBox->setCurrentIndex(1);on_ambiligth();};
       if (status=="moodlamp" &&  (!onAmbiligth || modeComboBox->currentIndex()!=2)) { modeComboBox->setCurrentIndex(2);on_ambiligth();};
   }

    if (!this->isHidden())
     QTimer::singleShot(5000, this, SLOT(checkSDK()));
    else
     QTimer::singleShot(2000, this, SLOT(checkSDK()));
}

void Window::setSDK(QString status)
{
    QFile file("status");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
             return;
   file.write(status.toUtf8());
   file.close();

}

void Window::readSettings()
{
    qDebug() << " load settings ";
   // QSettings settings("EraserSoft", "Ambiligth");

    //v.1.0.10 ����������� �����, ����� ����������
    #ifdef Q_WS_WIN
        isWinAPIGrab =settings->value("isWinAPIGrab",true).toBool();
    #endif

    modeComboBox->setCurrentIndex( settings->value("WorkMode",0).toInt());
    onAmbiligth = settings->value("onAmbilight",false).toBool();
     //v 1.1.2 auto start mode
    modeStartComboBox->setCurrentIndex( settings->value("StartMode",0).toInt());
    if (modeStartComboBox->currentIndex()<3)
       modeComboBox->setCurrentIndex(modeStartComboBox->currentIndex());
    changeMode();


   onAmbiligthCheckBox->setChecked(settings->value("onAmbilight",false).toBool());
   delaySpinBox->setValue(settings->value("Delay",40).toInt());
   viewZoneCheckBox->setChecked(settings->value("ViewZoneOnPreview",false).toBool());

   viewGridCheckBox->setChecked(settings->value("ViewGridOnPreview",false).toBool());
   stepSpinBox->setValue(settings->value("StepGrid",5).toInt());

   hotkeyLine->setText(settings->value("Hotkey","Ctrl+Alt+F").toString());
   hotkeyLineMode->setText(settings->value("HotkeyMode","Ctrl+Alt+M").toString());

   areasSpinBox->setValue(settings->value("Areas",4).toInt());
   COUNT_AREA = settings->value("Areas",4).toInt();

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

    qDebug() << "end load settings ";
}

void Window::readZonesSettings()
{
    for (int row = 0; row < COUNT_AREA; ++row) {
        for (int column = 0; column < 4; ++column) {
            QModelIndex index = model->index(row, column, QModelIndex());
            model->setData(index, settings->value(QString("Zone_%1_%2").arg(row).arg(column),0).toInt() );
        }
        QModelIndex index = model->index(row, 4, QModelIndex());
        model->setData(index, settings->value(QString("Zone_%1_Brightness").arg(row),100).toInt() );
    }
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
}


void Window::saveSettings()
{
     qDebug() << " save settings";
    settings->setValue("onAmbilight", onAmbiligthCheckBox->isChecked());
    settings->setValue("StartMode", modeStartComboBox->currentIndex());
    settings->setValue("Screen",screenComboBox->currentText());
    settings->setValue("Delay",delaySpinBox->value());
    settings->setValue("Areas",areasSpinBox->value());

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


    COUNT_AREA = areasSpinBox->value();
    createZones();
    readZonesSettings();
}

void Window::saveColorSettings()
{
    // v1.1.3 ��������� ����� ��� ������� ������
    qDebug() << " save color settings";
    settings->setValue(QString("Brightness_%1").arg(modeComboBox->currentIndex()), brightnessSlider->value());
    settings->setValue(QString("Saturation_%1").arg(modeComboBox->currentIndex()), saturationSlider->value());
    settings->setValue(QString("Contrast_%1").arg(modeComboBox->currentIndex()), contrastSlider->value());
    settings->setValue(QString("Gamma_%1").arg(modeComboBox->currentIndex()), gammaSlider->value());
    settings->setValue(QString("Red_%1").arg(modeComboBox->currentIndex()), redSlider->value());
    settings->setValue(QString("Green_%1").arg(modeComboBox->currentIndex()), greenSlider->value());
    settings->setValue(QString("Blue_%1").arg(modeComboBox->currentIndex()), blueSlider->value());
    settings->setValue(QString("Threshold_%1").arg(modeComboBox->currentIndex()), thresholdSlider->value());

    calcColorTable();
}

void Window::saveModeSettings()
{
    qDebug() << " save mode settings";
    settings->setValue("WorkMode", modeComboBox->currentIndex());
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

        AbstractSerial::BaudRate baud = AbstractSerial::BaudRate115200;
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

        AbstractSerial::DataBits dataBits = AbstractSerial::DataBits8;
                    switch (dbComboBox->currentIndex())
                    {
                    case 0: dataBits = AbstractSerial::DataBits7; break;
                    case 1: dataBits = AbstractSerial::DataBits8; break;
                        }
        if (!port->setDataBits(dataBits)) {
            qDebug() << "Set data bits " << dataBits << " error.";
        }

        AbstractSerial::Parity par = AbstractSerial::ParityNone;
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

        AbstractSerial::StopBits stopBits  = AbstractSerial::StopBits2;
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
        setSDK("off");
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
int BLimit(int b)
{
    if (b < 0)
      return 0;
    else if (b > 255)
      return 255;
    else
      return b;
 }

double Power(double Base,double Exponent)
{
    return exp(Exponent * log(Base));
}

//v.1.1.9 �����������
// ���������� ������� ��������������� ������
 int GammaT[255];
 int ContractT[255];

 void Window::calcColorTable()
{
    // .v1.1.8 gamma
    //{0.0 < L < 7.0}
    double L=gammaSlider->value()/10.0;
    GammaT[0]= 0;
    for (int X= 1; X<=255;++X)
      GammaT[X] = BLimit(255 * Power(X / 255.0, 1 / L));

    // .v1.1.7 contrast
    float vd;
    int mC = 128;
    int contrast_value = contrastSlider->value();
    if (contrast_value > 0)
        vd = 1 + (contrast_value / 10.0);
      else
        vd = 1 - (sqrt(-contrast_value) / 10.0);
      for (int X= 0; X<=255;++X)
        ContractT[X] = BLimit(mC + trunc((X - mC) * vd));
}

void Window::GetPix(QRect rect, int brightness)
{
  int Red=0;
  int Green=0;
  int Blue=0;

  QPainter p(&originalPixmap);

  //v1.1.5 - �� ��������� ���������� ���� ������ ������� �������
 if ((rect.height()==0) || (rect.width()==0))
  {
      ba->append(Red);
      ba->append(Green); //��������� � �����
      ba->append(Blue);
      return;
 }


  QImage img = originalPixmap.copy(rect).toImage();

  for (int y=0; y< rect.height();y++){
      for (int x=0; x<rect.width(); x++){
          QColor pix = img.pixel(x,y);
          int sRed = ContractT[pix.red()];
          int sGreen = ContractT[pix.green()];   // �������������
          int sBlue = ContractT[pix.blue()];

          sRed = GammaT[sRed];
          sGreen = GammaT[sGreen]; // �����
          sBlue = GammaT[sBlue];

          Red += sRed;
          Green += sGreen;         //��������� ��� �������
          Blue += sBlue;

//          pix.setRed(sRed);
//          pix.setBlue(sBlue);         // ��� ��� ��������
//          pix.setGreen(sGreen);
//          img.setPixel(x,y,pix.rgb());
        }
  }

  //p.drawImage(rect.x(),rect.y(),img); // � ��� ����

  int z =rect.width()*rect.height();
  Red=Red/z;
  Green=Green/ z;   //�������� ������� �������� ��� ������� ������
  Blue=Blue/z;

   int Average_Pix= qFloor((Red+Green+Blue)/ 3.0); //�������
   Red= (Red+((Red / 128.0) * saturationSlider->value()));
   Green=(Green+((Green / 128.0) * saturationSlider->value()));  // ������������
   Blue= (Blue+((Blue / 128.0) * saturationSlider->value()));
   int  Aver=qFloor((Red + Green + Blue)/ 3.0)- Average_Pix; //������� � ������ ����� ��������������

   Red=Red*( redSlider->value() / 128.0) - Aver;
   Green=Green*(  greenSlider->value() / 128.0) - Aver; // �������
   Blue=Blue*( blueSlider->value() / 128.0) - Aver;

   // v1.0.6 ������� �� �����
  int TrackBar_Light =255-brightnessSlider->value()/100.0*brightness;
  Red=BLimit(Red-(qFloor(Red*TrackBar_Light)/ 255.0));
  Green=BLimit(Green-(qFloor(Green*TrackBar_Light)/ 255.0));//�������
  Blue=BLimit(Blue-(qFloor(Blue*TrackBar_Light)/ 255.0));

  // v.1.1.8 fix ���������� ��������
  int  threshold = thresholdSlider->value();
  if ((0.299*Red + 0.587*Green + 0.114*Blue) <= threshold)
  {
      Red=0;
      Green=0;
      Blue=0;
  }

  ba->append(Red);
  ba->append(Green); //��������� � �����
  ba->append(Blue);

  // ������ ����
     if (!this->isHidden())
          if (viewZoneCheckBox->isChecked())
              {
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
    //    originalPixmap = QPixmap::grabWindow(QApplication::desktop()->screen(screen) ->winId(),
    //                                         screenres.x(), //!
    //                                         screenres.y(), //!
    //                                         screenres.width(),
    //                                         screenres.height());

    //v.1.0.10 fix blink cursor for windows
    if(isWinAPIGrab){
                  originalPixmap = GrabWinAPI::grabScreen(QApplication::desktop()->screen(screen) ->winId(),
                                                             screenres.x(), //!
                                                             screenres.y(), //!
                                                             screenres.width(),
                                                             screenres.height());
      } else {
                 originalPixmap = GrabQt::grabScreen(QApplication::desktop()->screen(screen) ->winId(),
                                                         screenres.x(), //!
                                                         screenres.y(), //!
                                                         screenres.width(),
                                                         screenres.height());
    }

    //�������� �������� ��� ��� ����� ����������
    float W = screenres.width() / 100.0;
    float H =screenres.height() / 100.0;

    ba->clear();
    ba->append(255); // �������
    //v.1.0.11 �������������� ���-�� ���
    for (int i=0;i<COUNT_AREA;++i)
    {
        QRect R = QRect(W*getData(i,0),H*getData(i,1),W*getData(i,2),H*getData(i,3));
        if (R.bottom()>originalPixmap.height()) R.setBottom(originalPixmap.height()-1);
        if (R.right()>originalPixmap.width()) R.setRight(originalPixmap.width()-1);
        GetPix(R,getData(i,4));
    }

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
   Red=BLimit(Red-(qFloor(Red*TrackBar_Light)/ 255.0));
   Green=BLimit(Green-(qFloor(Green*TrackBar_Light)/ 255.0));//�������
   Blue=BLimit(Blue-(qFloor(Blue*TrackBar_Light)/ 255.0));

    ba->clear();
    ba->append(255); // �������
    for (int i=0;i<COUNT_AREA;++i)
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
int speed=0;
int step =0;

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
        speed = random(20);
        int sw = random(6);
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



       int TrackBar_Sol = saturationSlider->value();
       int prRed=(Red+((Red / 128.0) * TrackBar_Sol));
       int prGreen=(Green+((Green / 128.0) * TrackBar_Sol));  // ������������
       int prBlue=(Blue+((Blue / 128.0) * TrackBar_Sol));

        int TrackBar_Light =255-brightnessSlider->value();
       prRed=BLimit(prRed-(qFloor(prRed*TrackBar_Light)/ 255.0));
       prGreen=BLimit(prGreen-(qFloor(prGreen*TrackBar_Light)/ 255.0));//�������
       prBlue=BLimit(prBlue-(qFloor(prBlue*TrackBar_Light)/ 255.0));

        ba->clear();
        ba->append(255); // �������
        for (int i=0;i<COUNT_AREA;++i)
        {
            ba->append(prRed);
            ba->append(prGreen);
            ba->append(prBlue);
        }

       writePort();

       if (step>=speed)
       {
           step=0;
           if(newRed!=Red) { if (Red>newRed) --Red; else ++Red;}
           if(newGreen!=Green)  {if (Green>newGreen) --Green; else ++Green;}
           if(newBlue!=Blue)  {if (Blue>newBlue) --Blue; else ++Blue;}
        }
        else
            ++step;

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

void Sleep(int time)
{
     QEventLoop loop; QTimer::singleShot(time, &loop, SLOT(quit())); loop.exec();
}

void  Window::TestColor()
{
    off_ambiligth();
    int n=0;
    int f =255;
    ba->clear();
    ba->append(255); // �������
    for (int i=0;i<COUNT_AREA;++i)
    {
        ba->append(f);
        ba->append(n);
        ba->append(n);
    }
    colorTestLabel->setText(tr("Red"));
    writePort();
   Sleep(1000);
    ba->clear();
    ba->append(255); // �������
    for (int i=0;i<COUNT_AREA;++i)
    {
        ba->append(n);
        ba->append(f);
        ba->append(n);
    }
    colorTestLabel->setText(tr("Green"));
    writePort();
    Sleep(1000);
    ba->clear();
    ba->append(255); // �������
    for (int i=0;i<COUNT_AREA;++i)
    {
        ba->append(n);
        ba->append(n);
        ba->append(f);
    }
    colorTestLabel->setText(tr("Blue"));
    writePort();
    Sleep(1000);
    ba->clear();
    ba->append(255); // �������
    for (int i=0;i<COUNT_AREA;++i)
    {
        ba->append(f);
        ba->append(f);
        ba->append(f);
    }
    colorTestLabel->setText(tr("White"));
    writePort();
    Sleep(1000);
    colorTestLabel->setText(tr("..."));

}

void  Window::TestZone()
{
    off_ambiligth();
    int n=0;
    int f =255;
    ba->clear();
    ba->append(255); // �������
    for (int i=0;i<COUNT_AREA;++i)
    {
        ba->clear();
        ba->append(255); // �������
        for (int j=0;j<COUNT_AREA;++j)
        {
            if (i==j)
            {
            ba->append(f);
            ba->append(f);
            ba->append(f);
           }
            else
            {
                ba->append(n);
                ba->append(n);
                ba->append(n);
            }
        }
        zoneTestLabel->setText(QString("%1 %2").arg(tr("Area ")).arg(i+1));
        writePort();
        Sleep(1000);
    }
    zoneTestLabel->setText("...");



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
            {
              setIcon(3);
              setSDK("off");
          }
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
    QLabel *areasLabel = new QLabel(tr("Number of zones"));
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

    areasSpinBox = new  QSpinBox();
    areasSpinBox->setMinimum(1);
    areasSpinBox->setMaximum(42);

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
    aLayout->addWidget(areasLabel,3,0);
    aLayout->addWidget(areasSpinBox,3,1);
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
    contrastLabel = new QLabel("0");

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
    contrastSlider = new QSlider(Qt::Horizontal);
    contrastSlider->setRange(-100,100);

    gammaSlider = new QSlider(Qt::Horizontal);
    gammaSlider->setRange(0,40);


    thresholdSlider = new QSlider(Qt::Horizontal);
    thresholdSlider->setRange(0,255);

    QGridLayout *messageLayout = new QGridLayout;

    messageLayout->addWidget(brightnessTLabel = new QLabel(tr("Brightness")), 0, 0);
    messageLayout->addWidget(brightnessSlider, 0, 1);
    messageLayout->addWidget(brightnessLabel, 0, 2);
    messageLayout->addWidget(saturationTLabel = new QLabel(tr("Saturation")), 1, 0);
    messageLayout->addWidget(saturationSlider, 1, 1);
    messageLayout->addWidget(saturationLabel, 1, 2);
    messageLayout->addWidget(contrastTLabel = new QLabel(tr("Contrast")), 2, 0);
    messageLayout->addWidget(contrastSlider, 2, 1);
    messageLayout->addWidget(contrastLabel, 2, 2);
    messageLayout->addWidget(gammaTLabel = new QLabel(tr("Gamma")), 3, 0);
    messageLayout->addWidget(gammaSlider, 3, 1);
    messageLayout->addWidget(gammaLabel = new QLabel("0"), 3, 2);
    messageLayout->addWidget(redTLabel = new QLabel(tr("Hue red")), 4, 0);
    messageLayout->addWidget(redSlider, 4, 1);
    messageLayout->addWidget(redLabel, 4, 2);
    messageLayout->addWidget(greenTLabel = new QLabel(tr("Hue green")), 5, 0);
    messageLayout->addWidget(greenSlider, 5, 1);
    messageLayout->addWidget(greenLabel, 5, 2);
    messageLayout->addWidget(blueTLabel = new QLabel(tr("Hue blue")), 6, 0);
    messageLayout->addWidget(blueSlider, 6, 1);
    messageLayout->addWidget(blueLabel, 6, 2);
    messageLayout->addWidget(thresholdTLabel = new QLabel(tr("Threshold")), 7, 0);
    messageLayout->addWidget(thresholdSlider, 7, 1);
    messageLayout->addWidget(thresholdLabel, 7, 2);

    messageLayout->setColumnMinimumWidth(2,20);

    // ������������ ����������� ��������
    connect(brightnessSlider, SIGNAL(valueChanged(int)), brightnessLabel,  SLOT(setNum(int)));
    connect(saturationSlider, SIGNAL(valueChanged(int)), saturationLabel,  SLOT(setNum(int)));
    connect(contrastSlider, SIGNAL(valueChanged(int)), contrastLabel,  SLOT(setNum(int)));
    connect(gammaSlider, SIGNAL(valueChanged(int)), this,  SLOT(gammaChange()));
    connect(redSlider, SIGNAL(valueChanged(int)), redLabel,  SLOT(setNum(int)));
    connect(greenSlider, SIGNAL(valueChanged(int)), greenLabel,  SLOT(setNum(int)));
    connect(blueSlider, SIGNAL(valueChanged(int)), blueLabel,  SLOT(setNum(int)));
    connect(thresholdSlider, SIGNAL(valueChanged(int)), thresholdLabel,  SLOT(setNum(int)));

    connect(contrastSlider, SIGNAL(valueChanged(int)), this,  SLOT(calcColorTable()));
    connect(gammaSlider, SIGNAL(valueChanged(int)), this,  SLOT(calcColorTable()));

    //messageLayout->setColumnStretch(2,2);
   // messageLayout->setRowStretch(5,1);
    colorGroupBox->setLayout(messageLayout);
}

void Window::gammaChange()
{
    gammaLabel->setText(QString("%1").arg(gammaSlider->value()/10.0));
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
         setSDK("ambilight");
        break;
    case 1:
       backLight();
       setSDK("backlight");
        break;
    case 2:
        moodLamp();
        setSDK("moodlamp");
        break;
    }

}

void  Window::off_ambiligth()
{
     setIcon(1);
     onAction->setEnabled(true);
     offAction->setEnabled(false);
     setSDK("off");
     onAmbiligth = false;
}

void Window::changeMode()
{
    switch (modeComboBox->currentIndex())
    {
    case 0:
        thresholdSlider->setVisible(true);
        brightnessSlider->setVisible(true);
        saturationSlider->setVisible(true);
        contrastSlider->setVisible(true);
        gammaSlider->setVisible(true);
        redSlider->setVisible(true);
        greenSlider->setVisible(true);
        blueSlider->setVisible(true);
        if (onAmbiligth) setSDK("ambilight");
        break;
    case 1:
        thresholdSlider->setVisible(false);
        brightnessSlider->setVisible(true);
        saturationSlider->setVisible(true);
        contrastSlider->setVisible(false);
        gammaSlider->setVisible(false);
        redSlider->setVisible(true);
        greenSlider->setVisible(true);
        blueSlider->setVisible(true);
        if (onAmbiligth) setSDK("backlight");
        break;
    case 2:
        thresholdSlider->setVisible(false);
        brightnessSlider->setVisible(true);
        saturationSlider->setVisible(true);
        contrastSlider->setVisible(false);
        gammaSlider->setVisible(false);
        redSlider->setVisible(false);
        greenSlider->setVisible(false);
        blueSlider->setVisible(false);
        if (onAmbiligth) setSDK("moodlamp");
        break;
    }
    thresholdLabel->setVisible(thresholdSlider->isVisible());
    thresholdTLabel->setVisible(thresholdSlider->isVisible());
    saturationLabel->setVisible(saturationSlider->isVisible());
    saturationTLabel->setVisible(saturationSlider->isVisible());
    contrastLabel->setVisible(contrastSlider->isVisible());
    contrastTLabel->setVisible(contrastSlider->isVisible());
    gammaLabel->setVisible(gammaSlider->isVisible());
    gammaTLabel->setVisible(gammaSlider->isVisible());
    redLabel->setVisible(redSlider->isVisible());
    redTLabel->setVisible(redSlider->isVisible());
    greenLabel->setVisible(greenSlider->isVisible());
    greenTLabel->setVisible(greenSlider->isVisible());
    blueLabel->setVisible(blueSlider->isVisible());
    blueTLabel->setVisible(blueSlider->isVisible());

    brightnessSlider->setValue(settings->value(QString("Brightness_%1").arg(modeComboBox->currentIndex()), 192).toInt());
    saturationSlider->setValue(settings->value(QString("Saturation_%1").arg(modeComboBox->currentIndex()), 96).toInt());
    contrastSlider->setValue(settings->value(QString("Contrast_%1").arg(modeComboBox->currentIndex()), 0).toInt());
    gammaSlider->setValue(settings->value(QString("Gamma_%1").arg(modeComboBox->currentIndex()), 10).toInt());
    redSlider->setValue(settings->value(QString("Red_%1").arg(modeComboBox->currentIndex()), 128).toInt());
    greenSlider->setValue(settings->value(QString("Green_%1").arg(modeComboBox->currentIndex()), 128).toInt());
    blueSlider->setValue(settings->value(QString("Blue_%1").arg(modeComboBox->currentIndex()), 128).toInt());
    thresholdSlider->setValue(settings->value(QString("Threshold_%1").arg(modeComboBox->currentIndex()), 0).toInt());
}

void Window::createActions()
{
    onAction = new QAction(QIcon(":/images/on.png"),tr("On ambilight"), this);
    connect(onAction, SIGNAL(triggered()), this, SLOT(on_ambiligth()));

    offAction = new QAction(QIcon(":/images/off.png"),tr("Off ambilight"), this);
    connect(offAction, SIGNAL(triggered()), this, SLOT(off_ambiligth()));

    settingsAction = new QAction(QIcon(":/images/settings.png"),tr("&Settings"), this);
    connect(settingsAction, SIGNAL(triggered()), this, SLOT(viewWindow()));

    quitAction = new QAction(QIcon(":/images/error.png"),tr("&Quit"), this);
    connect(quitAction, SIGNAL(triggered()), qApp, SLOT(quit()));
}

void Window::viewWindow()
{
    showNormal();
    changeMode();
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
    //zoneLayout->addWidget(zoneDesktopCheckBox);

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



    saveZone = new QPushButton(tr("Save"));
    zoneLayout->addWidget(saveZone,3,0,1,3);

    SpinBoxDelegate *delegate = new SpinBoxDelegate;
    tableView->setItemDelegate(delegate);
    QStringList headerLabels;
    headerLabels << tr("Left") <<tr("Top") << tr("Width") <<tr( "Heigth")<<tr("Brightness");
    model->setHorizontalHeaderLabels(headerLabels);
    tableView->horizontalHeader()->setResizeMode( QHeaderView::Stretch);
return;
}

void Window::createZones()
{
    model->removeRows(0,model->rowCount());
    QTableView *tableView = new QTableView();
    tableView->setModel(model);
    QStringList verticalLabels;
    for (int i=0;i<COUNT_AREA;++i)
       verticalLabels.append(QString("%1 %2").arg(tr("Area ")).arg(i+1));
    model->setVerticalHeaderLabels(verticalLabels);

}



