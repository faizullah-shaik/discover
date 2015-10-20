//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#include "RSUbcast.h"
#include <sstream>
#include <fstream>
#include <string>
using namespace std;

Define_Module(RSUbcast);

void RSUbcast::initialize(int stage) {
    BaseWaveApplLayer::initialize(stage);
    if (stage == 0) {
   //     RSUseqNum       = 0;
        RSUseqNum       = 1;    //modified by Ion: 0 value left free to distinguish beacon packets from data packets
        BcastOn         = par("BcastOn").doubleValue();
        BcastOff        = par("BcastOff").doubleValue();
        BcastInterval   = par("BcastInterval").doubleValue();
        ttl             = par("ttl");
        StopVehicle     = 20;
        payload         = par("payload").doubleValue();
        stabilizeTime   = 0.1;

  //      startBeacon = 300;

        sendDissemination = new cMessage("dissemination_evt", DISSEMINATION_BCAST);
        scheduleAt(simTime() + BcastOn +0.000000001, sendDissemination);

        /*sendStop = new cMessage("sendStop_evt", STOP_VEHICLE);
        scheduleAt(simTime() + StopVehicle, sendStop);*/

        // set a timer to trigger the writing on the file
    /*    simtime_t mySaveTimer = startBeacon + 30;
        if ( mySaveTimer > simTime()) {
            std::stringstream s;
            s << "SaveMess_" << myId;
            std::string name(s.str());
            saveMsg = new cMessage(name.c_str(), SAVE_MESSAGE);
            scheduleAt(mySaveTimer, saveMsg);
        }*/

        sim_name = ev.getConfigEx()->getActiveConfigName();
        path = "/home/ion/Desktop/DISCOVER_Results/";
        /*if (sim_name=="Managrid") path="/home/pierpaolo/Scrivania/Ion_Results/";
        if (sim_name=="Newyork1") path="/home/ion/Desktop/";
        if (sim_name=="Newyork2") path="/media/HD2/Ion_Backup/DISCOVER_DOWNLOAD/Newyork/Newyork2/StartBeacon300/";
        if (sim_name=="Newyork3") path="/media/HD2/Ion_Backup/DISCOVER_DOWNLOAD/Newyork/Newyork3/StartBeacon300/";
        if (sim_name=="Newyork4") path="/media/HD2/Ion_Backup/DISCOVER_DOWNLOAD/Newyork/Newyork4/StartBeacon300/";
        if (sim_name=="Newyork5") path="/media/HD2/Ion_Backup/DISCOVER_DOWNLOAD/Newyork/Newyork5/StartBeacon300/";*/

   /*     if (sim_name=="Roma1") path="/media/HD2/Ion_Backup/FLOODING/Roma/Roma1/StartBeacon300/";
        if (sim_name=="Roma2") path="/media/HD2/Ion_Backup/FLOODING/Roma/Roma2/StartBeacon300/";
        if (sim_name=="Roma3") path="/media/HD2/Ion_Backup/FLOODING/Roma/Roma3/StartBeacon300/";
        if (sim_name=="Roma4") path="/media/HD2/Ion_Backup/FLOODING/Roma/Roma4/StartBeacon300/";
        if (sim_name=="Roma5") path="/media/HD2/Ion_Backup/FLOODING/Roma/Roma5/StartBeacon300/";*/
    }
}

void RSUbcast::finish() {
    if (sendDissemination->isScheduled()) {
        cancelAndDelete(sendDissemination);
    }
    else {
        delete sendDissemination;
    }
    /*if (sendStop->isScheduled()) {
        cancelAndDelete(sendStop);
    }
    else {
        delete sendStop;
    }*/

    recordScalar("sentRSUpkt", RSUseqNum+1);
}

void RSUbcast::handleSelfMsg(cMessage* msg) {
    switch (msg->getKind()) {
        case DISSEMINATION_BCAST: {
            if (simTime() < BcastOff) {

                /*std::stringstream ss;
                ss << "data" << myId << "_sNum" << RSUseqNum;
                std::string name(ss.str());
                debugEV << "name: " << name <<endl;*/

                if ((RSUseqNum==1)||((simTime()-BcastOn)>=stabilizeTime)){
                    sendDown(prepareWSMext(RSUseqNum, "data", payload, type_CCH, dataPriority, 0, -1));
                    RSUseqNum = RSUseqNum+2;
                }
                //debugEV << "SUPERATO" <<endl;
                scheduleAt(simTime() + BcastInterval, sendDissemination);
                //debugEV << "SUPERATO 2" <<endl;
                //EV << "SEQNUM RSU: " << RSUseqNum <<endl;
                //RSUseqNum++;
		//RSUseqNum = RSUseqNum+2;
            }
            break;
        }
        /*case STOP_VEHICLE: {
            sendDown(prepareWSMext(RSUseqNum, "beacon", beaconLengthBits, type_CCH, beaconPriority, 0, -1));
            scheduleAt(simTime() + StopVehicle, sendStop);
            break;
        }*/

       /* case SAVE_MESSAGE: {
            if (!setOfBeacons.empty()){
                ofstream myfile;
                string path = "/home/pierpaolo/Scrivania/Ion_Results/neighbors/";
                string filename = "RSU";
                myfile.open((path+filename).c_str(), ios::app);

                for (set<string>::iterator it=setOfBeacons.begin(); it!=setOfBeacons.end(); it++){
                    myfile << *it + "\n";
                }

                myfile.close();
            }
            else{
                debugEV<< "The set of beacons is empty! Nothing to save!" <<endl;
            }

            break;
        }*/


        default: {
            if (msg)
                EV << "APP: Error: Got Self Message of unknown kind! Name: " << msg->getName() << endl;
            break;
        }
    }
}

WsmExt*  RSUbcast::prepareWSMext(unsigned long RSUseqNum, std::string name, int lengthBits, t_channel channel, int priority, int rcvId, int serial) {
    //EV << "############## SONO DENTRO PREPARE EXT" <<endl;
    WsmExt* ext = new WsmExt(name.c_str());
    ext->addBitLength(headerLength);
    ext->addBitLength(lengthBits);

    /*cModule *targetModule = getParentModule()->getSubmodule("nic")->getSubmodule("mac1609_4");
    t_channel prova = getParentModule()->getSubmodule("nic")->getSubmodule("mac1609_4")->getActiveChannel();
    debugEV << "Canale attivo al momento: " << targetModule <<endl;*/

    switch (channel) {
        case type_SCH: ext->setChannelNumber(Channels::SCH1); break; //keep it SCH1, will be rewritten at Mac1609_4 to actual Service Channel. This is just so no controlInfo is needed
        case type_CCH: ext->setChannelNumber(Channels::CCH); break;
    }
    ext->setPsid(0);
    ext->setPriority(priority);
    ext->setWsmVersion(1);
    ext->setTimestamp(simTime());
    ext->setSenderAddress(myId);
    ext->setRecipientAddress(rcvId);
    ext->setSenderPos(curPosition);
    ext->setSerial(serial);
    ext->setWsmLength(1400);
    ext->setSeqNum(RSUseqNum);
    ext->setTtl(getDefaultTTL());
  //  ext->setTtl(20);
    ext->setHopCount(0);

    debugEV << "RSU SENDING:" <<endl;
    debugEV << "\tSeqNum: " << ext->getSeqNum() << " | Channel: " << ext->getChannelNumber() << " | HopCount: " << ext->getHopCount() << " | Ttl: "<< ext->getTtl() << " | SrcAddress: " << ext->getSenderAddress() << " | SrcPos: " << ext->getSenderPos()<<endl;
    EV <<endl;

    return ext;
}

void RSUbcast::onBeacon(WaveShortMessage* wsm) {
    // Nothing! I'm the RSU ;)
    //delete wsm;
}

void RSUbcast::onData(WaveShortMessage* wsm) {

    WsmExt* ext = check_and_cast<WsmExt*>(wsm);

    if (ext->getSeqNum()==0){
        ofstream myfile;
        string path = this->path+"neighbors/";
        string filename = "RSU";
        myfile.open((path+filename).c_str(), ios::app);

        ostringstream ss1;
        ss1 << ext->getSenderAddress();
        string sender = ss1.str();

        ostringstream ss2;
        ss2 << ext->getSenderPos().x;
        string posX = ss2.str();

        ostringstream ss3;
        ss3 << ext->getSenderPos().y;
        string posY = ss3.str();

        ostringstream ss4;
        ss4 << ext->getTimestamp();
        string simtime = ss4.str();

        ostringstream ss5;
        ss5 << simTime();
        string endtime = ss5.str();

        ostringstream ss6;
        ss6 << ext->getHopCount();
        string hopcount = ss6.str();

    //    string elem = sender + "   " + posX + "   " + posY + "   " + hopcount + "   " + simtime + "   " + endtime;
    //    string elem = sender + "   " + posX + "   " + posY + "   " + simtime;
    //    setOfBeacons.insert(elem);
    //    myfile << sender + "   " + posX + "   " + posY + "   " + hopcount + "   " + simtime + "   " + endtime + "\n";
        myfile << sender + "   " + posX + "   " + posY + "   " + simtime + "\n";
        myfile.close();
    }

    if (ext->getSeqNum()==2){
        ofstream myfile;
        string path = this->path+"leaders/";
        ostringstream ss;
        ss << ext->getSenderAddress();
        string filename = "RSU";
        string toWrite = ss.str();
        myfile.open((path + filename).c_str(), ios::app);
        myfile << toWrite+"\n";
        myfile.close();
    }

}


/*Ion*/
int RSUbcast::getDefaultTTL(){
    int result = 0;
    ifstream myfile;
    string line;
    string filename = this->path+"TTL.txt";
    myfile.open(filename.c_str());
    if (myfile.is_open())
      {
        while ( getline (myfile,line) )
        {
            result = atoi(line.c_str());
        }
        myfile.close();
      }
    else debugEV<< "ERROR! CANNOT OPEN THE FILE!" <<endl;
    return result;
}
/*\Ion*/
