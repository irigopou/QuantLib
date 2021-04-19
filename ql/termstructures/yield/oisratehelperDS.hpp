/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009, 2012 Roland Lichters
 Copyright (C) 2009, 2012 Ferdinando Ametrano

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

/*! \file oisratehelper.hpp
    \brief Overnight Indexed Swap (aka OIS) rate helpers
*/

#ifndef quantlib_oisratehelperds_hpp
#define quantlib_oisratehelperds_hpp

#include <ql/termstructures/yield/ratehelpers.hpp>
#include <ql/instruments/oiswapds.hpp>

namespace QuantLib {

    //! Rate helper for bootstrapping over OiSwapDS rates
	//It broadens the original OISRateHelper. Not possible to inherit from OISRateHelper because the latter's contructor involves initializeDates() which sets the types of swap
    class OISRateHelperDS : public RelativeDateRateHelper {
      public:
        OISRateHelperDS(
            Natural settlementDays,
            const Period& tenor, // swap maturity
            const Handle<Quote>& fixedRate,
            const ext::shared_ptr<OvernightIndex>& overnightIndex,
            // exogenous discounting curve
            const Handle<YieldTermStructure>& discountingCurve,
            Natural paymentLag,
            BusinessDayConvention paymentConvention,
            Frequency paymentFrequency,
            const Calendar& paymentCalendar,
            const Period& forwardStart,
            Spread overnightSpread,
			//special on flags
            Natural rateCutoff, 
			OiSwapIndexDS::TypeOI typeOI, 
			OiSwapIndexDS::ApproxOI approxOI,
            Pillar::Choice pillar = Pillar::LastRelevantDate,
            Date customPillarDate = Date());
        //! \name RateHelper interface
        //@{
        Real impliedQuote() const;
        void setTermStructure(YieldTermStructure*);
        //@}
        //! \name inspectors
        //@{
        ext::shared_ptr<OiSwapDS> swap() const { return swap_; }
        //@}
        //! \name Visitability
        //@{
        void accept(AcyclicVisitor&);
        //@}
    protected:
        void initializeDates();
        Pillar::Choice pillarChoice_;

        Natural settlementDays_;
        Period tenor_;
        ext::shared_ptr<OvernightIndex> overnightIndex_;

        ext::shared_ptr<OiSwapDS> swap_;
        RelinkableHandle<YieldTermStructure> termStructureHandle_;

        Handle<YieldTermStructure> discountHandle_;
        bool telescopicValueDates_;
        RelinkableHandle<YieldTermStructure> discountRelinkableHandle_;

        Natural paymentLag_;
        BusinessDayConvention paymentConvention_;
        Frequency paymentFrequency_;
        Calendar paymentCalendar_;
        Period forwardStart_;
        Spread overnightSpread_;
		//special on flags
		Natural rateCutoff_;
        OiSwapIndexDS::TypeOI typeOI_;
		OiSwapIndexDS::ApproxOI approxOI_;
    };

}

#endif
