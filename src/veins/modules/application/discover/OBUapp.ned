//
// Copyright (C) 2006-2011 Christoph Sommer <christoph.sommer@uibk.ac.at>
//
// Documentation for these modules is at http://veins.car2x.org/
//
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

package org.car2x.veins.modules.application.discover;
import org.car2x.veins.modules.application.ieee80211p.BaseWaveApplLayer;
import org.car2x.veins.base.modules.IBaseApplLayer;

simple OBUapp extends BaseWaveApplLayer
{
    parameters:
        // Max number of entries in the list of already broadcasted messages
        double bcMaxEntries = default(100);
        // Time after which an entry for an already broadcasted msg can be deleted
        double bcDelTime @unit(s) = default(100 s);
        int delayType = default(2);
        // 0 = FLOODING
        // 1 = RANDOM
        // 2 = DBF
        // 3 = DBD
        double Tmax @unit(ms) = default(100 ms);
        bool connectivityMatrix = default(false);
        int TmaxType = default(0);
        // 0 = fixed Tmax
        // 1 = Tmax reduced for non-spurious forwarder
        // 3 = DBD
        
        //volatile double BcastInterval = default(1s) @unit(s); //the intervall between 2 RSU messages
    	//double BcastOn @unit(s);
        //double BcastOff @unit(s);
        @class(OBUapp);
        @display("i=block/fork");
}
