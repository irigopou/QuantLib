/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2004, 2005, 2006 Ferdinando Ametrano
 Copyright (C) 2006 Katiuscia Manzoni
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2003, 2004, 2005, 2006, 2008 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/time/frequency.hpp>
#include <ql/types.hpp>
#include <ql/errors.hpp>

namespace QuantLib {

	Real frequencyAsReal( Frequency f ) {
        switch (f) {
		  //*****  Special Cases Start  ******
          case Every26Weeks:
            return 2.0;
          case Every26Weeks_360:
            return 360.0/182.0;
          case Every26Weeks_365:
            return 365.0/182.0;
		  //*****  Special Cases End  ******

		  //*****  Invalid Cases Start  ******
          case OtherFrequency:
			  QL_FAIL("unknown frequency (" << Integer(f) << ")");
		  //*****  Invalid Cases End  ******
          default:
			return (Real) f;//Remaining cases. Here belongs the NoFrequency (-1) too, which should not cause error throwing here because it would normally not be used by the client!
        }
	}

    std::ostream& operator<<(std::ostream& out, Frequency f) {
        switch (f) {
          case NoFrequency:
            return out << "No-Frequency";
          case Once:
            return out << "Once";
          case Annual:
            return out << "Annual";
          case Semiannual:
            return out << "Semiannual";
          case EveryFourthMonth:
            return out << "Every-Fourth-Month";
          case Quarterly:
            return out << "Quarterly";
          case Bimonthly:
            return out << "Bimonthly";
          case Monthly:
            return out << "Monthly";
          case EveryFourthWeek:
            return out << "Every-fourth-week";
          case Biweekly:
            return out << "Biweekly";
          case Weekly:
            return out << "Weekly";
          case Daily:
            return out << "Daily";
		  //*****  DERISCOPE START  ******
          case Every26Weeks:
            return out << "Every-26-weeks";
          case Every26Weeks_360:
            return out << "Every-182-Days-360";
          case Every26Weeks_365:
            return out << "Every-182-Days-365";
		  //*****  DERISCOPE END  ******
          case OtherFrequency:
            return out << "Unknown frequency";
          default:
            QL_FAIL("unknown frequency (" << Integer(f) << ")");
        }
    }

}
