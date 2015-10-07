// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

#include "veins/modules/application/discover/OBUapp.h"
#include <sstream>
#include <fstream>
#include <string>
using namespace std;

Define_Module(OBUapp);

void OBUapp::initialize(int stage) {
	BaseWaveApplLayer::initialize(stage);
	if (stage == 0) {
		//traci               = TraCIMobilityAccess().get(getParentModule());
		mobility = Veins::TraCIMobilityAccess().get(getParentModule());
		traci = mobility->getCommandInterface();
		traciVehicle = mobility->getVehicleCommandInterface();
		//MAC                 = this->getParentModule()->getSubmodule("nic")->getSubmodule("mac1609_4");
		//debugEV << "MAC: " << Mac1609_4->getActiveChannel() <<endl;
		lastDroveAt         = simTime();
	    R                   = 830;                // for 500mW transmission power, 6Mbps and 2rayInterference
	    TmaxType            = 0; //par("TmaxType");
        TmaxOriginale       = (par("Tmax").doubleValue())/1000;
	    Tmax                = TmaxOriginale;
		PktReceived         = 0;
		PktForwarded        = 0;
		PktInibition        = 0;
		PktDist             = 0;
		SpuriousForwarders  = 0;
		bcMaxEntries        = 1000;
		bcDelTime           = 1000;               // we don't want to delete entries after some time
		delayType           = 1;//par("delayType");
		txDist              = registerSignal("TXsignal");
		tmrDelay            = registerSignal("DELsignal");
        tmrForwarder        = registerSignal("FORsignal");
		distFromFwds        = 0.0;
		timeIn              = -1;
		timeOut             = 100000;
		if (delayType == 5){
		    maxDist         = 0.0;
	    }
        ReceptionsVec.setName("ReceptionsVec");
        ReceptionsVecDist.setName("ReceptionsVecDist");
        ForwardVec.setName("ForwardVec");

        width = 10100;
        height = 50;
        borderX = width/10;
        borderY = height/10;
      //  disabilitaNearBorder = 0;

        /*Ion: variables initialization*/
        BIDIRECTION = true;
        MULTIHOP_BEACON = false;
        RANDOM = false;
        GEONET = false;
        sendBeacon = new cMessage();
        beacon = true;
   //     timeToReply = 0;
   //     currentHopCount = 0;
        defaultTimer = 30;
        delayTimerType1 = 0.005; //0.002 for BIDIRECTION=true; 0.005 for BIDIRECTION=false
        delayTimerType2 = 0.1;
        defaultDistance = getDefaultDistance();
        debugEV<< "Default Distance = " << defaultDistance <<endl;
        leader = false;
        alreadyParticipated = false;
        readyToSave = true;
        startBeacon = 100;
  //      senderX = 1520;
  //      senderY = 2970;
        tempMax = 1.0;
        PktSize = 0;
        PktSizeLimit = 1400;
        TTL = getDefaultTTL();
        set<string> setOfBeacons;

        sim_name = ev.getConfigEx()->getActiveConfigName();
        if (sim_name=="Managrid") path="/home/pierpaolo/Scrivania/Ion_Results/";
        if (sim_name=="Newyork1") path="/home/ion/Desktop/";
        if (sim_name=="Newyork2") path="/media/HD2/Ion_Backup/DISCOVER_DOWNLOAD/Newyork/Newyork2/StartBeacon300/";
        if (sim_name=="Newyork3") path="/media/HD2/Ion_Backup/DISCOVER_DOWNLOAD/Newyork/Newyork3/StartBeacon300/";
        if (sim_name=="Newyork4") path="/media/HD2/Ion_Backup/DISCOVER_DOWNLOAD/Newyork/Newyork4/StartBeacon300/";
        if (sim_name=="Newyork5") path="/media/HD2/Ion_Backup/DISCOVER_DOWNLOAD/Newyork/Newyork5/StartBeacon300/";

     /*   if (sim_name=="Roma1") path="/media/HD2/Ion_Backup/FLOODING/Roma/Roma1/StartBeacon300/";
        if (sim_name=="Roma2") path="/media/HD2/Ion_Backup/FLOODING/Roma/Roma2/StartBeacon300/";
        if (sim_name=="Roma3") path="/media/HD2/Ion_Backup/FLOODING/Roma/Roma3/StartBeacon300/";
        if (sim_name=="Roma4") path="/media/HD2/Ion_Backup/FLOODING/Roma/Roma4/StartBeacon300/";
        if (sim_name=="Roma5") path="/media/HD2/Ion_Backup/FLOODING/Roma/Roma5/StartBeacon300/";*/
        /*\Ion*/

        /*Ion: Create a beacon message that will be transmitted to the node itself when the timer ends*/
       // beacon = par("beaconMessage").boolValue();
        if(beacon) {
            //defaultTimer = 100;
         //   simtime_t myBeaconTimer = startBeacon + myId*0.01;
            simtime_t myBeaconTimer = startBeacon + dblrand();
            if ( myBeaconTimer > simTime()) {
                std::stringstream ss;
                ss << "BeaconMess_" << myId;
                std::string name(ss.str());
                beaconMsg = new cMessage(name.c_str(), BEACON_MESSAGE );
                scheduleAt(myBeaconTimer, beaconMsg);
            }
        }
        /*\Ion*/

        // set a timer to trigger the writing on the file
        simtime_t mySaveTimer = startBeacon + defaultTimer;
        if ( mySaveTimer > simTime()) {
            std::stringstream s;
            s << "SaveMess_" << myId;
            std::string name(s.str());
            saveMsg = new cMessage(name.c_str(), SAVE_MESSAGE);
            scheduleAt(mySaveTimer, saveMsg);
        }

        /*Ion: clean the output directory*/
        /*if (myId==0){
            system("exec rm -r /home/pierpaolo/Scrivania/Ion_Results/leaders/*");
            system("exec rm -r /home/pierpaolo/Scrivania/Ion_Results/neighbors/*");
            system("exec rm -r /home/pierpaolo/Scrivania/Ion_Results/request/*");
            system("exec rm -r /home/pierpaolo/Scrivania/Ion_Results/TXPackets/*");
            system("exec rm -r /home/pierpaolo/Scrivania/Ion_Results/RXPackets/*");
        }
        /*\Ion*/
	}
}

void OBUapp::finish() {

    cBroadcastList::iterator it;
    for (it = bcMsgs.begin(); it != bcMsgs.end(); it++) {
        delete it->packet;
    }
    bcMsgs.clear();

    recordScalar("PktReceived", PktReceived);
    recordScalar("PktForwarded", PktForwarded);
    recordScalar("PktInibition", PktInibition);
    recordScalar("PktDist", PktDist);
    recordScalar("SpuriousForwarders", SpuriousForwarders);
    recordScalar("timeIn", timeIn);
    recordScalar("timeOut", timeOut);
}


// handles received packets
void OBUapp::onData(WaveShortMessage* wsm) { //TODO

            WsmExt* ext = check_and_cast<WsmExt*>(wsm);
            debugEV << "Pos: (" << curPosition.x << "," << curPosition.y << ")" << " | Pkt received! SeqNum: " << ext->getSeqNum() << " | HopCount: "<< ext->getHopCount() << " | WsmLength: "<< ext->getWsmLength() /*<< " | TotalLength: "<< ext->getBitLength()*/ <<endl;
            debugEV<< "TTL = " << ext->getTtl() <<endl;

            /*Ion: Verify if the received packet is a data packet or beacon*/
            if (ext->getSeqNum()==0){
                /*if MULTIHOP_BEACON=true then the multihop beaconing is triggered and the data collection is done using the basic algorithm
                 * in which every node broadcast its own beacon and forwards the beacons received from other nodes until TTL=0*/
                if (MULTIHOP_BEACON){
                    debugEV << "RECEIVED NODE'S " << ext->getSenderAddress() << " BEACON WHITH POSITION " << ext->getSenderPos().x
                            << "|" << ext->getSenderPos().y << " AND SENDING TIME " << ext->getTimestamp() <<endl;

                    //saveNeighborToList(myId, ext);

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

                    string elem = sender + "   " + posX + "   " + posY + "   " + hopcount + "   " + simtime + "   " + endtime;
                    setOfBeacons.insert(elem);

                    if (!Received(ext)){
                        if ( ext->getTtl() > 1 ) {
                            if (RANDOM){
                                std::stringstream ss;
                                ss << "expTimer_evt_node" << myId << "_sNum" << ext->getSeqNum();
                                std::string name(ss.str());
                                expiredTimer = new cMessage(name.c_str(), EXPIRED_TIMER);
                                setIdTimer(expiredTimer->getId(),ext);
                                double temp_delay = dblrand()*delayTimerType2;
                                if (temp_delay < 100000){        // if it's 100000, then it didn't pass a test
                                    debugEV << ">>>>>>>>>>>>>>>> Scheduled delay (s): " << temp_delay <<endl;
                                    scheduleAt(simTime()+temp_delay, expiredTimer);   // schedules selfMessage (duplicate of ext) after delay
                                } else {
                                    debugEV << ">>>>>>>>>>>>>>>> Delay not scheduled!" <<endl;
                                }
                            }
                            else if (GEONET){
                                std::stringstream ss;
                                ss << "expTimer_evt_node" << myId << "_sNum" << ext->getSeqNum();
                                std::string name(ss.str());
                                expiredTimer = new cMessage(name.c_str(), EXPIRED_TIMER);
                                setIdTimer(expiredTimer->getId(),ext);
                                double tempDist = ext->getSenderPos().distance(curPosition);
                                double temp_delay = delayTimerType2*(1-tempDist/R);
                                debugEV<< "Distance " << tempDist << " and Timer " << temp_delay <<endl;
                                if (temp_delay < 100000){        // if it's 100000, then it didn't pass a test
                                    debugEV << ">>>>>>>>>>>>>>>> Scheduled delay (s): " << temp_delay <<endl;
                                    if (temp_delay<0) scheduleAt(simTime(), expiredTimer);
                                    else scheduleAt(simTime()+temp_delay, expiredTimer);   // schedules selfMessage (duplicate of ext) after delay
                                } else {
                                    debugEV << ">>>>>>>>>>>>>>>> Delay not scheduled!" <<endl;
                                }
                            }
                            else{
                                std::stringstream ss;
                                ss << "expTimer_evt_node" << myId << "_sNum" << ext->getSeqNum();
                                std::string name(ss.str());
                                expiredTimer = new cMessage(name.c_str(), EXPIRED_TIMER);
                                setIdTimer(expiredTimer->getId(),ext);
                                double temp_delay = loadDelay(ext->getSeqNum());
                                if (temp_delay < 100000){        // if it's 100000, then it didn't pass a test
                                    debugEV << ">>>>>>>>>>>>>>>> Scheduled delay (s): " << temp_delay <<endl;
                                    scheduleAt(simTime()+temp_delay, expiredTimer);   // schedules selfMessage (duplicate of ext) after delay
                                } else {
                                    debugEV << ">>>>>>>>>>>>>>>> Delay not scheduled!" <<endl;
                                }
                            }
                        } else {
                            debugEV << "Max hops reached (ttl = "<< ext->getTtl() <<") -> delete message" <<endl;
                        }
                    }
                    else{
                        debugEV << "BEACON ALREADY RECEIVED! DO NOTHING!" <<endl;
                    }
                }
                else{
                    debugEV << "RECEIVED BEACON FROM NODE " << ext->getSenderAddress() << " AT POSITION " << ext->getSenderPos().x
                            << "|" << ext->getSenderPos().y << " WITH SIMTIME " << ext->getTimestamp() <<endl;
                    debugEV << "WILL WRITE BEACON IN LIST:" <<endl;
                    // writes new packet into list
                    beaconMsgs.push_back(Beacons(ext->getSenderAddress(), ext->getSenderPos().x, ext->getSenderPos().y, ext->getTimestamp()));
                    // print the list
          /*          cBeaconsList::iterator it;
                    for (it = beaconMsgs.begin(); it != beaconMsgs.end(); it++) {
                        debugEV << "SenderAddress: " << it->senderAddress << " | Posx: " << it->senderPositionX << "  | Posy: " << it->senderPositionY << " | SimTime: " << it->simulTime <<endl;
                    }*/
                    debugEV << "LIST SIZE = " << beaconMsgs.size() << endl;

                    // write neighbor to list
                    saveNeighborToList(myId, ext);
                }
            }
            /*\Ion*/

            else{
              /*if !BIDIRECTION then only the downstream phase is simulated*/
              if (!BIDIRECTION){
                debugEV<< "RECEIVED DATA PACKET FROM NODE " << ext->getSenderAddress() <<endl;

                saveRXPacket(myId, ext);

                    dist = ext->getSenderPos().distance(curPosition);

                    if (simTime() >= simulation.getWarmupPeriod()){
                        PktReceived++;
                        emit(txDist, dist);
                        ReceptionsVec.record(round(ext->getSeqNum()));
                    }

                    //check if pkt was received and ttl
                    if ( !Received(ext) ) {
                       /* if (ext->getSeqNum()>1){
                            alreadyParticipated = false;
                        }*/
                        if (ext->getSeqNum()==1){
                            if (!alreadyParticipated){
                                findHost()->getDisplayString().updateWith("r=5,green");         // if node received pkt, becomes green
                                debugEV << "NEW PACKET !" <<endl;

                                Beacons sender;
                                sender.senderAddress = ext->getSenderAddress();
                                sender.senderPositionX = ext->getSenderPos().x;
                                sender.senderPositionY = ext->getSenderPos().y;

                                if (distanceOK(curPosition.x, curPosition.y, ext->getSenderPos().x, ext->getSenderPos().y)){
                                    if (ext->getSeqNum()==1){
                                        leader = electedLeader(sender, ext);
                                    }

                                    if (leader){
                                        debugEV<< "I AM THE LEADER!!!" <<endl;
                                        if ( ext->getTtl() > 1 ) {
                                            std::stringstream ss;
                                            ss << "expTimer_evt_node" << myId << "_sNum" << ext->getSeqNum();
                                            std::string name(ss.str());
                                            expiredTimer = new cMessage(name.c_str(), EXPIRED_TIMER);
                                            setIdTimer(expiredTimer->getId(),ext);
                                            double temp_delay = loadDelay(ext->getSeqNum());
                                            if (temp_delay < 100000){                           // if it's 100000, then it didn't pass a test
                                                debugEV << ">>>>>>>>>>>>>>>> Scheduled delay (s): " << temp_delay <<endl;
                                                scheduleAt(simTime()+temp_delay, expiredTimer);                   // schedules selfMessage (duplicate of ext) after delay
                                            } else {
                                                debugEV << ">>>>>>>>>>>>>>>> Delay not scheduled!" <<endl;
                                            }
                                        } else {
                                                debugEV << "Max hops reached (ttl = "<< ext->getTtl() <<") -> delete message" <<endl;
                                        }
                                    }
                                    else{
                                        debugEV<< "NOT LEADER! DO NOTHING!" <<endl;
                                    }
                                }
                            }
                            else{
                                debugEV<< "ALREADY PARTICIPATED!!!" <<endl;
                            }
                        }
                        else{
                            if (leader){
                                debugEV<< "I AM THE LEADER!!!" <<endl;
                                if ( ext->getTtl() > 1 ) {
                                    std::stringstream ss;
                                    ss << "expTimer_evt_node" << myId << "_sNum" << ext->getSeqNum();
                                    std::string name(ss.str());
                                    expiredTimer = new cMessage(name.c_str(), EXPIRED_TIMER);
                                    setIdTimer(expiredTimer->getId(),ext);
                                    double temp_delay = loadDelay(ext->getSeqNum());
                                    if (temp_delay < 100000){                           // if it's 100000, then it didn't pass a test
                                        debugEV << ">>>>>>>>>>>>>>>> Scheduled delay (s): " << temp_delay <<endl;
                                        scheduleAt(simTime()+temp_delay, expiredTimer);                   // schedules selfMessage (duplicate of ext) after delay
                                    } else {
                                        debugEV << ">>>>>>>>>>>>>>>> Delay not scheduled!" <<endl;
                                    }
                                } else {
                                    debugEV << "Max hops reached (ttl = "<< ext->getTtl() <<") -> delete message" <<endl;
                                }
                            }
                            else{
                                debugEV<< "NOT LEADER! DO NOTHING!" <<endl;
                            }
                        }
                    }

              }
              else{
                  /*if BIDIRECTION=true then both downstream and upstream phases are evaluated*/
                  if ((ext->getSeqNum() % 2) != 0){ // packets with odd seqNum are requests

                      debugEV<< "RECEIVED REQUEST PACKET FROM NODE " << ext->getSenderAddress() <<endl;
                      dist = ext->getSenderPos().distance(curPosition);

                      if (simTime() >= simulation.getWarmupPeriod()){
                          PktReceived++;
                          emit(txDist, dist);
                          ReceptionsVec.record(round(ext->getSeqNum()));
                      }

                      //check if pkt was received and ttl
                      if ( !Received(ext) ) {
                          // the condition bellow considers the case when the RSU sends more than one packet
                          //if (ext->getSeqNum()>1){
                          //    alreadyParticipated = false;
                          //}

                          if (!alreadyParticipated){
                              debugEV << "NEW PACKET !" <<endl;
                              Beacons sender;
                              sender.senderAddress = ext->getSenderAddress();
                              sender.senderPositionX = ext->getSenderPos().x;
                              sender.senderPositionY = ext->getSenderPos().y;

                              if (distanceOK(curPosition.x, curPosition.y, ext->getSenderPos().x, ext->getSenderPos().y)){
                           //       if (ext->getSeqNum()==1){
                                  leader = electedLeader(sender, ext);
                           //       }

                                  if (leader){
                                      debugEV<< "I AM THE LEADER!!!" <<endl;

                                      if ( ext->getTtl() > 1 ) {
                                          // set a short timer in order to forward the received message
                                          std::stringstream ss;
                                          ss << "expTimer_evt_node" << myId << "_sNum" << ext->getSeqNum();
                                          std::string name(ss.str());
                                          expiredTimer = new cMessage(name.c_str(), EXPIRED_TIMER);
                                          setIdTimer(expiredTimer->getId(),ext);
                                          double temp_delay = loadDelay(ext->getSeqNum());
                                          if (temp_delay < 100000){                           // if it's 100000, then it didn't pass a test
                                              debugEV << ">>>>>>>>>>>>>>>> Scheduled delay (s): " << temp_delay <<endl;
                                              scheduleAt(simTime()+temp_delay, expiredTimer);  // schedules selfMessage (duplicate of ext) after delay
                                          } else {
                                              debugEV << ">>>>>>>>>>>>>>>> Delay not scheduled!" <<endl;
                                          }


                                          // set a (big) default timer based on the distance from RSU in order to send the reply message
                                          WsmExt* rep = ext->dup();
                                          rep->setSeqNum(ext->getSeqNum()+1);

                                          bcMsgs.push_back(Bcast(rep->getSeqNum(), rep->getSenderAddress(), 0, rep->getSenderPos().x, rep->getSenderPos().y, rep->getHopCount(), rep->dup(), 0, -1, 0, 0, 0));

                                          double myReplyTimer = tempMax*(1-((double)ext->getHopCount()/(double)TTL))+dblrand()*0.05;
                                          debugEV << "My Reply Timer = " << myReplyTimer <<endl;
                                          std::stringstream ss1;
                                          ss1 << "ReplyMess_" << myId;
                                          std::string name1(ss1.str());
                                          replyBeacon = new cMessage(name1.c_str(), REPLY_BEACON );
                                          setIdTimer(replyBeacon->getId(),rep);
                                         // timeToReply = simTime()+myReplyTimer;
                                         // currentHopCount = ext->getHopCount();
                                          scheduleAt(simTime()+myReplyTimer, replyBeacon);
                                          saveRequestPacket(myId, ext->getSenderAddress(), simTime(), myReplyTimer, ext->getHopCount());

                                      } else {
                                          if (leader){
                                              debugEV<< "MAX HOPS REACHED! SENDING REPLY MESSAGE!" <<endl;

                                              ext->setSeqNum(ext->getSeqNum()+1);

                                              bcMsgs.push_back(Bcast(ext->getSeqNum(), ext->getSenderAddress(), 0, ext->getSenderPos().x, ext->getSenderPos().y, ext->getHopCount(), ext->dup(), 0, -1, 0, 0, 0));

                                              double myReplyTimer = dblrand()*delayTimerType1;
                                              debugEV << "My Reply Timer = " << myReplyTimer <<endl;
                                              std::stringstream ss;
                                              ss << "ReplyMess_" << myId;
                                              std::string name(ss.str());
                                              replyBeacon = new cMessage(name.c_str(), REPLY_BEACON );
                                              setIdTimer(replyBeacon->getId(),ext);
                                          //    timeToReply = simTime()+myReplyTimer;
                                           //   currentHopCount = ext->getHopCount();
                                              scheduleAt(simTime()+myReplyTimer, replyBeacon);
                                          }
                                          else{
                                              debugEV << "Max hops reached (ttl = "<< ext->getTtl() <<") -> delete message" <<endl;
                                          }
                                      }
                                  }
                                  else{
                                      debugEV<< "NOT LEADER! DO NOTHING!" <<endl;
                                  }
                              }
                              else{
                                  debugEV<< "Not participating at the election. The distance is greater than D+D/2" <<endl;
                              }
                          }
                          else{
                              debugEV<< "ALREADY PARTICIPATED!!!" <<endl;
                          }
                      }
                  }

                  //packets with even seqNum are replies
                  else{
                      debugEV<< "RECEIVED REPLY PACKET FROM NODE " << ext->getSenderAddress() <<endl;

                      dist = ext->getSenderPos().distance(curPosition);

                      if (simTime() >= simulation.getWarmupPeriod()){
                          PktReceived++;
                          emit(txDist, dist);
                          ReceptionsVec.record(round(ext->getSeqNum()));
                      }

                      //check if pkt was received and ttl
                  //    if ( !Received(ext) ) {

                 //     }
         //             debugEV << "NEW REPLY PACKET!" <<endl;
                      if (leader){
                          if (readyToSave){
                              saveReplyPacket(myId, ext->getSenderAddress());
                              mergeNeighbors(myId, ext);
                          }
                          else debugEV<< "Already replied! Cannot save the sender!" <<endl;
                      }
                      else debugEV<< "Not leader! Cannot save the sender!" <<endl;

                      debugEV << "WILL WRITE IN LIST:" <<endl;
                      debugEV << "\t\tSeqNum: " << ext->getSeqNum() << " | SrcAddr: " << ext->getSenderAddress() << " | Posx: " << ext->getSenderPos().x
                              << "  | Posy: " << ext->getSenderPos().y << " | HopCount: " << ext->getHopCount() << " | DelT: " << simTime() +bcDelTime <<endl;
                              EV <<endl;

                      if (simTime() >= simulation.getWarmupPeriod()){
                          EV << "STO SCRIVENDO PKT " << round(ext->getSeqNum()) <<endl;
                          PktDist++;
                          ReceptionsVecDist.record(round(ext->getSeqNum()));
                      }
                      delay = computeDelay(ext);  // time after which node will ACTUALLY send pkt
                      if (simTime() >= simulation.getWarmupPeriod()){
                          emit(tmrDelay, delay);
                      }

                      // writes new packet into list
                      bcMsgs.push_back(Bcast(ext->getSeqNum(), ext->getSenderAddress(), simTime() +bcDelTime, ext->getSenderPos().x, ext->getSenderPos().y, ext->getHopCount(), ext->dup(), 0, -1, delay, maxDist, 0));
                  }
              }
            }
}

// fake function to ensure compatibility with BaseWaveApplLayer
void OBUapp::onBeacon(WaveShortMessage* wsm) {
	debugEV << "ON BEACON" <<endl;
    findHost()->getDisplayString().updateWith("r=5,black");
    /*if (traci->)
    traci->commandStopNode("3/0", 92, 0, 0, 10);
    delete(wsm); */
}

void OBUapp::handleSelfMsg(cMessage* msg){

    switch(msg->getKind()) {

        case EXPIRED_TIMER: {
            debugEV << ">>>>>>>>>>>>>>>> Timer elapsed" <<endl;
  //          WsmExt* loaded_ext = loadPkt(getSeqNumFromTimer(msg->getId()));
            WsmExt* loaded_ext = loadPkt(msg->getId());

            if (loaded_ext == NULL){
                debugEV<< "ALERT!!!!" <<endl;
                break;
            }

            // checks if packet was already forwarded by another node (i.e., node have already received a copy of this pkt)
            if (!alreadyForwarded(loaded_ext)){

                if (delayType == 5){
                    updateMaxDist(loaded_ext, 0);
                }

                findHost()->getDisplayString().updateWith("r=5,red");       // if node wants to forward pkt, becomes red
                debugEV << "NOT FORWARDED!" <<endl;

                if (loaded_ext->getTtl()>1){

                    if (!BIDIRECTION) loaded_ext->setTtl(loaded_ext->getTtl());
                    else {
                        if (MULTIHOP_BEACON){
                            loaded_ext->setTtl( loaded_ext->getTtl());
                        }
                        else loaded_ext->setTtl( loaded_ext->getTtl() -1);
                    }

                    loaded_ext->setHopCount(loaded_ext->getHopCount() +1);
                    /*if MULTIHOP_BEACON=true we must maintain the original sender and position*/
                    if (!MULTIHOP_BEACON){
                        loaded_ext->setSenderAddress(myId);            // myId is elaborated by BaseWaveApplLayer
                        loaded_ext->setSenderPos(curPosition);         // curPosition is elaborated by BaseWaveApplLayer, is type Coord
                    }
                    if (GEONET){
                        loaded_ext->setSenderPos(curPosition);
                    }
                    loaded_ext->setChannelNumber(Channels::CCH);

                    debugEV << "I WILL FORWARD:" <<endl;
                    debugEV << "\tSeqNum: " << loaded_ext->getSeqNum() << " | Delay: " << loadDelay(getSeqNumFromTimer(msg->getId())) << " | HopCount: " << loaded_ext->getHopCount() << " | Ttl: "<< loaded_ext->getTtl() << " | SrcAddress: " << loaded_ext->getSenderAddress() << " | SrcPos: " << loaded_ext->getSenderPos()<<endl;
                    EV <<endl;

                    debugEV << ">>>>>>>>>>>>>>>> Actually send packet to lower layers" <<endl;

                    /*send packet to lower layers*/
                    if (loaded_ext != NULL) {
                        sendDown(loaded_ext);
                        saveTXPacket(myId, loaded_ext);

                        //saveTXPacket(myId, loaded_ext);
                     /*   ostringstream ss1;
                        ss1 << loaded_ext->getSenderAddress();
                        string sender = ss1.str();

                        ostringstream ss2;
                        ss2 << loaded_ext->getSeqNum();
                        string seqNum = ss2.str();

                        ostringstream ss5;
                        ss5 << simTime();
                        string endtime = ss5.str();

                        string elem = sender + "   " + endtime + "   " + seqNum;
                        setOfTXPackets.insert(elem);*/

                        debugEV<< "The packet length at the app layer is " << loaded_ext->getWsmLength() <<endl;
                        debugEV<< "The data contained in the packet is " << loaded_ext->getWsmData() <<endl;
                        setForwardedFlag(loaded_ext->getSeqNum());

                        if (simTime() >= simulation.getWarmupPeriod()){// && (!nearBorder(1, borderX, borderY, disabilitaNearBorder)) ){
                            PktForwarded++;
                            emit(tmrForwarder, loadDelay(loaded_ext->getSeqNum() ));
                            ForwardVec.record( (int)( loaded_ext->getSeqNum() ) );
                        }
                    } else {
                        opp_error(" ############ ERROR ########### packet is NULL! Check loadDelay function");
                    }

                }
                else{
                    debugEV<<"TTL was changed! Max hops reached! NOT FORWARDED!"<<endl;
                }

            } else {
                EV << ">>>>>>>>>>>>>>>> Already forwarded!" <<endl;

                if (delayType == 5){
                    updateMaxDist(loaded_ext, 1);
                }
                delete loaded_ext;  // not needed anymore since I'm not forwarding

                if (simTime() >= simulation.getWarmupPeriod())
                    {PktInibition++;}
            }
            break;
        }


        case REPLY_BEACON: {
        //    saveNeighborsList();
            debugEV << ">>>>>>>>>>>>>>>> Timer elapsed" <<endl;
            debugEV<< "RECEIVED REPLY BEACON!" <<endl;
            readyToSave = false;
       //     WsmExt* loaded_ext = loadPkt(getSeqNumFromTimer(msg->getId()));
            WsmExt* loaded_ext = loadPkt(msg->getId());
            WsmExt* reply = loaded_ext->dup();
            if (reply == NULL){
                debugEV<< "ALERT!!!!!!!!!!!" <<endl;
                break;
            }
            reply->setTimestamp(simTime());
            reply->setSenderAddress(myId);
            reply->setSenderPos(curPosition);
            reply->setSeqNum(loaded_ext->getSeqNum());
            reply->setTtl(loaded_ext->getHopCount()+1);
            reply->setHopCount(0);
            if (PktSize <= PktSizeLimit) reply->setWsmLength(PktSize);
            else reply->setWsmLength(PktSizeLimit);
            sendDown(reply);
            setForwardedFlag(reply->getSeqNum());
            alreadyParticipated = false;  // set up the default value for this variable
            saveTXPacket(myId, reply);
            if (simTime() >= simulation.getWarmupPeriod()){// && (!nearBorder(1, borderX, borderY, disabilitaNearBorder)) ){
                PktForwarded++;
                emit(tmrForwarder, loadDelay(reply->getSeqNum() ));
                ForwardVec.record( (int)( reply->getSeqNum() ) );
            }
            break;
        }


        case PERMISSION_TO_SEND: {
            debugEV << "RECEIVED THE PERMISSION TO FORWARD!" <<endl;
       //     WsmExt* loaded_ext = loadPkt(getSeqNumFromTimer(msg->getId()));
            WsmExt* loaded_ext = loadPkt(msg->getId());
            WsmExt* rep = loaded_ext->dup();
            bool b = false;

            cBroadcastList::iterator elem;
            for (elem=bcMsgs.begin(); elem!=bcMsgs.end(); elem++){
                if (elem->seqNum==loaded_ext->getSeqNum()){
                    if (elem->copies>0) b=true;
                }
            }

            if (!b){
                if (!alreadyForwarded(loaded_ext)){

                    if (delayType == 5){
                        updateMaxDist(loaded_ext, 0);
                    }

                    findHost()->getDisplayString().updateWith("r=5,red");       // if node wants to forward pkt, becomes red
                    debugEV << "NOT FORWARDED!" <<endl;

                    if (loaded_ext->getTtl()>1){
                        leader = true;
                        if (!BIDIRECTION) loaded_ext->setTtl( loaded_ext->getTtl());
                        else loaded_ext->setTtl( loaded_ext->getTtl() -1);
                        loaded_ext->setHopCount(loaded_ext->getHopCount() +1);
                        loaded_ext->setSenderAddress(myId);            // myId is elaborated by BaseWaveApplLayer
                        loaded_ext->setSenderPos(curPosition);         // curPosition is elaborated by BaseWaveApplLayer, is type Coord
                        loaded_ext->setChannelNumber(Channels::CCH);

                        debugEV << "I WILL FORWARD:" <<endl;
                        debugEV << "\tSeqNum: " << loaded_ext->getSeqNum() << " | Delay: " << loadDelay(getSeqNumFromTimer(msg->getId())) << " | HopCount: " << loaded_ext->getHopCount() << " | Ttl: "<< loaded_ext->getTtl() << " | SrcAddress: " << loaded_ext->getSenderAddress() << " | SrcPos: " << loaded_ext->getSenderPos()<<endl;
                                EV <<endl;

                        debugEV << ">>>>>>>>>>>>>>>> Actually send packet to lower layers" <<endl;

                        if (loaded_ext != NULL) {
                            sendDown(loaded_ext);
                            setForwardedFlag(loaded_ext->getSeqNum());
                            saveTXPacket(myId, loaded_ext);
                            if (simTime() >= simulation.getWarmupPeriod()){// && (!nearBorder(1, borderX, borderY, disabilitaNearBorder)) ){
                                PktForwarded++;
                                emit(tmrForwarder, loadDelay(loaded_ext->getSeqNum() ));
                                ForwardVec.record( (int)( loaded_ext->getSeqNum() ) );
                            }

                        } else {
                            opp_error(" ############ ERROR ########### packet is NULL! Check loadDelay function");
                        }

                        // set a (big) default timer based on the distance from RSU in order to send the reply message
                        if (BIDIRECTION){

                            if (!beaconMsgs.empty()){

                                rep->setSeqNum(loaded_ext->getSeqNum()+1);

                                bcMsgs.push_back(Bcast(rep->getSeqNum(), rep->getSenderAddress(), 0, rep->getSenderPos().x, rep->getSenderPos().y, rep->getHopCount(), rep->dup(), 0, -1, 0, 0, 0));

                                double myReplyTimer = tempMax*(1-((double)rep->getHopCount()/(double)TTL))+dblrand()*0.05;
                                debugEV << "My Reply Timer = " << myReplyTimer <<endl;
                                std::stringstream ss1;
                                ss1 << "ReplyMess_" << myId;
                                std::string name1(ss1.str());
                                replyBeacon = new cMessage(name1.c_str(), REPLY_BEACON );
                                setIdTimer(replyBeacon->getId(),rep);
                             //   timeToReply = simTime()+myReplyTimer;
                             //   currentHopCount = ext->getHopCount();
                                scheduleAt(simTime()+myReplyTimer, replyBeacon);
                                saveRequestPacket(myId, rep->getSenderAddress(), simTime(), myReplyTimer, rep->getHopCount());
                            }
                            else{
                                debugEV<< "I have no neighbors! Reply beacon not set!" <<endl;
                            }
                        }
                    }
                    else{
                        debugEV<<"TTL was changed! Max hops reached! NOT FORWARDED!"<<endl;
                    }

                } else {
                    EV << ">>>>>>>>>>>>>>>> Already forwarded!" <<endl;

                    if (delayType == 5){
                        updateMaxDist(loaded_ext, 1);
                    }
                    delete loaded_ext;  // not needed anymore since I'm not forwarding

                    if (simTime() >= simulation.getWarmupPeriod())
                         {PktInibition++;}
                }
            }
            else{
                debugEV<< "NO PERMISSION TO FORWARD! RECEIVED MORE THAN 1 COPY!" <<endl;
            }

            break;
        }


        /*Ion: Handle the beacon message*/
        case BEACON_MESSAGE: {
            debugEV<< "BEACON SELF MESSAGE RECEIVED" <<endl;
            WsmExt* beacon_to_send = new WsmExt("data");
            beacon_to_send->addBitLength(headerLength);
            beacon_to_send->addBitLength(0);
            beacon_to_send->setChannelNumber(Channels::CCH);
            beacon_to_send->setPsid(0);
            beacon_to_send->setPriority(dataPriority);
            beacon_to_send->setWsmVersion(1);
            beacon_to_send->setTimestamp(simTime());
            beacon_to_send->setSenderAddress(myId);
            beacon_to_send->setRecipientAddress(0);
            beacon_to_send->setSenderPos(curPosition);
            beacon_to_send->setSerial(-1);
            beacon_to_send->setSeqNum(0);
           // beacon_to_send->setWsmLength(5);
            if (!MULTIHOP_BEACON) beacon_to_send->setTtl(1);
            else beacon_to_send->setTtl(getDefaultTTL());
            beacon_to_send->setHopCount(0);
            beacon_to_send->setWsmLength(14);
            sendDown(beacon_to_send);
            debugEV<< "The packet length at the app layer is " << beacon_to_send->getWsmLength() <<endl;
            debugEV<< "The data contained in the packet is " << beacon_to_send->getWsmData() <<endl;
            saveTXPacket(myId, beacon_to_send);
        /*    ostringstream ss1;
            ss1 << beacon_to_send->getSenderAddress();
            string sender = ss1.str();

            ostringstream ss2;
            ss2 << beacon_to_send->getSeqNum();
            string seqNum = ss2.str();

            ostringstream ss5;
            ss5 << simTime();
            string endtime = ss5.str();

            string elem = sender + "   " + endtime + "   " + seqNum;
            setOfTXPackets.insert(elem);*/

            beaconMsgs.push_back(Beacons(beacon_to_send->getSenderAddress(), beacon_to_send->getSenderPos().x, beacon_to_send->getSenderPos().y, beacon_to_send->getTimestamp()));
            if (MULTIHOP_BEACON){
                bcMsgs.push_back(Bcast(beacon_to_send->getSeqNum(), beacon_to_send->getSenderAddress(), simTime() +bcDelTime, beacon_to_send->getSenderPos().x, beacon_to_send->getSenderPos().y, beacon_to_send->getHopCount(), beacon_to_send->dup(), 0, -1, delay, maxDist, 0));
            }
            debugEV<< "BEACON MESSAGE BROADCASTED " << curPosition.x << "|" << curPosition.y <<endl;

            /*the module bellow is needed to set a fixed periodical beacon exchange... uncomment if necessary*/
      /*      simtime_t myBeaconTimer = msg->getArrivalTime() + defaultTimer;
            if ( myBeaconTimer > simTime()) {
                std::stringstream ss;
                ss << "BeaconMess_" << myId;
                std::string name(ss.str());
                beaconMsg = new cMessage(name.c_str(), BEACON_MESSAGE );
                scheduleAt(myBeaconTimer, beaconMsg);
            }
            debugEV<< "BEACON SELF MESSAGE SCHEDULED TO SEND AFTER TIMER EXPIRES" <<endl;
            debugEV<< "Beacons list size before updating = " << beaconMsgs.size() <<endl;
            debugEV<< "UPDATING THE BEACONS LIST" <<endl;
            updateBeaconsList(beaconMsgs, simTime());
            debugEV<< "Beacons list size after updating = " << beaconMsgs.size() <<endl;*/
            break;
        }
        /*\Ion*/


        case SAVE_MESSAGE: {
            if (!setOfBeacons.empty()){
                ofstream myfile;
                string path = this->path+"neighbors/";
                ostringstream ss;
                ss << myId;
                string filename = ss.str();
                myfile.open((path+filename).c_str(), ios::app);

                for (set<string>::iterator it=setOfBeacons.begin(); it!=setOfBeacons.end(); it++){
                    myfile << *it + "\n";
                }

                myfile.close();
            }
            else{
                debugEV<< "The set of beacons is empty! Nothing to save!" <<endl;
            }

            if (!setOfTXPackets.empty()){
                ofstream myfile1;
                string path = this->path+"TXPackets/";
                ostringstream ss1;
                ss1 << myId;
                string filename = ss1.str();
                myfile1.open((path+filename).c_str(), ios::app);

                for (set<string>::iterator it=setOfTXPackets.begin(); it!=setOfTXPackets.end(); it++){
                    myfile1 << *it + "\n";
                }

                myfile1.close();
            }
            else{
                debugEV<< "The set of TX packets is empty! Nothing to save!" <<endl;
            }

            break;
        }


        default: {
            debugEV << "SelfMessage non riconosciuto!" <<endl;
            break;
        }
    }
}

void OBUapp::updateMaxDist(WsmExt* ext, short int f){
    cBroadcastList::iterator it;
    double alpha = 0.125;

    for (it = bcMsgs.begin(); it != bcMsgs.end(); it++) {
        if (it->seqNum == ext->getSeqNum()) {
             debugEV << "ext->getSenderPos().x: " << ext->getSenderPos().x << "\tit->posx: " << it->posx <<endl;
             if (f == 1) {          // I'm inhibited, calculate dist between senders
                 distFromFwds = sqrt( pow((ext->getSenderPos().x - it->posx),2) + pow((ext->getSenderPos().y - it->posy),2) );
                 break;
             } else {               // I'm the forwarder, calculate dist from me
                 distFromFwds = sqrt( pow((ext->getSenderPos().x - curPosition.x),2) + pow((ext->getSenderPos().y - curPosition.y),2) );
                 break;
             }
        }
    }
    maxDist = alpha*maxDist + ((1-alpha)*distFromFwds);
    debugEV << "distFromFwds: " << distFromFwds << "\tmaxDist: " << maxDist <<endl;
}


// calls BaseWaveApplLayer function to update node position, plus (commented) how to trigger pkt sending by broken down vehicle TODO
void OBUapp::handlePositionUpdate(cObject* obj) {
    findHost()->getDisplayString().updateWith("r=5,white");
    BaseWaveApplLayer::handlePositionUpdate(obj);
    lastDroveAt = simTime();
    if ( (Tmax != TmaxOriginale) && (TmaxType == 1) ){
            debugEV << "RESETTO IL MIO TMAX! prima: " << Tmax  <<endl;
            Tmax = TmaxOriginale;
            debugEV << "RESETTO IL MIO TMAX! dopo: " << Tmax <<endl;
    }
    if (timeIn == -1 && (curPosition.x > borderX && curPosition.x < width-borderX) ) { timeIn = simTime(); }//debugEV << "Scrivo timeIn: " << timeIn <<endl;}
    if ( (curPosition.x < borderX || curPosition.x > width-borderX) && timeOut == 100000 && timeIn != -1) { timeOut = simTime(); }//debugEV << "Scrivo timeOut: " << timeOut <<endl;}
}


// checks if arrived pkt was already received
bool OBUapp::Received(WsmExt* ext) {

    debugEV << "INTO RECEIVED FUNCTION!" <<endl;
    cBroadcastList::iterator it;

    for (it = bcMsgs.begin(); it != bcMsgs.end(); it++) {

        if (it->seqNum == ext->getSeqNum()) { // if TRUE, message was already received
            findHost()->getDisplayString().updateWith("r=5,blue");

            /*if MULTIHOP_BEACON=true then only the code inside this conditional must be executed and then return true*/
            if (MULTIHOP_BEACON){
                if (it->srcAddr == ext->getSenderAddress()){
                    it->packet = ext->dup();
                    it->copies++;
                    return true;
                }
            }
            else{
            /*    if (it->hopCount == ext->getHopCount()){
                    debugEV << "PACKET " << ext->getSeqNum() << " COMING FROM SPURIOUS FORWARDER! (same hopCount) - discarding." << "\tsavedHopCount: " << it->hopCount << "\tfrom ID: " << it->srcAddr << "\tarrivedHopCount: " << ext->getHopCount() << "\tfrom ID: " << ext->getSenderAddress() <<endl;
                    if (simTime() >= simulation.getWarmupPeriod()){// && (!nearBorder(1, borderX, borderY, disabilitaNearBorder)) ){
                        SpuriousForwarders++;
                    }

                } else */
                    if (it->copies > -1) {
                        debugEV << "UPDATING PACKET "<< ext->getSeqNum() << "'s COPY" <<endl;

                        int tempTtl = it->packet->getTtl();     //added by Ion
                        int tempHopCount = it->packet->getHopCount();   // added by Ion
                       // double tempX = it->posx;
                       // double tempY = it->posy;
                        delete it->packet;   // delete first to get rid of undisposed objects
                        it->packet = ext->dup();
                        it->copies++;

                        if (BIDIRECTION){
                            if (distanceOK(curPosition.x, curPosition.y, ext->getSenderPos().x, ext->getSenderPos().y)){
                                debugEV<< "The distance is ok!" <<endl;
                                //if (!collinear(tempX, tempY, ext->getSenderPos().x, ext->getSenderPos().y, curPosition.x, curPosition.y)){
                                if (alreadyParticipated){
                                    debugEV<< "I have already participated at the election. Not updating the HopCount and TTL!" <<endl;
                                    it->packet->setTtl(tempTtl);
                                    it->packet->setHopCount(tempHopCount);
                                }
                                else{
                                    debugEV<< "I have not participated at the election. Updating HopCount and TTL!" <<endl;
                                }
                            }
                            else{
                                debugEV<< "The distance is not ok. Not updating the HopCount and TTL!" <<endl;
                                it->packet->setTtl(tempTtl);
                                it->packet->setHopCount(tempHopCount);
                            }

                            debugEV<< "Temp TTL = " << tempTtl <<endl;
                            debugEV<< "Current TTL = " << it->packet->getTtl() <<endl;
                            debugEV<< "Temp Hop Count = " << tempHopCount <<endl;
                            debugEV<< "Current Hop Count = " << it->packet->getHopCount() <<endl;
                        }

                        if  (didIForwardIt(it->seqNum)) {
                            debugEV << "§§§§§ I DID! " <<endl; //TODO
                        }

                        if (ext->getSeqNum()==1){
                            debugEV<< "Node " << myId << " (" << curPosition.x << "|" << curPosition.y << ") and Node "
                                    << ext->getSenderAddress() << " (" << ext->getSenderPos().x << "|" << ext->getSenderPos().y << ")" <<endl;
                            if (distanceOK(curPosition.x, curPosition.y, ext->getSenderPos().x, ext->getSenderPos().y)){
                                return false;
                            }
                        }
                }
                return true;
            }
        }
    }

    //delete oldest entry if max size is reached
    if (bcMsgs.size() >= bcMaxEntries) {
        debugEV<<"bcMsgs is full, delete oldest entry" <<endl;
        bcMsgs.pop_front();
    }

    debugEV << "WILL WRITE IN LIST:" <<endl;
    debugEV << "\t\tSeqNum: " << ext->getSeqNum() << " | SrcAddr: " << ext->getSenderAddress() << " | Posx: " << ext->getSenderPos().x
            << "  | Posy: " << ext->getSenderPos().y << " | HopCount: " << ext->getHopCount() << " | DelT: " << simTime() +bcDelTime <<endl;
    EV <<endl;

    if (simTime() >= simulation.getWarmupPeriod()){
        EV << "STO SCRIVENDO PKT " << round(ext->getSeqNum()) <<endl;
        PktDist++;
        ReceptionsVecDist.record(round(ext->getSeqNum()));
    }
    delay = computeDelay(ext);    // time after which node will ACTUALLY send pkt
    if (simTime() >= simulation.getWarmupPeriod()){
        emit(tmrDelay, delay);
    }

    // writes new packet into list
    bcMsgs.push_back(Bcast(ext->getSeqNum(), ext->getSenderAddress(), simTime() +bcDelTime, ext->getSenderPos().x, ext->getSenderPos().y, ext->getHopCount(), ext->dup(), 0, -1, delay, maxDist, 0));
    return false;
}

// set forwarded flag to 1
void OBUapp::setForwardedFlag(unsigned long seqNum) {
    cBroadcastList::iterator it;
    for (it = bcMsgs.begin(); it != bcMsgs.end(); it++) {
        if (it->seqNum == seqNum ){
            it->forwarded = 1;
            return;
        }
    }
}

bool OBUapp::didIForwardIt(unsigned long seqNum){
    cBroadcastList::iterator it;

    //search the broadcast list for already forwarded pkts
    for (it = bcMsgs.begin(); it != bcMsgs.end(); it++) {
        if (it->seqNum == seqNum ) {
            return it->forwarded;
        }
    }
}


// checks if packet has been already forwarded by another node (ie, copies > 0)
bool OBUapp::alreadyForwarded(WsmExt* ext) {
    cBroadcastList::iterator it;
    bool already = false;
    debugEV << "INTO FORWARDED FUNCTION!" <<endl;
    if (MULTIHOP_BEACON){
        if (RANDOM || GEONET){
            debugEV<< "Verify if beacon " << ext->getSenderAddress() << " was not already forwarded!" <<endl;
            debugEV<< "Beacons received so far with more than 1 copy:" <<endl;
            for (it = bcMsgs.begin(); it != bcMsgs.end(); it++) {
                if ( (it->seqNum == ext->getSeqNum() && it->copies > 0) ) {
                    debugEV<< it->srcAddr <<endl;
                    if (it->srcAddr == ext->getSenderAddress()){
                        debugEV << "NODE'S " << it->srcAddr << " BEACON WAS ALREADY FORWARDED! DO NOTHING!" <<endl;
                        return true;
                    }
                }
            }
            return false;
        }
        else return false;
    }
    for (it = bcMsgs.begin(); it != bcMsgs.end(); it++) {
        //message was already forwarded
            if ( (it->seqNum == ext->getSeqNum() && it->copies > 0) ) {
                if (!leader){
                    debugEV << "\tPacket " << it->seqNum << " already forwarded or with same hopCount! \tcopies: " << it->copies << "\tsaved hopCount: " << it->hopCount << "\tarrived hopCount: " << ext->getHopCount() <<endl;
                    already = true;
                    break;
                }
            }
    }
    return already;
}

/*WsmExt* OBUapp::loadPkt(unsigned long seqNum) {
    cBroadcastList::iterator it;
    WsmExt * pkt = NULL;
    for (it = bcMsgs.begin(); it != bcMsgs.end(); it++) {
        if (seqNum == it->seqNum) {
            pkt = (it->packet)->dup();
        }
    }
    return pkt;
}*/


WsmExt* OBUapp::loadPkt(unsigned long idTmr) {
    cBroadcastList::iterator it;
    WsmExt * pkt = NULL;
    for (it = bcMsgs.begin(); it != bcMsgs.end(); it++) {
        if (idTmr == it->idTimer) {
            pkt = (it->packet)->dup();
        }
    }
    return pkt;
}


double OBUapp::loadDelay(unsigned long seqNum) {
    cBroadcastList::iterator it;
    double dly = -1;

    for (it = bcMsgs.begin(); it != bcMsgs.end(); it++) {
        if (seqNum == it->seqNum) {
            dly = it->delayTimer;
        }
    }
    return dly;
}

void OBUapp::setIdTimer(unsigned long timer, WsmExt* ext){
    cBroadcastList::iterator it;

    for (it = bcMsgs.begin(); it != bcMsgs.end(); it++) {
        if (MULTIHOP_BEACON){
            if (ext->getSenderAddress() == it->srcAddr){
                it->idTimer = timer;
                debugEV << "MY PACKET LIST: " <<endl;
                for (it = bcMsgs.begin(); it != bcMsgs.end(); it++) {
                    debugEV << "\t\tseqNum: " << it->seqNum << "\t\thopCount: " << it->hopCount << "\t\tcopies: " << it->copies << "\t\tidTimer: " << it->idTimer << "\t\tdelay: " << it->delayTimer << "\t\tcurrent maxDist: " << it->currentMaxDist << "\t\tForwarded: " << it->forwarded <<endl;
                }
                EV <<endl;
                break;
            }
        }
        else{
            if (ext->getSeqNum() == it->seqNum) {
                it->idTimer = timer;
                debugEV << "MY PACKET LIST: " <<endl;
                for (it = bcMsgs.begin(); it != bcMsgs.end(); it++) {
                    debugEV << "\t\tseqNum: " << it->seqNum << "\t\thopCount: " << it->hopCount << "\t\tcopies: " << it->copies << "\t\tidTimer: " << it->idTimer << "\t\tdelay: " << it->delayTimer << "\t\tcurrent maxDist: " << it->currentMaxDist << "\t\tForwarded: " << it->forwarded <<endl;
                }
                EV <<endl;
                break;
            }
        }
    }
}

unsigned long OBUapp::getSeqNumFromTimer(unsigned long idTmr){
    cBroadcastList::iterator it;
    unsigned long sqnm = -1;

    for (it = bcMsgs.begin(); it != bcMsgs.end(); it++) {
        if (idTmr == it->idTimer) {
            sqnm = it->seqNum;
        }
    }
    return sqnm;
}


/*Ion: a method to update the beacons list*/
void OBUapp::updateBeaconsList(cBeaconsList &myList, simtime_t time){;
    if (!myList.empty()){
        cBeaconsList::iterator it;
        for (it = myList.begin(); it != myList.end();) {
            if ((time - it->simulTime)>10){        // the number 10 depends on the maximum number of cars, in this case 500 cars
                it = myList.erase(it);
            }
            else
                ++it;
        }
    }
}
/*\Ion*/


/*Ion: a method to elect the leader*/
bool OBUapp::electedLeader(Beacons sender, WsmExt* ext){
    cBeaconsList::iterator it;
    double counter = 0.0;
    double tempDist = 0.0;
    std::vector<myPair> neighborsList;
    Beacons myself;
    alreadyParticipated = true;

    if (beaconMsgs.empty()){
        debugEV<< "I HAVE NO NEIGHBORS!!!" <<endl;
        return false;
    }

    for (it=beaconMsgs.begin(); it!=beaconMsgs.end(); it++){
        if (it->senderAddress==myId) myself = *it;
        tempDist = abs(sqrt( pow((sender.senderPositionX - it->senderPositionX),2) + pow((sender.senderPositionY - it->senderPositionY),2))-defaultDistance);
        neighborsList.push_back(myPair(*it,tempDist));
    }

    std::sort(neighborsList.begin(), neighborsList.end(), sortVector());

    std::vector<myPair>::iterator iter;

    for (iter=neighborsList.begin(); iter!=neighborsList.end(); iter++){
        debugEV<< "Node " << iter->first.senderAddress << " is at distance " << iter->second <<endl;
        counter++;
        if (iter->first.senderAddress==myself.senderAddress) break;
    }

    for (iter=neighborsList.begin(); iter!=neighborsList.end(); iter++){
        debugEV<< "The node at the minimum distance is " << iter->first.senderAddress << " with distance " << iter->second <<endl;

        if (iter->first.senderAddress == myself.senderAddress){
            return true;
        }

        if (distanceOK(myself.senderPositionX, myself.senderPositionY, iter->first.senderPositionX, iter->first.senderPositionY)){

            if (collinear(sender.senderPositionX, sender.senderPositionY, iter->first.senderPositionX, iter->first.senderPositionY, myself.senderPositionX, myself.senderPositionY)){
                debugEV<< "WE ARE COLLINEAR!" <<endl;
                debugEV<< "NOT LEADER!" <<endl;

                //if (counter<=30){
                if (iter->second<=(defaultDistance/2)){
                    if ( ext->getTtl() > 1 ) {
                        std::stringstream ss;
                        ss << "sendBeacon_evt_node" << myId << "_sNum" << ext->getSeqNum();
                        std::string name(ss.str());
                        sendBeacon->setName(name.c_str());
                        sendBeacon->setKind(PERMISSION_TO_SEND);
                        setIdTimer(sendBeacon->getId(),ext);
                        debugEV<< "COUNTER = " << counter <<endl;
                        double temp_delay = (counter-1)*delayTimerType1+0.002;
                        if (temp_delay < 100000){                           // if it's 100000, then it didn't pass a test
                            debugEV << ">>>>>>>>>>>>>>>> Scheduled delay (s): " << temp_delay <<endl;
                            scheduleAt(simTime()+temp_delay, sendBeacon);                   // schedules selfMessage (duplicate of ext) after delay
                        } else {
                            debugEV << ">>>>>>>>>>>>>>>> Delay not scheduled!" <<endl;
                        }
                    } else {
                        debugEV << "Max hops reached (ttl = "<< ext->getTtl() <<") -> delete message" <<endl;
                    }
                }
                return false;
            }
        }
    }
}
/*\Ion*/


/*Ion: a method to save to file the reply packet*/
void OBUapp::saveReplyPacket(int parentNode, int replyNode){
    ofstream myfile;
    string path = this->path+"leaders/";
    ostringstream ss1;
    ostringstream ss2;
    ss1 << parentNode;
    ss2 << replyNode;
    string filename = ss1.str();
    string toWrite = ss2.str();
    myfile.open((path + filename).c_str(), ios::app);
    myfile << toWrite+"\n";
    myfile.close();
}
/*\Ion*/


/*Ion: a method to save to file the request packet*/
void OBUapp::saveRequestPacket(int parentNode, int requestNode, simtime_t actualTime, double delayTime, int hopCount){
    ofstream myfile;
    string path = this->path+"request/";
    ostringstream ss1;
    ostringstream ss2;
    ostringstream ss3;
    ostringstream ss4;
    ostringstream ss5;
    ss1 << parentNode;
    ss2 << requestNode;
    ss3 << actualTime;
    ss4 << delayTime;
    ss5 << hopCount;
    string filename = ss1.str();
    string node = ss2.str();
    string time1 = ss3.str();
    string time2 = ss4.str();
    string hop = ss5.str();
    myfile.open((path + filename).c_str(), ios::app);
    myfile << node + " " + time1 + " " + time2 + " " + hop + "\n";
    myfile.close();
}
/*\Ion*/


/*Ion: a method to save to file the received packet*/
/*void OBUapp::saveNeighborsList(){
    ofstream myfile;
    string path = "/home/ion/Documents/omnetpp-4.3.1/ion/mixim/output/neighbors/";
    ostringstream ss;
    ss << myId;
    string filename = ss.str();
    myfile.open((path+filename).c_str());

    if (!beaconMsgs.empty()){
        cBeaconsList::iterator it;
        for (it=beaconMsgs.begin(); it!=beaconMsgs.end(); it++){
            ostringstream ss1;
            ostringstream ss2;
            ostringstream ss3;
            ostringstream ss4;
            ss1 << it->senderAddress;
            ss2 << it->senderPositionX;
            ss3 << it->senderPositionY;
            ss4 << it->simulTime;
            string id = ss1.str();
            string posx = ss2.str();
            string posy = ss3.str();
            string simtime = ss4.str();
            myfile << id + "   " + posx + "   " + posy + "   " + simtime + "\n";
        }
    }
    else {
        myfile << "No neighbors";
    }
    myfile.close();
}*/
/*\Ion*/


/*Ion*/
void OBUapp::saveNeighborToList(int node, WsmExt* ext){
    debugEV<< "Inside saving method!!!" <<endl;
    ofstream myfile;
    string path = this->path+"neighbors/";
    ostringstream ss;
    ss << node;
    string filename = ss.str();
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

    if (MULTIHOP_BEACON){
        myfile << sender + "   " + posX + "   " + posY + "   " + hopcount + "   " + simtime + "   " + endtime + "\n";
    }
    else if (RANDOM  || GEONET){
        myfile << sender + "   " + posX + "   " + posY + "   " + hopcount + "   " + simtime + "   " + endtime + "\n";
    }
    else
        myfile << sender + "   " + posX + "   " + posY + "   " + simtime + "\n";
    myfile.close();
}
/*\Ion*/


/*Ion*/
void OBUapp::saveTXPacket(int node, WsmExt* ext){
    ofstream myfile;
    string path = this->path+"TXPackets/";
    ostringstream ss;
    ss << node;
    string filename = ss.str();
    myfile.open((path+filename).c_str(), ios::app);

    ostringstream ss1;
    ss1 << ext->getSenderAddress();
    string sender = ss1.str();

    ostringstream ss2;
    ss2 << ext->getSeqNum();
    string seqNum = ss2.str();

    ostringstream ss5;
    ss5 << simTime();
    string endtime = ss5.str();

    myfile << sender + "   " + endtime + "   " + seqNum + "\n";
    myfile.close();
}
/*\Ion*/


/*Ion*/
void OBUapp::saveRXPacket(int node, WsmExt* ext){
    ofstream myfile;
    string path = this->path+"RXPacketsD/";
    ostringstream ss;
    ss << node;
    string filename = ss.str();
    myfile.open((path+filename).c_str(), ios::app);

    ostringstream ss1;
    ss1 << ext->getSenderAddress();
    string sender = ss1.str();

    ostringstream ss2;
    ss2 << ext->getSeqNum();
    string seqNum = ss2.str();

    ostringstream ss5;
    ss5 << simTime();
    string endtime = ss5.str();

    myfile << sender + "   " + endtime + "   " + seqNum + "\n";
    myfile.close();
}
/*\Ion*/


/*Ion*/
void OBUapp::mergeNeighbors(int node, WsmExt* ext){
    string rdLine;
    set<string> set1;
    set<string> set2;

    stringstream ss1;
    ss1 << node;
    string filename1 = ss1.str();

    stringstream ss2;
    ss2 << ext->getSenderAddress();
    string filename2 = ss2.str();

    string path1 = this->path+"RXPackets/";
    string path2 = this->path+"neighbors/";

    fstream selfFile;
    fstream myfile;
    myfile.open((path1+filename1).c_str());

    if (myfile.is_open()){ //file exist
    //    myfile.open((path1+filename1).c_str(), ios::in);
   //     ifstream selfFile((path1+filename1).c_str(), ios::in);
        ifstream srcFile((path2+filename2).c_str(), ios::in);
        debugEV<< "My file exists! Open the existing one!" <<endl;
        // fill set1 with the data from the existing file
        while ( getline (myfile,rdLine) ){
            set1.insert(rdLine);
        }
        //fill set2 with the data of the incoming node
        while ( getline (srcFile,rdLine) ){
            set2.insert(rdLine);
        }
    /*    debugEV<< "SET 1" <<endl;
        for (set<string>::iterator it=set1.begin(); it!=set1.end(); it++){
            debugEV<< *it <<endl;
        }
        debugEV<< "SET 2" <<endl;
        for (set<string>::iterator it=set2.begin(); it!=set2.end(); it++){
            debugEV<< *it <<endl;
        }*/
        set1.insert(set2.begin(), set2.end());
        PktSize = 14*set1.size();
  /*      debugEV<< "MERGED SET" <<endl;
        for (set<string>::iterator it=set1.begin(); it!=set1.end(); it++){
            debugEV<< *it <<endl;
        }
        debugEV<< "Set size = " << set1.size() <<endl;*/
        myfile.close();
        myfile.open((path1+filename1).c_str(), ios::out|ios::trunc);
        for (set<string>::iterator it=set1.begin(); it!=set1.end(); it++){
            myfile << *it + "\n";
        }
        srcFile.close();
    }
    else{ //file does not exist
        ifstream selfFile((path2+filename1).c_str(), ios::in);
        ifstream srcFile((path2+filename2).c_str(), ios::in);
        debugEV<< "My file does not exists! Create a new one!" <<endl;
        // fill set1 with own neighbors
        while ( getline (selfFile,rdLine) ){
            set1.insert(rdLine);
        }
        //fill set2 with the data of the incoming node
        while ( getline (srcFile,rdLine) ){
            set2.insert(rdLine);
        }
        set1.insert(set2.begin(), set2.end());
        PktSize = 14*set1.size();
        myfile.open((path1+filename1).c_str(), ios::out);
        for (set<string>::iterator it=set1.begin(); it!=set1.end(); it++){
            myfile << *it + "\n";
        }
        srcFile.close();
        selfFile.close();
    }
    myfile.close();
}
/*\Ion*/


/*Ion*/
int OBUapp::getDefaultDistance(){
    int result = 0;
    ifstream myfile;
    string line;
    string filename = "/home/pierpaolo/Scrivania/Ion_Results/Script/D.txt";
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


/*Ion*/
int OBUapp::getDefaultTTL(){
    int result = 0;
    ifstream myfile;
    string line;
    string filename = "/home/pierpaolo/Scrivania/Ion_Results/Script/TTL.txt";
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


/*Ion: a method to verify if the distance between 2 nodes is less than D+D/2*/
bool OBUapp::distanceOK(double x1, double y1, double x2, double y2){
    double dist = sqrt( pow((x1 - x2),2) + pow((y1 - y2),2));
    if (dist<(defaultDistance+defaultDistance/2)){
        return true;
    }
    else return false;
}
/*\Ion*/


/*Ion: a method to verify if 3 points are collinear using the triangle rule*/
bool OBUapp::collinear(double x1, double y1, double x2, double y2, double x3, double y3) {
    double a = sqrt( pow((x1 - x2),2) + pow((y1 - y2),2));  //the distance between the sender and my neighbor
    double b = sqrt( pow((x3 - x1),2) + pow((y3 - y1),2));  //the distance between myself and the sender
    double c = sqrt( pow((x3 - x2),2) + pow((y3 - y2),2));  //the distance between myself and my neighbor
    double cosalfa = (pow(b,2)+pow(c,2)-pow(a,2))/(2*b*c);
    double cosbeta = (pow(a,2)+pow(c,2)-pow(b,2))/(2*a*c);
    double cosgama = (pow(a,2)+pow(b,2)-pow(c,2))/(2*a*b);
    double alfa = acos(cosalfa) * 180.0 / PI;
    double beta = acos(cosbeta) * 180.0 / PI;
    double gama = acos(cosgama) * 180.0 / PI;
  /*  debugEV<< "Sender: x=" << x1 << " | y=" << y1 <<endl;
    debugEV<< "Neighbor: x=" << x2 << " | y=" << y2 <<endl;
    debugEV<< "Myself: x=" << x3 << " | y=" << y3    <<endl;
    debugEV<< "alfa = " << alfa <<endl;
    debugEV<< "beta = " << beta <<endl;
    debugEV<< "gama = " << gama <<endl;*/
    if (alfa<30 || (180-alfa)<30) return true;
    else{
        if (gama<30 || (180-gama)<30) return true;
        else return false;
    }
}
/*\Ion*/



OBUapp::~OBUapp() {

}


double OBUapp::computeDelay(WsmExt* ext){
    dist = ext->getSenderPos().distance(curPosition);
    //emit(txDist, dist);
    debugEV << "Current Tmax: " << Tmax <<endl;

    if (delayType == 0) {
        return 0;                          // FLOODING
    }

    else if (delayType == 1) {
        return dblrand()*delayTimerType1;             // RANDOM
    }

    else if (delayType == 2) {
        if (dist > R) { return 100000; }
        else { return Tmax*(1 - dist/R); }          // DISTANCE BASED FORWARDING
    }

    else if (delayType == 3) {
        double probability = dist/R;        // PROBABILISTIC BROADCAST
        double minProb = 0;                 // TODO: forse manca qualcosa! rivedere..
        double diff = 1 - minProb;
        double alea = ( ((double)rand() / RAND_MAX) * diff ) + minProb;
        if (alea < probability){
            return 0;
        }
    }
    else if (delayType == 4) {
        return 1/dist;          // TEST
    }

    else if (delayType == 5) {

        double time = 0.0;
        if (maxDist == 0.0 || dist >= maxDist){
            time = Tmax*(1 - dist/R);
        } else {
            time = Tmax*(1 - dist/maxDist);
        }

        return time;
    }

    else if (delayType == 6) {

        dist = abs(sqrt( pow((ext->getSenderPos().x - curPosition.x),2) + pow((ext->getSenderPos().y - curPosition.y),2) ) - 500);

        if (dist > R) { return 100000; }
        else { return Tmax*(1 - dist/500); }          // TBN
    }

    else {
        opp_error("########## ERROR COMPUTING DELAY !!! ##########");
    }
    return 100000;
}








