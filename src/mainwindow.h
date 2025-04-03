#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QTime>
#include <QTimer>
#include <QtCharts/QChartView>
#include <QtCharts/QSplineSeries>
#include <QValueAxis>
#include <QDateTimeAxis>
#include <QLineSeries>
#include <QDebug>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    /*********************************PID调参*********************************/
    float pidRcvTarData;
    float pidRcvActData;

    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    /*********************************串口基本设置相关函数*********************************/
    void serialPortInit();      // 串口初始化
    void refreshCom();          // 刷新串口
    
    /*********************************PID调参相关函数*********************************/
    void pidParsingData(QByteArray *protocalData);
    QByteArray protocalDataSet(uchar cmd, double doubleData);
    void pidInit();             // PID初始化
    void updateCurrentValueLabels(); // 更新当前值标签
    void sendAllPidParams();        // 统一发送所有PID参数

    /*********************************共用函数*********************************/
    uchar msgProcCrc(QByteArray *protocalData);
    QString ByteArrayToHexString(QByteArray &ba);

private:
    Ui::MainWindow *ui;
    QSerialPort* serialPort;

public slots:
    void comboBox_buadClicked(); // comboBox被点击

private slots:
    /*********************************串口基本设置相关函数*********************************/
    void openSerialport();      // 串口开启
    void closeSerialport();     // 串口关闭
    void sendData();            // 发送串口数据
    void receiveData();         // 接收串口数据
    void setBuad(int);          // 设置波特率
    void handleSerialError(QSerialPort::SerialPortError serialPortErr); // 串口异常捕获

    /*********************************PID调参相关函数*********************************/
    void showPidWave();
    void pidKpSet(double sendKp);
    void pidKiSet(double sendKi);
    void pidKdSet(double sendKd);
    void pidTarSet(double sendTar);
};

#endif // MAINWINDOW_H 