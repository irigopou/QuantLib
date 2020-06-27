/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2008 StatPro Italia srl

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

/*! \file shiftedtermstructure.hpp
    \brief Implied term structure
*/

#ifndef quantlib_shifted_term_structure_hpp
#define quantlib_shifted_term_structure_hpp

#include <ql/termstructures/yieldtermstructure.hpp>

namespace QuantLib {

    //! Implied term structure arising by a parallel shift of forward discount factors
    /*! The reference date remains unchanged.

        \note This term structure will remain linked to the original
              structure, i.e., any changes in the latter will be
              reflected in this structure as well.

        \ingroup yieldtermstructures
    */
    class ShiftedTermStructure : public YieldTermStructure {
      public:
		//Deriscope: Added 3 optional arguments after shiftedReferenceDate
        ShiftedTermStructure(const Handle<YieldTermStructure>&,
                             const Date& shiftedReferenceDate,
                             Real dfMultiplier = 1.0,
                             const Date& lastZeroDfDt = Date(),
                             const Date& firstZeroDfDt = Date());
        //! \name YieldTermStructure interface
        //@{
        DayCounter dayCounter() const;
        Calendar calendar() const;
        Natural settlementDays() const;
        Date maxDate() const;
        Date shiftedReferenceDate() const;
		//Deriscope: Added 3 accessors
        Real dfMultiplier() const;
        const Date& lastZeroDfDt() const;
        const Date& firstZeroDfDt() const;
      protected:
        DiscountFactor discountImpl(Time) const;
        //@}
      private:
        Handle<YieldTermStructure> originalCurve_;
		Date shiftedReferenceDate_;
		//Deriscope: Added 3 private variables
		Real dfMultiplier_;
		Date lastZeroDfDt_;
		Date firstZeroDfDt_;
    };


    // inline definitions

    inline ShiftedTermStructure::ShiftedTermStructure(
                                          const Handle<YieldTermStructure>& h,
                                          const Date& shiftedReferenceDate,
										  Real dfMultiplier,
										  const Date& lastZeroDfDt,
										  const Date& firstZeroDfDt)
    : YieldTermStructure(h->referenceDate()), originalCurve_(h), 
		shiftedReferenceDate_(shiftedReferenceDate), dfMultiplier_(dfMultiplier),
		lastZeroDfDt_(lastZeroDfDt), firstZeroDfDt_(firstZeroDfDt) {
        registerWith(originalCurve_);
    }

    inline DayCounter ShiftedTermStructure::dayCounter() const {
        return originalCurve_->dayCounter();
    }

    inline Calendar ShiftedTermStructure::calendar() const {
        return originalCurve_->calendar();
    }

    inline Natural ShiftedTermStructure::settlementDays() const {
        return originalCurve_->settlementDays();
    }

    inline Date ShiftedTermStructure::maxDate() const {
        return originalCurve_->maxDate();
    }

    inline Date ShiftedTermStructure::shiftedReferenceDate() const {
        return shiftedReferenceDate_;
    }

    inline Real ShiftedTermStructure::dfMultiplier() const {
        return dfMultiplier_;
    }

    inline const Date& ShiftedTermStructure::lastZeroDfDt() const {
        return lastZeroDfDt_;
    }

    inline const Date& ShiftedTermStructure::firstZeroDfDt() const {
        return firstZeroDfDt_;
    }

    inline DiscountFactor ShiftedTermStructure::discountImpl(Time t) const {
		//*** Start of Deriscope addition  ***
		Date const & refDate = referenceDate();
		//return 1 when t is 0 in all cases because several pricing routines often divide the 
		//final result with the discount factor as of the reference date
		if(t==0)
			return 1.0;
		//return 0 if t indicates a date occurring on or before lastZeroDfDt_
		if( lastZeroDfDt_ != Date() ) {
			Time timeToLastZeroDfDt = dayCounter().yearFraction(refDate, lastZeroDfDt_);
			if(t<=timeToLastZeroDfDt)
				return 0.0;
		}
		//return 0 if t indicates a date occurring on or after firstZeroDfDt_
		if( firstZeroDfDt_ != Date() ) {
			Time timeToFirstZeroDfDt = dayCounter().yearFraction(refDate, firstZeroDfDt_);
			if(t>=timeToFirstZeroDfDt)
				return 0.0;
		}
		//calc the time until shiftedRefDate_
		//*** End of Deriscope addition  ***
		Time shift = dayCounter().yearFraction(
                                        referenceDate(), shiftedReferenceDate_);
		Real d1;
        /* discount at evaluation date cannot be cached
           since the original curve could change between
           invocations of this method */
		if(shift < 0) {
			d1 = originalCurve_->discount(-shift, true);
			QL_REQUIRE(d1 != 0, "discount factor cannot be zero");
			d1 = 1.0 / d1;
		}
		else if(t > shift)
			d1 = originalCurve_->discount(shift, true);
		else
			d1 = 1.0;
        /* When t is less than the shift interval,
			it is reasonable to assume that the implied discount factor equals the original.
			This is achieved by setting shiftedTime equal to t*/
		Time shiftedTime = t > shift ? t-shift : t;
		Real d2 = originalCurve_->discount(shiftedTime, true);
		//*** Start of Deriscope addition  ***
        //return d1*d2;
        return dfMultiplier_*d1*d2;
		//*** End of Deriscope addition  ***
    }

}


#endif
