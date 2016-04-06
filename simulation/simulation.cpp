#include "simulation.h"

Simulation::Simulation(QObject *parent) : QObject(parent)
{

}

Simulation::~Simulation()
{

}

void Simulation::setEmitter(Object *emitter)
{
    this->emitter=emitter;
}

void Simulation::setReceivers(const std::list<Receiver*> &receivers)
{
    this->receivers=receivers;
}

void Simulation::setObstacles(const std::list<Obstacle*> &obstacles)
{
    this->obstacles=obstacles;
}

void Simulation::simulate()
{
    std::cout<<"simulate"<<std::endl;
    std::cout<<"In the simulation we have "<<this->obstacles.size()<<" obstacles and "<<this->receivers.size()<<" receivers"<<std::endl;

    for(auto r=receivers.begin();r!=receivers.end();r++){
        (*r)->resetReceiver();
        for(auto o=obstacles.begin();o!=obstacles.end();o++){

            double emitterToObstacleDistance = emitter->getDistance((*o));
            double distance = emitterToObstacleDistance + (*r)->getDistance((*o));
            double time = ((distance/100)/SPEED_OF_SOUND)*pow(10,6);
            (*r)->addTime(time);
            std::cout<<"Distance between emitter to receiver ["<<(*r)->getPositionX()<<","<<(*r)->getPositionY()<<"] through obstacle ["<<(*o)->getPositionX()<<","<<(*o)->getPositionY();
            std::cout<<"] is equal to "<<distance<<", TOF = "<<time<<"[us]"<<std::endl;
        }
        std::cout<<"-------------------"<<std::endl;
    }

    Receiver * referenceReceiver = receivers.front();
    list<double> times = referenceReceiver->getTimes();
    for(auto t=times.begin();t!=times.end();t++){
        double time = (*t);
        for(auto r=receivers.begin();r!=receivers.end();r++){
            list<double> rTimes = (*r)->getTimes();
            double maxDistanceBetweenSensorsInUs = ((referenceReceiver->getDistance((*r))/100)/SPEED_OF_SOUND)*pow(10,6);
            for(auto tt=rTimes.begin();tt!=rTimes.end();tt++){
                double timeDelay =(*tt) - time;
                if(abs(timeDelay) <= maxDistanceBetweenSensorsInUs){
                    (*r)->addDelay(timeDelay);
                    tt = rTimes.erase(tt);
                    (*r)->setTimes(rTimes);
                    break;
                }
            }
        }
        t = times.erase(t);
    }
    for(auto r=receivers.begin();r!=receivers.end();r++){
        (*r)->setTimes((*r)->getCopyTimes()); //this step is important after computing the delays to recover the TOF
        cout<<(*r)->getTimes().size();
        list<double> delays = (*r)->getDelays();
        for(auto t=delays.begin();t!=delays.end();t++){
            double v =(*t);
            std::cout<<" "<<v;
        }
        cout<<endl;
    }

    /*************************************************/
    /* DRAWING PLOTS BASED ON TOF FROM EACH RECEIVER */
    /*************************************************/

    double minTime = 999999;
    for(auto r=receivers.begin();r!=receivers.end();r++){
        list<double> rTimes = (*r)->getTimes();
        for(auto tt=rTimes.begin();tt!=rTimes.end();tt++){
            if((*tt)<minTime){
                minTime = (*tt);
            }
        }
    }
    for(auto r=receivers.begin();r!=receivers.end();r++){
        list<double> rTimes = (*r)->getTimes();
        for(auto tt=rTimes.begin();tt!=rTimes.end();tt++){
            (*r)->getSignal()->addTime((*tt)- minTime);
        }
    }
    detectZeroCrossings();
}

bool Simulation::allReceiversHaveSignals(bool *signalTable, int size)
{
    for(int i=0;i<size;i++){
        if(signalTable[i] == false){
            return false;
        }
    }

    return true;
}

void Simulation::detectZeroCrossings()
{
    // [1] method showSignals() computes
    for(auto r=receivers.begin();r!=receivers.end();r++){
        (*r)->getSignal()->showSignals();
    }

    // [2] initialize necessary
    bool* hasSignal = new bool[receivers.size()];
    double* previousProbes = new double[receivers.size()];
    for(unsigned int i=0;i<receivers.size();i++){
        hasSignal[i]=false;
        previousProbes[i]=0;
    }
    int receiverNumber;
    int signalProbesSize = receivers.front()->getSignal()->getSignalProbes().size();

    bool areReceiversHaveSignals = false;

    for(int uS=0;uS<signalProbesSize;uS++){
        // iteration over all signals and find phase delays
        receiverNumber=0;
        for(auto r=receivers.begin();r!=receivers.end();r++){
            double actualProbe = (*r)->getSignal()->getSignalProbes().at(uS);
            if(!areReceiversHaveSignals && actualProbe!=0){
                hasSignal[receiverNumber]=true;
            }

            if(allReceiversHaveSignals(hasSignal,3)){
                if(!areReceiversHaveSignals){
                    std::cout<<"Wszystkie maja sygnal!"<<std::endl;
                    areReceiversHaveSignals = true;
                }else{
                    if(previousProbes[receiverNumber] < 0 && actualProbe > 0){
                        std::cout<<"Receiver "<<receiverNumber<<" : zero crossing detection -/+ at "<<uS<<std::endl;
                    }else if(previousProbes[receiverNumber] > 0 && actualProbe < 0){
                        std::cout<<"Receiver "<<receiverNumber<<" : zero crossing detection +/- at "<<uS<<std::endl;
                    }
                    previousProbes[receiverNumber] = actualProbe;
                }

            }
            receiverNumber++;
        }

    }


    /**********************************/
    /* CLEAR ALL POINTERS             */
    /**********************************/
    delete[] hasSignal, previousProbes;
}

