#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle("PID调参工具");
    
    // 初始化串口
    serialPort = new QSerialPort(this);
    serialPortInit();
    
    // 初始化PID参数界面
    pidInit();
    
    // 连接信号和槽
    connect(ui->pushButton_openSerialPort, &QPushButton::clicked, this, &MainWindow::openSerialport);
    connect(ui->pushButton_closeSerialPort, &QPushButton::clicked, this, &MainWindow::closeSerialport);
    connect(serialPort, &QSerialPort::readyRead, this, &MainWindow::receiveData);
    connect(serialPort, &QSerialPort::errorOccurred, this, &MainWindow::handleSerialError);
    
    // 修改这里的信号连接，使用按钮来刷新
    connect(ui->pushButton_openSerialPort, &QPushButton::clicked, this, &MainWindow::refreshCom);
    connect(ui->comboBox_setBuad, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &MainWindow::setBuad);
    
    // 添加统一发送按钮连接
    connect(ui->pushButton_sendAllParams, &QPushButton::clicked, this, &MainWindow::sendAllPidParams);
    
    // 发送数据按钮连接
    connect(ui->pushButton_Send, &QPushButton::clicked, this, &MainWindow::sendData);
    
    // 更新当前值显示
    updateCurrentValueLabels();
}

MainWindow::~MainWindow()
{
    if (serialPort->isOpen())
        serialPort->close();
    delete ui;
}

// 更新当前值标签的函数
void MainWindow::updateCurrentValueLabels()
{
    ui->label_KpCurrent->setText(QString("当前: %1").arg(ui->doubleSpinBox_pidKp->value()));
    ui->label_KiCurrent->setText(QString("当前: %1").arg(ui->doubleSpinBox_pidKi->value()));
    ui->label_KdCurrent->setText(QString("当前: %1").arg(ui->doubleSpinBox_pidKd->value()));
    ui->label_TargetCurrent->setText(QString("当前: %1").arg(ui->doubleSpinBox_pidTar->value()));
}

// 统一发送PID参数的函数
void MainWindow::sendAllPidParams()
{
    if (!serialPort->isOpen()) {
        QMessageBox::warning(this, "警告", "串口未打开，无法发送参数");
        return;
    }
    
    double kp = ui->doubleSpinBox_pidKp->value();
    double ki = ui->doubleSpinBox_pidKi->value();
    double kd = ui->doubleSpinBox_pidKd->value();
    double target = ui->doubleSpinBox_pidTar->value();
    
    pidKpSet(kp);
    pidKiSet(ki);
    pidKdSet(kd);
    pidTarSet(target);
    
    // 更新当前值标签
    updateCurrentValueLabels();
    
    // 显示发送成功提示
    ui->statusbar->showMessage("PID参数发送成功", 2000);
}

/*********************************串口基本设置相关函数*********************************/
void MainWindow::serialPortInit()
{
    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts()) {
        ui->comboBox_chooseCom->addItem(info.portName());
    }
    
    // 波特率设置
    ui->comboBox_setBuad->addItem("9600");
    ui->comboBox_setBuad->addItem("19200");
    ui->comboBox_setBuad->addItem("38400");
    ui->comboBox_setBuad->addItem("57600");
    ui->comboBox_setBuad->addItem("115200");
    ui->comboBox_setBuad->setCurrentIndex(4); // 默认115200
}

void MainWindow::comboBox_buadClicked()
{
    refreshCom();
}

void MainWindow::refreshCom()
{
    ui->comboBox_chooseCom->clear();
    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts()) {
        ui->comboBox_chooseCom->addItem(info.portName());
    }
}

void MainWindow::openSerialport()
{
    if (serialPort->isOpen()) {
        return;
    }
    
    serialPort->setPortName(ui->comboBox_chooseCom->currentText());
    serialPort->setBaudRate(ui->comboBox_setBuad->currentText().toInt());
    serialPort->setDataBits(QSerialPort::Data8);
    serialPort->setParity(QSerialPort::NoParity);
    serialPort->setStopBits(QSerialPort::OneStop);
    serialPort->setFlowControl(QSerialPort::NoFlowControl);
    
    if (serialPort->open(QIODevice::ReadWrite)) {
        ui->pushButton_openSerialPort->setEnabled(false);
        ui->pushButton_closeSerialPort->setEnabled(true);
        ui->comboBox_chooseCom->setEnabled(false);
        ui->comboBox_setBuad->setEnabled(false);
    } else {
        QMessageBox::critical(this, "错误", "无法打开串口: " + serialPort->errorString());
    }
}

void MainWindow::closeSerialport()
{
    if (serialPort->isOpen()) {
        serialPort->close();
    }
    
    ui->pushButton_openSerialPort->setEnabled(true);
    ui->pushButton_closeSerialPort->setEnabled(false);
    ui->comboBox_chooseCom->setEnabled(true);
    ui->comboBox_setBuad->setEnabled(true);
}

void MainWindow::receiveData()
{
    QByteArray data = serialPort->readAll();
    
    // 解析接收到的PID数据
    pidParsingData(&data);
    
    // 显示PID波形
    showPidWave();
}

void MainWindow::sendData()
{
    if (!serialPort->isOpen()) {
        return;
    }
    
    QByteArray data = ui->textEdit_SendData->toPlainText().toLatin1();
    serialPort->write(data);
}

void MainWindow::setBuad(int index)
{
    if (serialPort->isOpen()) {
        closeSerialport();
        serialPort->setBaudRate(ui->comboBox_setBuad->currentText().toInt());
        openSerialport();
    }
}

void MainWindow::handleSerialError(QSerialPort::SerialPortError error)
{
    if (error == QSerialPort::ResourceError) {
        QMessageBox::critical(this, "串口错误", "串口发生错误: " + serialPort->errorString());
        closeSerialport();
    }
}

/*********************************PID调参相关函数*********************************/
void MainWindow::pidInit()
{
    ui->comboBox_pidShowPeriod->addItem("100");
    ui->comboBox_pidShowPeriod->addItem("1000");
    ui->comboBox_pidShowPeriod->addItem("5000");
    ui->comboBox_pidShowPeriod->setCurrentIndex(1);

    QChart *pidChart = ui->chartViewPid->chart(); // 获取ChartView关联的chart
    pidChart->setAnimationOptions(QChart::SeriesAnimations);
    ui->chartViewPid->setChart(pidChart);  // 为ChartView设置chart
    ui->chartViewPid->setRenderHint(QPainter::Antialiasing);
    ui->chartViewPid->setViewportUpdateMode(QChartView::FullViewportUpdate);

    QLineSeries *pidSeriesLineTar = new QLineSeries();
    pidSeriesLineTar->setName("目标值");
    QLineSeries *pidSeriesLineAct = new QLineSeries();
    pidSeriesLineAct->setName("实际值");

    pidChart->addSeries(pidSeriesLineTar);
    pidChart->addSeries(pidSeriesLineAct);

    // 创建x坐标
    QDateTimeAxis *pidQaX = new QDateTimeAxis;
    // 格式
    pidQaX->setFormat("mm:ss");
    pidQaX->setTickCount(10);
    pidQaX->setTitleText("时间");

    // 创建y坐标
    QValueAxis *PidQaY = new QValueAxis;
    // 设置范围
    PidQaY->setRange(-2, 8);
    PidQaY->setTickCount(5);
    PidQaY->setTitleText("值");

    // 将线条放入表中
    pidChart->setAxisX(pidQaX, pidSeriesLineTar);
    pidChart->setAxisY(PidQaY, pidSeriesLineTar);
    pidChart->setAxisX(pidQaX, pidSeriesLineAct);
    pidChart->setAxisY(PidQaY, pidSeriesLineAct);
    
    // 设置SpinBox范围和默认值
    ui->doubleSpinBox_pidKp->setRange(0, 100);
    ui->doubleSpinBox_pidKi->setRange(0, 100);
    ui->doubleSpinBox_pidKd->setRange(0, 100);
    ui->doubleSpinBox_pidTar->setRange(-10, 10);
    
    ui->doubleSpinBox_pidKp->setValue(1.0);
    ui->doubleSpinBox_pidKi->setValue(0.1);
    ui->doubleSpinBox_pidKd->setValue(0.01);
    ui->doubleSpinBox_pidTar->setValue(5.0);
}

void MainWindow::showPidWave()
{
    if (!ui->radioButton_pidShow->isChecked()) {
        return;
    }
    
    // 获取当前时间
    QDateTime currentTime = QDateTime::currentDateTime();
    // 获取初始化的qchart
    QChart *chart = ui->chartViewPid->chart();

    // 获取之前的ser
    QLineSeries *pidSeriesLineTar = (QLineSeries *)ui->chartViewPid->chart()->series().at(0);
    pidSeriesLineTar->append(currentTime.toMSecsSinceEpoch(), pidRcvTarData);

    QLineSeries *pidSeriesLineAct = (QLineSeries *)ui->chartViewPid->chart()->series().at(1);
    pidSeriesLineAct->append(currentTime.toMSecsSinceEpoch(), pidRcvActData);

    chart->axisX()->setMin(QDateTime::currentDateTime().addSecs(-1 * 30));
    chart->axisX()->setMax(QDateTime::currentDateTime().addSecs(1 * 10));
}

// cmd1为0xBA, 将cmd2和f32数据进行组包
QByteArray MainWindow::protocalDataSet(uchar cmd, double doubleData)
{
    uchar preFix = 0xA5;
    uchar len = 0x0A;
    uchar cmd1 = 0xBA;
    uchar cmd2 = cmd;
    uchar uData[4];
    uchar crc = len + cmd1 + cmd2;
    uchar postFix = 0x5A;
    QByteArray sendBuf;
    float data = static_cast<float>(doubleData);
    sendBuf.append(preFix);
    sendBuf.append(len);
    sendBuf.append(cmd1);
    sendBuf.append(cmd2);
    *(int *)&uData[0] = *(int *)&data;
    sendBuf.append(uData[0]);
    sendBuf.append(uData[1]);
    sendBuf.append(uData[2]);
    sendBuf.append(uData[3]);

    for (int i = 4; i < 8; i++) {
        crc += sendBuf.at(i);
    }
    sendBuf.append(crc);
    sendBuf.append(postFix);
    return sendBuf;
}

void MainWindow::pidKpSet(double sendKp)
{
    if (serialPort->isOpen()) {
        uchar cmd = 0x14;
        QByteArray sendBuf = protocalDataSet(cmd, sendKp);
        serialPort->write(sendBuf);
    }
}

void MainWindow::pidKiSet(double sendKi)
{
    if (serialPort->isOpen()) {
        uchar cmd = 0x15;
        QByteArray sendBuf = protocalDataSet(cmd, sendKi);
        serialPort->write(sendBuf);
    }
}

void MainWindow::pidKdSet(double sendKd)
{
    if (serialPort->isOpen()) {
        uchar cmd = 0x16;
        QByteArray sendBuf = protocalDataSet(cmd, sendKd);
        serialPort->write(sendBuf);
    }
}

void MainWindow::pidTarSet(double sendTar)
{
    if (serialPort->isOpen()) {
        uchar cmd = 0x17;
        QByteArray sendBuf = protocalDataSet(cmd, sendTar);
        serialPort->write(sendBuf);
    }
}

void MainWindow::pidParsingData(QByteArray *protocalData)
{
    uchar preFix = 0xA5;
    uchar cmd1Rcv = 0xAB;
    uchar cmd2RcvTar = 0x13;
    uchar cmd2RcvAct = 0x12;
    uchar crc = 0;
    uchar temp = 0;
    
    if (protocalData->isEmpty() || static_cast<uchar>(protocalData->at(0)) != preFix) {
        return;
    }
    
    // 检查长度是否足够
    if (protocalData->length() < 10) {
        return;
    }
    
    for (int i = 1; i < protocalData->length() - 2; i++) {
        temp = static_cast<uchar>(protocalData->at(i));
        crc += static_cast<uchar>(protocalData->at(i));
    }
    
    if (crc != static_cast<uchar>(protocalData->at(protocalData->length() - 2))) {
        return;
    }
    
    uchar cmd1 = protocalData->at(2);
    uchar cmd2 = protocalData->at(3);

    if (cmd1 == cmd1Rcv) {
        uchar uData[4];
        uData[0] = static_cast<uchar>(protocalData->at(4));
        uData[1] = static_cast<uchar>(protocalData->at(5));
        uData[2] = static_cast<uchar>(protocalData->at(6));
        uData[3] = static_cast<uchar>(protocalData->at(7));
        
        if (cmd2 == cmd2RcvTar) {
            *(int *)&pidRcvTarData = *(int *)&uData;
            ui->lcdnumber_pidTarget->display(pidRcvTarData);
        }
        
        if (cmd2 == cmd2RcvAct) {
            *(int *)&pidRcvActData = *(int *)&uData;
            ui->lcdnumber_pidActual->display(pidRcvActData);
        }
    }
}

/*********************************共用函数*********************************/
uchar MainWindow::msgProcCrc(QByteArray *protocalData)
{
    uchar crc = 0;
    
    for (int i = 1; i < protocalData->length() - 2; i++) {
        crc += static_cast<uchar>(protocalData->at(i));
    }
    
    return crc;
}

QString MainWindow::ByteArrayToHexString(QByteArray &data)
{
    QString result;
    QString temp;
    
    for (int i = 0; i < data.size(); i++) {
        temp = QString("%1").arg((uchar)data.at(i), 2, 16, QLatin1Char('0')).toUpper() + " ";
        result += temp;
    }
    
    return result;
} 