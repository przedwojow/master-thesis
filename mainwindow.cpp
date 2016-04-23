#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "paintwidget.h"
void MainWindow::createPaintWidget(list<Obstacle*> obstacles, Emitter* emitter)
{
    PaintWidget* widget = new PaintWidget;
    for(auto it=sensors.begin();it!=sensors.end();it++){
        widget->addReceiver((*it));
    }

    widget->addEmitter(emitter);

    for(auto it=obstacles.begin();it!=obstacles.end();it++){
        widget->addObstacle((*it));
    }

    ui->gridLayout->addWidget(widget);
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    simulation = new Simulation();
    console = new Console;
    signal = new Signal();

    ui->setupUi(this);

    console->setEnabled(false);
    // console->setMaximumHeight(100);
    ui->consoleLayout->addWidget(console);
    serial = new QSerialPort(this);
    status = new QLabel;
    settings = new SettingsDialog;
    matlabExporter = new MatlabExporter;

    //ui->statusBar->addWidget(status);
    ui->connectButton->setEnabled(true);
    ui->disconnectButton->setEnabled(false);
    ui->uartSettingsButton->setEnabled(true);

    setCentralWidget(ui->tabWidget);

    list<Obstacle*> obstacles;
    obstacles.push_back(new Obstacle(100,22,0));
   // obstacles.push_back(new Obstacle(140,20,0));
    obstacles.push_back(new Obstacle(70,2,0));

    sensors.push_back(new Receiver(0,0,0,0));
    sensors.push_back(new Receiver(1,0,11,0)); //11mm
    sensors.push_back(new Receiver(2,0,15,0)); //4mm


    Emitter* emitter = new Emitter(0,0,0);

    simulation->setEmitter(emitter);
    simulation->setReceivers(sensors);
    simulation->setObstacles(obstacles);

    matlabExporter->setSimulation(simulation);

    createPaintWidget(obstacles, emitter);

    initActionsConnections();
    /* plots */
    setupPlotsTab();
    setupDeltaTTab();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::openSerialPort()
{
    SettingsDialog::Settings p = settings->settings();
    serial->setPortName(p.name);
    serial->setBaudRate(p.baudRate);
    serial->setDataBits(p.dataBits);
    serial->setParity(p.parity);
    serial->setStopBits(p.stopBits);
    serial->setFlowControl(p.flowControl);
    if (serial->open(QIODevice::ReadWrite)) {
        console->setEnabled(true);
        console->setLocalEchoEnabled(p.localEchoEnabled);
        ui->disconnectButton->setEnabled(true);
        ui->connectButton->setEnabled(false);
        ui->uartSettingsButton->setEnabled(false);
        showStatusMessage(tr("Connected to %1 : %2, %3, %4, %5, %6")
                          .arg(p.name).arg(p.stringBaudRate).arg(p.stringDataBits)
                          .arg(p.stringParity).arg(p.stringStopBits).arg(p.stringFlowControl));
    } else {
        QMessageBox::critical(this, tr("Error"), serial->errorString());

        showStatusMessage(tr("Open error"));
    }
}
//! [4]

//! [5]
void MainWindow::closeSerialPort()
{
    if (serial->isOpen()){
        serial->close();
    }
    console->setEnabled(false);
    ui->disconnectButton->setEnabled(false);
    ui->connectButton->setEnabled(true);
    ui->uartSettingsButton->setEnabled(true);
    showStatusMessage(tr("Disconnected"));
}

//! [6]
void MainWindow::writeData(const QByteArray &data)
{
    serial->write(data);
}
//! [6]

//! [7]
void MainWindow::readData()
{
    QByteArray data = serial->readAll();
    console->putData(data);
    //ui->lcdNumber_3->display((ui->lcdNumber_3->value())+1);
}
//! [7]

//! [8]
void MainWindow::handleError(QSerialPort::SerialPortError error)
{
    if (error == QSerialPort::ResourceError) {
        QMessageBox::critical(this, tr("Critical Error"), serial->errorString());
        closeSerialPort();
    }
}

void MainWindow::showSignals()
{
    for(list<Receiver*>::iterator r=sensors.begin();r!=sensors.end();r++){
        (*r)->getSignal()->showSignals();
    }
}


void MainWindow::initActionsConnections()
{
    connect(ui->pushButton, &QPushButton::clicked, simulation, &Simulation::simulate);

    connect(serial, SIGNAL(error(QSerialPort::SerialPortError)), this, SLOT(handleError(QSerialPort::SerialPortError)));
    connect(serial, SIGNAL(readyRead()), this, SLOT(readData()));
    connect(console, SIGNAL(getData(QByteArray)), this, SLOT(writeData(QByteArray)));

    connect(ui->uartSettingsButton, SIGNAL(clicked()), settings, SLOT(show()));
    connect(ui->connectButton, SIGNAL(clicked()), this, SLOT(openSerialPort()));
    connect(ui->disconnectButton, SIGNAL(clicked()), this, SLOT(closeSerialPort()));
    connect(ui->clearConsoleButton, SIGNAL(clicked()), console, SLOT(clear()));

    connect(ui->startButton,SIGNAL(clicked()), this, SLOT(sendStartSignal()));
    connect(ui->stopButton,SIGNAL(clicked()), this, SLOT(sendStopSignal()));

    /* connections for plot tab */
    connect(ui->plotsSignalsButton,&QPushButton::clicked, this, &MainWindow::showSignals);
    connect(ui->exportResults,&QPushButton::clicked, matlabExporter, &MatlabExporter::exportResults);
}

void MainWindow::showStatusMessage(const QString &message)
{
    status->setText(message);
}

void MainWindow::sendStartSignal(){
    if (serial->open(QIODevice::ReadWrite)) {
        writeData("start\r\n");
    }
}

void MainWindow::sendStopSignal(){
    if (serial->open(QIODevice::ReadWrite)) {
        writeData("stop\r\n");
    }

}

void MainWindow::setupPlotsTab()
{
    for(auto r=sensors.begin();r!=sensors.end();r++){
        std::cout<<(*r)->getReceiverNumber()<<std::endl;
        QwtPlot* plot = (*r)->getSignal()->getPlot();
        createXAxisLine(plot);
        ui->plotLayout->addWidget(plot);
    }

}


void MainWindow::setupDeltaTTab()
{
    plot = new QwtPlot();
    plot->setAxisScale( plot->xBottom, 0.0, 1500.0 );
    plot->setAxisScale( plot->yLeft, -1.0, 80);
    plot->setAxisTitle(plot->xBottom,"time [us]");
    plot->setAxisTitle(plot->yLeft,"Δt [us]");
    ui->delta_T->addWidget(plot);

    simulation->setPlot(plot);


}
void MainWindow::createXAxisLine(QwtPlot* plot)
{
    QwtPlotMarker *mY = new QwtPlotMarker();
    mY->setLabelAlignment( Qt::AlignRight | Qt::AlignTop );
    mY->setLineStyle( QwtPlotMarker::HLine );
    mY->setYValue( 0.0 );
    mY->attach( plot );
}

