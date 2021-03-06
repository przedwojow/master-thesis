#include "algorithm.h"

/*std::ostream& operator << (std::ostream&  Strm, Type4KSearchResult Val){
    const char* Name[] = { "TK_good", "TK_ambiguous", "TK_none" };
    return Strm << Name[Val];
  }*/

Algorithm::Algorithm(QObject *parent) : QObject(parent)
{
 realMeasurement = new RealMeasurement;
}

AlgorithmResult Algorithm::findAngleByKValuesFor(int r0R1DistanceInMm, int r1R2DistanceInMm, double timeDelay1, double timeDelay2,double frequency, double epsilon) {
    AlgorithmResult result;
    int  K1_Idx, K2_Idx;
    int  K1_res = K_EMPTY_INDEX, K2_res = K_EMPTY_INDEX;
    double  AngleSin1, AngleSin2;

    for (K1_Idx = K_MIN; K1_Idx <= K_MAX; ++K1_Idx) {
        if (!CheckK_4_RecData(r0R1DistanceInMm,timeDelay1,K1_Idx,frequency, AngleSin1)) continue;
        for  (K2_Idx = K_MIN; K2_Idx <= K_MAX; ++K2_Idx) {
          if (!CheckK_4_RecData(r0R1DistanceInMm+r1R2DistanceInMm,timeDelay2,K2_Idx,frequency, AngleSin2)) continue;
          if (fabs(AngleSin1-AngleSin2) >= epsilon) continue;
          if (K1_res != K_EMPTY_INDEX) {
              cout<<"-------------------------------------------"<<endl;
            cout << ":)  Ambiguous solution found." << endl
             << "       1.  k1 = " << K1_res << "  k2 = " << K2_res << endl
             << "       2.  k1 = " << K1_Idx << "  k2 = " << K2_Idx << endl;
            cout << " ***   K1: " << K1_Idx << "    K2: " << K1_Idx << " angle " <<  RAD2DEG(asin((AngleSin1+AngleSin2)/2)) <<endl;
cout<<"------------"<<endl;
CheckK_4_RecData(r0R1DistanceInMm,timeDelay1,K1_res,frequency, AngleSin1);
CheckK_4_RecData(r0R1DistanceInMm+r1R2DistanceInMm,timeDelay2,K2_res,frequency, AngleSin2);
cout << " ***   K1res: " << K1_res << "    K2res: " << K2_res << " angle " <<  RAD2DEG(asin((AngleSin1+AngleSin2)/2)) <<endl;
cout<<"-----------------------------------------"<<endl;
//result.angle = RAD2DEG(asin((AngleSin1+AngleSin2)/2));
cout<<"new angle: " << result.angle << endl;
            return result;
          }

          K1_res = K1_Idx;  K2_res = K2_Idx;
                cout << " ***   K1k: " << K1_res << "    K2k: " << K2_res << " angle " <<  RAD2DEG(asin((AngleSin1+AngleSin2)/2)) <<endl;
          result.angle = RAD2DEG(asin((AngleSin1+AngleSin2)/2));
        }
    }
    if(K1_res != K_EMPTY_INDEX){
        result.status = TK_good;
    }else{
        result.status = TK_none;
    }
    cout<<"F new angle: " << result.angle << endl;

    return result;
}

/*!
 * A value for angle determination is computed
 */
bool Algorithm::CheckK_4_RecData(double Gap_R_mm, double DTime_us,int Ki, double frequency,double &AngleSin) {
    double actualFrequency = ( pow(10,6)/frequency);
    double  Part_Upper = URS__ULTRASONIC_WAVE_SPEED_mmUS*DTime_us + Ki*(URS__ULTRASONIC_WAVE_SPEED__mmS / actualFrequency);
    double  Base = Gap_R_mm;  // It can be < 0.

    if (fabs(Part_Upper) > fabs(Base)) return false;
    AngleSin = Part_Upper/Base;
    return true;

}

double Algorithm::correctTime( double Delta_T_us, double T_period_us)
{
  if (Delta_T_us < 0) {
    while ((Delta_T_us += T_period_us) < 0) {}
    return Delta_T_us;
  }
  if (Delta_T_us > T_period_us) {
    while ((Delta_T_us -= T_period_us) > T_period_us) {}
  }
  return Delta_T_us;
}

void Algorithm::setMainWindow(QWidget *value)
{
    mainWindow = value;
}

void Algorithm::setAlgorithmResultPlot(QwtPlot *value)
{
    algorithmResultPlot = value;
}

void Algorithm::handleRealResults()
{
    algorithmResultPlot->setAxisScale( algorithmResultPlot->xBottom, 0.0, 1500.0 );
    algorithmResultPlot->setAxisScale( algorithmResultPlot->yLeft, -90.0, 90.0);
    algorithmResultPlot->setAxisTitle(algorithmResultPlot->yLeft,"angle [degree]");
    algorithmResultPlot->setAxisTitle(algorithmResultPlot->xBottom,"time [us]");
    QwtPlotGrid *grid = new QwtPlotGrid();
    grid->attach( algorithmResultPlot );
    grid->setPen(QPen(Qt::gray));

    resultPoints.clear();
    QString filename = QFileDialog::getOpenFileName(
                mainWindow,
                tr("Open file with measurement results..."),
                "",
                "All files (*.*);;Text files (*.txt)"
                );
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)){
        return;
    }
    bool isFirstLine = true;

    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine();
        if(!line.startsWith(COMMENT_SIGN)){
            std::stringstream ss(line.toStdString());
            if(isFirstLine){
                ss >> realMeasurement->R0R1InMm >> realMeasurement->R1R2InMm;
                isFirstLine = false;
            }else{
                double T1_o_us, Delta_T2_o_us, Delta_T3_o_us, T_wave_us;
                ss >> T1_o_us >> Delta_T2_o_us >> Delta_T3_o_us >> T_wave_us;
                realMeasurement->R0Times.push_back(T1_o_us);
                realMeasurement->R1Times.push_back(Delta_T2_o_us);
                realMeasurement->R2Times.push_back(Delta_T3_o_us);
                realMeasurement->TWaves.push_back(T_wave_us);
            }
        }
    }

    for(unsigned int i=0; i< realMeasurement->R0Times.size(); i++){
        AlgorithmResult aResult = findAngleByKValuesFor(realMeasurement->R0R1InMm,realMeasurement->R1R2InMm,realMeasurement->R1Times.at(i),realMeasurement->R2Times.at(i), realMeasurement->TWaves.at(i));
std::cout<<"R "<<aResult.angle<<std::endl;
        QwtPlotMarker* m = new QwtPlotMarker();
        m->setSymbol(new QwtSymbol( QwtSymbol::Ellipse, Qt::red, Qt::NoPen, QSize( 10, 10 )));
        m->setValue( QPointF( realMeasurement->R0Times.at(i), aResult.angle ));
        std::cout<<"1"<<std::endl;
        resultPoints.push_back(new Point(realMeasurement->R0Times.at(i), aResult.angle ));
        std::cout<<"2"<<std::endl;
        m->attach( algorithmResultPlot );
        std::cout<<"3"<<std::endl;

    }
    algorithmResultPlot->replot();
    QTabWidget *tabWidget = mainWindow->findChild<QTabWidget*>("tabWidget");
    tabWidget->setCurrentIndex(tabWidget->currentIndex()  + 1 ) ;
}

void Algorithm::exportAlgorithmResultsToMatlabScript()
{
    QString filename = QFileDialog::getSaveFileName(
                mainWindow,
                tr("Save algorithm results..."),
                "algorithmResult.m",
                ".m"
                );
    if(!filename.contains((".m"))){
        filename.append(".m");
    }
    QFile file(filename);
    if (file.open(QIODevice::ReadWrite)) {
        QTextStream stream(&file);
        stream<<"figure()"<<endl;
        stream<<"grid on;"<<endl;
        stream<<"hold on;"<<endl;
         for(auto point = resultPoints.begin(); point != resultPoints.end();point++){
             stream<<"plot("<<(*point)->getX()<<","<<(*point)->getY()<<",'o','MarkerSize',12);"<<endl;
         }
         stream<<"x=[ ";
         for(auto point = resultPoints.begin(); point != resultPoints.end();point++){
             stream<<(*point)->getX()<<" ";
         }
         stream<<"];"<<endl;

         stream<<"y=[ ";
         for(auto point = resultPoints.begin(); point != resultPoints.end();point++){
             stream<<(*point)->getY()<<" ";
         }
         stream<<"];"<<endl;
         stream<<"plot(x,y);"<<endl;
    }
    file.close();

}

