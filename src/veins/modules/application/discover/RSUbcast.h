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

#ifndef __MIXIM_RSUBCAST_H_
#define __MIXIM_RSUBCAST_H_

#include <omnetpp.h>
#include "veins/modules/application/ieee80211p/BaseWaveApplLayer.h"
#include <veins/modules/messages/WsmExt_m.h>

#include <string>
using namespace std;

/**
 * TODO - Generated class
 */
class RSUbcast : public BaseWaveApplLayer
{
    public:
        virtual void initialize(int stage);
        virtual void finish();

        enum RSUMessageKinds {
            DISSEMINATION_BCAST,
            STOP_VEHICLE
        };

    protected:
        virtual void onBeacon(WaveShortMessage* wsm);
        virtual void onData(WaveShortMessage* wsm);
        virtual void handleSelfMsg(cMessage* msg);
        virtual WsmExt* prepareWSMext(unsigned long RSUseqNum, std::string name, int dataLengthBits, t_channel channel, int priority, int rcvId, int serial=0);
        /*Methods added by Ion*/
        int getDefaultTTL();
        /*\Ion*/

    protected:
        unsigned long RSUseqNum;
        cMessage* sendDissemination;
        cMessage* sendStop;
        double BcastOn;
        double BcastOff;
        volatile double BcastInterval;
        int ttl;
        double StopVehicle;
        int payload;
        string sim_name;
        string path;
        double stabilizeTime;
      //  double startBeacon;
      //  cMessage* saveMsg;
      //  set<string> setOfBeacons;
};

#endif
