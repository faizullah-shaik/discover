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

#ifndef OBUapp_H
#define OBUapp_H

#include <list>
#include <string>
using namespace std;

#include "veins/modules/application/ieee80211p/BaseWaveApplLayer.h"
#include "veins/modules/mobility/traci/TraCIMobility.h"
#include "veins/modules/mac/ieee80211p/Mac1609_4.h"
#include "veins/modules/application/discover/RSUbcast.h"
#include <veins/base/modules/BaseMobility.h>
#include <veins/modules/messages/WsmExt_m.h>
#include "veins/modules/mobility/traci/TraCICommandInterface.h"

using Veins::TraCIMobility;
using Veins::TraCICommandInterface;

/**
 * Flooding implementation for WAVE
 */
class OBUapp : public BaseWaveApplLayer {

    //friend class Mac1609_4;

	public:
		virtual void initialize(int stage);
	    virtual void finish();

	    enum OBUMessageKinds {
	                EXPIRED_TIMER,
	//                CON_MATRIX,
	                BEACON_MESSAGE,  // variable added by Ion
	                SAVE_MESSAGE,
	                REPLY_BEACON,       // added by Ion
	                PERMISSION_TO_SEND
	         //       DEFAULT_BEACON      // added by Ion
	            };

	protected:
        //TraCIMobility* traci;
	    TraCIMobility* mobility;
	    TraCICommandInterface* traci;
	    TraCICommandInterface::Vehicle* traciVehicle;
        //Mac1609_4* MAC;           // pointer to Mac1609_4 submodule
        simtime_t lastDroveAt;
        simsignal_t txDist;
        simsignal_t tmrDelay;
        simsignal_t tmrForwarder;   // saves the timer used when actually forwarding
        long PktReceived;           // all pkts received (with copies)
        long PktDist;               // distinct packets received
        long PktForwarded;          // sending to MAC layer
		long PktInibition;          // already sent by others, no need to forward
		long SpuriousForwarders;    // counts spurious forwarders
		double delay;               // time to wait before sending down pkts
		simtime_t timeIn;              // records when node exits border zone
		simtime_t timeOut;             // records when node enters border zone
        cMessage *expiredTimer;
 //       cMessage* conMatrixMess;
        short int delayType;
		int R;                      // max TX range
		int TmaxType;               // type of Tmax (fixed or variable)
        double TmaxOriginale;		// Tmax value from ini's parameter
		double Tmax;				// new Tmax, modified by spurious forwarding detection
        double maxDist;
        double dist;
        double distFromFwds;
        double width;
        double height;
        double borderX;
        double borderY;
        bool disabilitaNearBorder;
        cOutVector ReceptionsVec;   // vector with number of copies received for each seq num
        cOutVector ReceptionsVecDist;   // same as before but only distinct copies
        cOutVector Copies;          // number of copies for each seqNum
        cOutVector ForwardVec;      // stores sequence number of forwarded pkts
        bool conMatrix;
        double stopAt;
        double resumeAt;

        /*Ion: variables added by Ion*/
        cMessage* beaconMsg;        // variable added by Ion: a beacon message
        cMessage* saveMsg;
        cMessage* replyBeacon;      // added by Ion
        cMessage* sendBeacon;       // a self message that permits to forward the message
        //cMessage* defaultBeacon;    // added by Ion
        bool MULTIHOP_BEACON;
        bool RANDOM;
        bool GEONET;
        bool beacon;                // a variable to choose if we want to use beacon messages or not
        double defaultTimer;        // a parameter to create the timer
        double delayTimerType1;     // a delay timer for delayType = 1
        double delayTimerType2;
        double defaultDistance;     // the ideal distance from a sender node in order to elect a new leader
        bool leader;                // identify if a node is leader or not
        bool alreadyParticipated;   // identify if a node already participated in the election process
        bool readyToSave;           // says if a node can save the reply messages or not
        bool BIDIRECTION;           // if true, it is activated the bidirectional communication between RSU and nodes
        double startBeacon;         // a timer to initiate the neighborhood discovery
        double senderX;             // X position of the RSU
        double senderY;             // Y position of the RSU
        double tempMax;             // TMAX in order to calculate the timer for the reply beacon
        int TTL;                    // TTL in order to calculate the timer for the reply beacon
        int PktSize;
        int PktSizeLimit;
        set<string> setOfBeacons;
        set<string> setOfTXPackets;
        string sim_name;
        string path;
     //   simtime_t timeToReply;
     //   int currentHopCount;
        /*\Ion*/

		// defines a new type for linked list
	protected:
	    class Bcast {
	    public:
	        unsigned long       seqNum;
	        LAddress::L3Type    srcAddr;
	        simtime_t           delTime;
	        double              posx;
	        double              posy;
	        unsigned long       hopCount;
	        WsmExt*             packet;         //copy of the pkt
	        int                 copies;
	        unsigned long       idTimer;
	        double              delayTimer;
	        double              currentMaxDist;
	        short int           forwarded;

	    public:
	        // initialize Bcast list
	        Bcast(unsigned long n=-1, const LAddress::L3Type& s = LAddress::L3NULL,  simtime_t_cref d=SIMTIME_ZERO, double x=-1.0, double y=-1.0, unsigned long h=-1, WsmExt* pkt=NULL, int c=-1, unsigned long id=-1, double dly=1000, double cMaxD=-1, short int fwded=0) :
	            seqNum(n), srcAddr(s), delTime(d), posx(x), posy(y), hopCount(h), packet(pkt), copies(c), idTimer(id) , delayTimer(dly), currentMaxDist(cMaxD), forwarded(fwded) {
	        }
	    };

	    /*Ion: defines a new type of list for beacons*/
	    class Beacons {
	            public:
	                int         senderAddress;
	                double      senderPositionX;
	                double      senderPositionY;
	                simtime_t   simulTime;

	            public:
	                // initialize list
	                Beacons(int n=-1, double x=-1.0, double y=-1.0, simtime_t t=SIMTIME_ZERO) :
	                    senderAddress(n), senderPositionX(x), senderPositionY(y), simulTime(t) {
	                }
	            };
	    /*\Ion*/

	    /*Ion: a data structure to describe a road segment*/
	    class RoadSegment {
	           public:
	                int         roadID;
	                double      roadPosX;
	                double      roadPosY;
	                double      roadWeight;

	           public:
	                // initialize list
	                RoadSegment(int n=-1, double x=-1.0, double y=-1.0, double w=-1) :
	                     roadID(n), roadPosX(x), roadPosY(y), roadWeight(w) {
	                }
	           };
	    /*\Ion*/


	    typedef std::list<Bcast> cBroadcastList;

	    /*Ion: typedef and struct*/
	    typedef std::list<Beacons> cBeaconsList;
	    typedef std::pair<Beacons,double> myPair;
	    typedef std::list<RoadSegment> roadSegmentList;
	    typedef std::pair<Beacons,double> vehRoadPair;
	    typedef std::pair<Beacons,int> vehCounterPair;

	    struct sortVector
        {
        bool operator () (const myPair& left, const myPair& right)
            {
            return left.second < right.second;
            }
        };

	    cBeaconsList beaconMsgs;
	    roadSegmentList roadSegments;
	    //vehRoadPair rnPair;
	    /*\Ion*/

        cBroadcastList bcMsgs;
	    unsigned int bcMaxEntries;      // cBroadcastlist max size
	    simtime_t bcDelTime;            // time to keep entry in list


	    // Methods
	    virtual void onBeacon(WaveShortMessage* wsm);   // fake one!
        virtual void onData(WaveShortMessage* wsm);     // handles received pkts
        virtual void handlePositionUpdate(cObject* obj);// calls BaseWaveApplLayer function to refresh nodes position
        bool Received(WsmExt* ext);                     // checks whether pkt is received
        void setForwardedFlag(unsigned long seqNum);    // sets forwarded flag to 1
        bool didIForwardIt(unsigned long seqNum);       // checks whether pkt has been forwarded BY ME
        bool alreadyForwarded(WsmExt* ext);             // checks whether pkt has been already forwarded
        void handleSelfMsg(cMessage* msg);              // sends down pkt after delay
        double computeDelay(WsmExt* ext);               // computes delays for pkts

        /*Ion: methods*/
        void updateBeaconsList(cBeaconsList &myList, simtime_t time);                       //updates the beacons list
        bool electedLeader(Beacons sender, WsmExt* ext);                                                 // elects the leader node
        bool collinear(double x1, double y1, double x2, double y2, double x3, double y3);   // a method to verify if 3 points are collinear using the triangle rule
        bool distanceOK(double x1, double y1, double x2, double y2);                        // a method to verify if the distance between 2 nodes is less than D+D/2
        void saveReplyPacket(int parentNode, int replyNode);                                // a method to save into a file the reply packet
        void saveRequestPacket(int parentNode, int requestNode, simtime_t actualTime, double delayTime, int hopCount);
        void saveNeighborsList();                                                           // a method to save to file the neighbors of the given leader
        void saveNeighborToList(int node, WsmExt* ext);                                     // a method to save to file the neighbor of the given node
        void saveTXPacket(int node, WsmExt* ext);
        void saveRXPacket(int node, WsmExt* ext);
        void mergeNeighbors(int node, WsmExt* ext);
        int getDefaultDistance();
        void readListFromFile();
        double getRoadSegWeight(Beacons veh);
        void sortListByDistance(cBeaconsList& beaconsList, Beacons sender);
        int getDefaultTTL();
        /*\Ion*/

        WsmExt* loadPkt(unsigned long seqNum);
        double loadDelay(unsigned long seqNum);
        void setIdTimer(unsigned long timer, WsmExt* ext);
        unsigned long getIdTimer(unsigned long seqNum);
        unsigned long getSeqNumFromTimer(unsigned long idTmr);
        cMessage* getTimer(std::string name, unsigned long seqNum);
        double calcDistFromFwds(WsmExt* ext);           // calculate distance from last 2 forwarders
        void updateMaxDist(WsmExt* ext, short int f);
        ~OBUapp();

};

#endif
