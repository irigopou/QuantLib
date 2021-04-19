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

/*! \file impliedtermstructure.hpp
    \brief Implied term structure
*/

#ifndef quantlib_implied_term_structure_hpp
#define quantlib_implied_term_structure_hpp

#include <ql/termstructures/yieldtermstructure.hpp>
#include <utility>

namespace QuantLib {

    //! Implied term structure at a given date in the future
    /*! The given date will be the implied reference date.

        \note This term structure will remain linked to the original
              structure, i.e., any changes in the latter will be
              reflected in this structure as well.

        \ingroup yieldtermstructures

        \test
        - the correctness of the returned values is tested by
          checking them against numerical calculations.
        - observability against changes in the underlying term
          structure is checked.
    */
    class ImpliedTermStructure : public YieldTermStructure {
      public:
        ImpliedTermStructure(Handle<YieldTermStructure>, const Date& referenceDate);
        //! \name YieldTermStructure interface
        //@{
        DayCounter dayCounter() const override;
        Calendar calendar() const override;
        Natural settlementDays() const override;
        Date maxDate() const override;

      protected:
        DiscountFactor discountImpl(Time) const override;
        //@}
      private:
        Handle<YieldTermStructure> originalCurve_;
    };


    // inline definitions

    inline ImpliedTermStructure::ImpliedTermStructure(Handle<YieldTermStructure> h,
                                                      const Date& referenceDate)
    : YieldTermStructure(referenceDate), originalCurve_(std::move(h)) {
        registerWith(originalCurve_);
    }

    inline DayCounter ImpliedTermStructure::dayCounter() const {
        return originalCurve_->dayCounter();
    }

    inline Calendar ImpliedTermStructure::calendar() const {
        return originalCurve_->calendar();
    }

    inline Natural ImpliedTermStructure::settlementDays() const {
        return originalCurve_->settlementDays();
    }

    inline Date ImpliedTermStructure::maxDate() const {
        return originalCurve_->maxDate();
    }

    inline DiscountFactor ImpliedTermStructure::discountImpl(Time t) const {
        /* t is relative to the current reference date
           and needs to be converted to the time relative
           to the reference date of the original curve */
        Date const & ref = referenceDate();
		Date const & spotDate = originalCurve_->referenceDate();
        Time originalTime = t + dayCounter().yearFraction(spotDate, ref);
        /* discount at evaluation date cannot be cached
           since the original curve could change between
           invocations of this method */

		//*******************************************************************************************************************
		// DERISCOPE: Below commented out on 16.09.20 and replaced with the shown code because the discount() method may fail 
		// if the global eval date does not match the spot date, i.e. the originalCurve_->referenceDate() (see nest 2 lines)
		// Note the interpretation holds: originalCurve_ is the spot curve and *this is the forward curve
		// from which follows: spot date = originalCurve_->referenceDate() and forward date = this->referenceDate()
		//
		// The solution is to temporarily set the global eval date equal to the spot date and reset it back to its initial value at the end.
		// This solution has the drawback that it causes run time delays.
		// A useful trick is for the client to ensure that the global eval date is already set to the spot date before this function here is called.

//        return originalCurve_->discount(originalTime, true) /
//               originalCurve_->discount(ref, true);

		Date globalEvalDate = Settings::instance().evaluationDate();//read the current global eval date
		//For efficiency, handle the case refOrig == ref separately
		if( spotDate == globalEvalDate ) {
			//Here we can safely access the methods of originalCurve_ because the global eval date equals its own local reference date
			DiscountFactor denom = originalCurve_->discount(ref, true);
            QL_REQUIRE(denom != 0,
                       "Discount factor of spot curve for maturity " <<
                       ref << " must not be 0");
			DiscountFactor numer = originalCurve_->discount(originalTime, true);
			return numer / denom;
		}
		else {
			//Here we can safely access the methods of originalCurve_ only after the global eval date has been reset to equal own local reference date
			try {
				Settings::instance().evaluationDate() = spotDate;//set (temporarily) the global eval date to equal the spot date
				//Now we can safely access the methods of originalCurve_
				DiscountFactor denom = originalCurve_->discount(ref, true);
				QL_REQUIRE(denom != 0,
						   "Discount factor of spot curve for maturity " <<
						   ref << " must not be 0");
				DiscountFactor numer = originalCurve_->discount(originalTime, true);
				Settings::instance().evaluationDate() = globalEvalDate;//reset
				return numer / denom;
			}
			catch (...) {
				Settings::instance().evaluationDate() = globalEvalDate;//reset
				throw;
			}
		}
		//*******************************************************************************************************************

    }

}


#endif
