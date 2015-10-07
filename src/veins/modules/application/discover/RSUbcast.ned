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

package org.car2x.veins.modules.application.discover;
import org.car2x.veins.base.modules.IBaseApplLayer;
import org.car2x.veins.modules.application.ieee80211p.BaseWaveApplLayer;

//
// TODO auto-generated type
//
simple RSUbcast extends BaseWaveApplLayer
{
        parameters:
        volatile double BcastInterval = default(1s) @unit(s); //the intervall between 2 RSU messages
		double BcastOn @unit(s);
        double BcastOff @unit(s);
        int ttl = default(10);
        int payload = default(954bit) @unit(bit); //the length of a data packet

        @class(RSUbcast);
}
