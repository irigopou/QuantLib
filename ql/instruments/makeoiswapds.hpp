//Added by Deriscope

#ifndef quantlib_makeoiswapds_hpp
#define quantlib_makeoiswapds_hpp

#include <ql/instruments/oiswapds.hpp>
#include <ql/time/dategenerationrule.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>

namespace QuantLib {
	
	//It broadens the original MakeOIS
	//Used to create objects of type OiSwapDS
    class MakeOiSwapDS {
      public:
        MakeOiSwapDS(const Period& swapTenor,
                const ext::shared_ptr<OvernightIndex>& overnightIndex,
                Rate fixedRate = Null<Rate>(),
                const Period& fwdStart = 0*Days);

        operator OiSwapDS() const;
        operator ext::shared_ptr<OiSwapDS>() const;

        MakeOiSwapDS& receiveFixed(bool flag = true);
        MakeOiSwapDS& withType(OiSwapDS::Type type);
        MakeOiSwapDS& withNominals(std::vector<Real> n);
        MakeOiSwapDS& withNominal(Real n);

        MakeOiSwapDS& withSettlementDays(Natural settlementDays);
        MakeOiSwapDS& withEffectiveDate(const Date&);
        MakeOiSwapDS& withTerminationDate(const Date&);
        MakeOiSwapDS& withRule(DateGeneration::Rule r);

        MakeOiSwapDS& withPaymentFrequency(Frequency f);
        MakeOiSwapDS& withPaymentAdjustment(BusinessDayConvention convention);
        MakeOiSwapDS& withPaymentLag(Natural lag);
        MakeOiSwapDS& withPaymentCalendar(const Calendar& cal);

        MakeOiSwapDS& withEndOfMonth(bool flag = true);

        MakeOiSwapDS& withFixedLegDayCount(const DayCounter& dc);

        MakeOiSwapDS& withOvernightLegSpread(Spread sp);

        MakeOiSwapDS& withDiscountingTermStructure(
                  const Handle<YieldTermStructure>& discountingTermStructure);

        MakeOiSwapDS& withPricingEngine(
                              const ext::shared_ptr<PricingEngine>& engine);

        MakeOiSwapDS &withRateCutoff(Natural rateCutoff);
        MakeOiSwapDS &withTypeOI(OiSwapIndexDS::TypeOI typeOI);
        MakeOiSwapDS &withApproxOI(OiSwapIndexDS::ApproxOI approxOI);
      private:
        Period swapTenor_;
        ext::shared_ptr<OvernightIndex> overnightIndex_;
        Rate fixedRate_;
        Period forwardStart_;

        Natural settlementDays_;
        Date effectiveDate_, terminationDate_;
        Calendar calendar_;

        Frequency paymentFrequency_;
        Calendar paymentCalendar_;
        BusinessDayConvention paymentAdjustment_;
        Natural paymentLag_;

        DateGeneration::Rule rule_;
        bool endOfMonth_, isDefaultEOM_;

        OiSwapDS::Type type_;
        std::vector<Real> nominals_;

        Spread overnightSpread_;
        DayCounter fixedDayCount_;

        ext::shared_ptr<PricingEngine> engine_;
		Natural rateCutoff_;
        OiSwapIndexDS::TypeOI typeOI_;
		OiSwapIndexDS::ApproxOI approxOI_;
    };

}

#endif
