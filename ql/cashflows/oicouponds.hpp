//Added by Deriscope

#ifndef quantlib_oicouponds_hpp
#define quantlib_oicouponds_hpp

#include <ql/cashflows/overnightindexedcoupon.hpp>
#include <ql/indexes/oiswapindexds.hpp>
#include <ql/time/schedule.hpp>

namespace QuantLib {
	
	//It broadens the original OvernightIndexedCoupon
	//It represents a generic oi coupon where the overnight index is either compounded or averaged over the referenced period
    class OiCouponDS : public OvernightIndexedCoupon {
      public:
        OiCouponDS(
            const Date& paymentDate,
            Real nominal,
            const Date& startDate,
            const Date& endDate,
            const ext::shared_ptr<OvernightIndex>& overnightIndex,
            Real gearing = 1.0,
            Spread spread = 0.0,
            const Date& refPeriodStart = Date(),
            const Date& refPeriodEnd = Date(),
            const DayCounter& dayCounter = DayCounter(),
			Natural rateCutoff = 0,
			OiSwapIndexDS::TypeOI typeOI = OiSwapIndexDS::CompoundingOI,
			OiSwapIndexDS::ApproxOI approxOI = OiSwapIndexDS::NoneOI);
        //name Inspectors
		const Natural rateCutoff() const { return rateCutoff_; }
		const OiSwapIndexDS::TypeOI typeOI() const { return typeOI_; }
		const OiSwapIndexDS::ApproxOI approxOI() const { return approxOI_; }
        void accept(AcyclicVisitor&);
        //@}
      private:
		Natural rateCutoff_;
        OiSwapIndexDS::TypeOI typeOI_;
		OiSwapIndexDS::ApproxOI approxOI_;
    };


    //helper class building a sequence of overnight coupons
    class OvernightLegDS : public OvernightLeg {
      public:
        OvernightLegDS( OvernightLeg const & oiLeg );
        OvernightLegDS( Schedule const & schedule, ext::shared_ptr<OiSwapIndexDS> const & oiSwapIndexDs);
		//The various withXYZ need to be declared here so that they return OvernightLegDS rather than OvernightLeg
        OvernightLegDS& withNotionals(Real notional);
        OvernightLegDS& withNotionals(const std::vector<Real>& notionals);
        OvernightLegDS& withPaymentDayCounter(const DayCounter&);
        OvernightLegDS& withPaymentAdjustment(BusinessDayConvention);
        OvernightLegDS& withPaymentCalendar(const Calendar&);
        OvernightLegDS& withPaymentLag(Natural lag);
        OvernightLegDS& withGearings(Real gearing);
        OvernightLegDS& withGearings(const std::vector<Real>& gearings);
        OvernightLegDS& withSpreads(Spread spread);
        OvernightLegDS& withSpreads(const std::vector<Spread>& spreads);
        OvernightLegDS& withRateCutoff(Natural rateCutoff);
        OvernightLegDS& withTypeOI(OiSwapIndexDS::TypeOI typeOI);
        OvernightLegDS& withApproxOI(OiSwapIndexDS::ApproxOI approxOI);
        operator Leg() const;
      private:
		Natural rateCutoff_;
        OiSwapIndexDS::TypeOI typeOI_;
		OiSwapIndexDS::ApproxOI approxOI_;
    };

}

#endif
