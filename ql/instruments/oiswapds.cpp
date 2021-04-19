//Added by Deriscope

#include <ql/instruments/oiswapds.hpp>
#include <ql/cashflows/oicouponds.hpp>
#include <ql/cashflows/fixedratecoupon.hpp>

namespace QuantLib {
	
	OiSwapDS::OiSwapDS(
		//both legs
		Type type,
        const std::vector<Real>& nominals,
		//fixed leg
        const Schedule& fxdSchedule,
        const std::vector<Real>& fxdRates,
        Natural fxdPmtLag,
		enum BusinessDayConvention fxdPmtAdj,
        const Calendar& fxdPmtCal,
        const DayCounter& fxdPmtDC, 
		//on leg
		const Schedule& onSchedule,
        const ext::shared_ptr<OvernightIndex>& index, 
		std::vector<Real> gearings,
        const std::vector<Spread>& spreads,
        Natural onPmtLag,
		enum BusinessDayConvention onPmtAdj,
        const Calendar& onPmtCal,
        const DayCounter& onPmtDC, 
		//special on flags
        Natural rateCutoff, 
		OiSwapIndexDS::TypeOI typeOI, 
		OiSwapIndexDS::ApproxOI approxOI)
		: Swap(2), type_(type), nominals_(nominals), fxdRates_(fxdRates), fxdSchedule_(fxdSchedule), fxdPmtLag_(fxdPmtLag), fxdPmtAdj_(fxdPmtAdj), fxdPmtCal_(fxdPmtCal), fxdPmtDC_(fxdPmtDC),
		index_(index), gearings_(gearings), spreads_(spreads), onSchedule_(onSchedule), onPmtLag_(onPmtLag), onPmtAdj_(onPmtAdj), onPmtCal_(onPmtCal), onPmtDC_(onPmtDC), 
		rateCutoff_(rateCutoff), typeOI_(typeOI), approxOI_(approxOI) {
        initialize();
    }

    OiSwapDS::OiSwapDS(
		//both legs
        Type type,
        Real nominal,
        const Schedule& schedule,
		//fixed leg
        Rate fxdRate,
		//on leg
        const ext::shared_ptr<OvernightIndex>& index,
        Spread spread,
		//both legs
        Natural pmtLag,
        enum BusinessDayConvention pmtAdj,
        const Calendar& pmtCal,
        const DayCounter& pmtDC,
		//special on flags
        Natural rateCutoff, 
		OiSwapIndexDS::TypeOI typeOI, 
		OiSwapIndexDS::ApproxOI approxOI)
		: Swap(2), type_(type), nominals_(1,nominal), fxdRates_(1,fxdRate), fxdSchedule_(schedule), fxdPmtLag_(pmtLag), fxdPmtAdj_(pmtAdj), fxdPmtCal_(pmtCal), fxdPmtDC_(pmtDC),
		index_(index), gearings_(1,1.0), spreads_(1,spread), onSchedule_(schedule), onPmtLag_(pmtLag), onPmtAdj_(pmtAdj), onPmtCal_(pmtCal), onPmtDC_(pmtDC), 
		rateCutoff_(rateCutoff), typeOI_(typeOI), approxOI_(approxOI) {
        initialize();
    }

    void OiSwapDS::initialize() {
		Size k;
		//set flat_nominals_
		for (k = 0; k < nominals_.size(); k++) {
			if( k > 0 && nominals_[k] != nominals_[k-1] )
				break;
		}
		flat_nominals_ = k > 0 && k == nominals_.size();
		//set flat_fxdRates_
		for (k = 0; k < fxdRates_.size(); k++) {
			if( k > 0 && fxdRates_[k] != fxdRates_[k-1] )
				break;
		}
		flat_fxdRates_ = k > 0 && k == fxdRates_.size();
		//set flat_gearings_
		for (k = 0; k < gearings_.size(); k++) {
			if( k > 0 && gearings_[k] != gearings_[k-1] )
				break;
		}
		flat_gearings_ = k > 0 && k == gearings_.size();
		//set flat_spreads_
		for (k = 0; k < spreads_.size(); k++) {
			if( k > 0 && spreads_[k] != spreads_[k-1] )
				break;
		}
		flat_spreads_ = k > 0 && k == spreads_.size();
		//set the undefined variables
		if( fxdPmtCal_.empty() )
			fxdPmtCal_ = fxdSchedule_.calendar();
		if( onPmtCal_.empty() )
			onPmtCal_ = onSchedule_.calendar();
        if (fxdPmtDC_==DayCounter())
            fxdPmtDC_ = index_->dayCounter();
        if (onPmtDC_==DayCounter())
            onPmtDC_ = index_->dayCounter();
		//set the legs
        legs_[0] = FixedRateLeg(fxdSchedule_)
            .withNotionals(nominals_)
            .withCouponRates(fxdRates_, fxdPmtDC_)
            .withPaymentLag(fxdPmtLag_)
            .withPaymentAdjustment(fxdPmtAdj_)
            .withPaymentCalendar(fxdPmtCal_);

		legs_[1] = OvernightLegDS(OvernightLeg(onSchedule_, index_))
			.withRateCutoff(rateCutoff_)
			.withTypeOI(typeOI_)
			.withApproxOI(approxOI_)
            .withNotionals(nominals_)
            .withSpreads(spreads_)
            .withPaymentLag(onPmtLag_)
            .withPaymentAdjustment(onPmtAdj_)
            .withPaymentCalendar(onPmtCal_);

        for (Size j=0; j<2; ++j) {
            for (Leg::iterator i = legs_[j].begin(); i!= legs_[j].end(); ++i)
                registerWith(*i);
        }

        switch (type_) {
          case Payer:
            payer_[0] = -1.0;
            payer_[1] = +1.0;
            break;
          case Receiver:
            payer_[0] = +1.0;
            payer_[1] = -1.0;
            break;
          default:
            QL_FAIL("Unknown overnight-swap type");
        }
    }

    Real OiSwapDS::fairRate() const {
        QL_REQUIRE(flat_fxdRates_, "result not available");
        static Spread basisPoint = 1.0e-4;
        calculate();
        return fxdRates_[0] - NPV_/(fixedLegBPS()/basisPoint);
    }

    Spread OiSwapDS::fairSpread() const {
        QL_REQUIRE(flat_spreads_, "result not available");
        static Spread basisPoint = 1.0e-4;
        calculate();
        return spreads_[0] - NPV_/(overnightLegBPS()/basisPoint);
    }

    Real OiSwapDS::fixedLegBPS() const {
        calculate();
        QL_REQUIRE(legBPS_[0] != Null<Real>(), "result not available");
        return legBPS_[0];
    }

    Real OiSwapDS::overnightLegBPS() const {
        calculate();
        QL_REQUIRE(legBPS_[1] != Null<Real>(), "result not available");
        return legBPS_[1];
    }

    Real OiSwapDS::fixedLegNPV() const {
        calculate();
        QL_REQUIRE(legNPV_[0] != Null<Real>(), "result not available");
        return legNPV_[0];
    }

    Real OiSwapDS::overnightLegNPV() const {
        calculate();
        QL_REQUIRE(legNPV_[1] != Null<Real>(), "result not available");
        return legNPV_[1];
    }

}
