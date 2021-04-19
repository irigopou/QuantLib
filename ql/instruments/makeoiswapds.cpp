//Added by Deriscope

#include <ql/instruments/makeoiswapds.hpp>
#include <ql/pricingengines/swap/discountingswapengine.hpp>
#include <ql/indexes/iborindex.hpp>
#include <ql/time/schedule.hpp>

namespace QuantLib {
	
    MakeOiSwapDS::MakeOiSwapDS(const Period& swapTenor,
                     const ext::shared_ptr<OvernightIndex>& overnightIndex,
                     Rate fixedRate,
                     const Period& forwardStart)
    : swapTenor_(swapTenor), overnightIndex_(overnightIndex),
      fixedRate_(fixedRate), forwardStart_(forwardStart),
      settlementDays_(2),
      calendar_(overnightIndex->fixingCalendar()),
      paymentFrequency_(Annual),
      paymentCalendar_(Calendar()),
      paymentAdjustment_(Following),
      paymentLag_(0),
      rule_(DateGeneration::Backward),
      // any value here for endOfMonth_ would not be actually used
      isDefaultEOM_(true),
      type_(OiSwapDS::Payer), nominals_(1, 1.0),
      overnightSpread_(0.0),
      fixedDayCount_(overnightIndex->dayCounter()), 
	  rateCutoff_(0), typeOI_(OiSwapIndexDS::CompoundingOI), approxOI_(OiSwapIndexDS::NoneOI) {}

    MakeOiSwapDS::operator OiSwapDS() const {
        ext::shared_ptr<OiSwapDS> ois = *this;
        return *ois;
    }

	//similar to the code in MakeOiSwapDS
    MakeOiSwapDS::operator ext::shared_ptr<OiSwapDS>() const {
        Date startDate;
        if (effectiveDate_ != Date())
            startDate = effectiveDate_;
        else {
            Date refDate = Settings::instance().evaluationDate();
            // if the evaluation date is not a business day
            // then move to the next business day
            refDate = calendar_.adjust(refDate);
            Date spotDate = calendar_.advance(refDate,
                                              settlementDays_*Days);
            startDate = spotDate+forwardStart_;
            if (forwardStart_.length()<0)
                startDate = calendar_.adjust(startDate, Preceding);
            else
                startDate = calendar_.adjust(startDate, Following);
        }

        // OIS end of month default
        bool usedEndOfMonth =
            isDefaultEOM_ ? calendar_.isEndOfMonth(startDate) : endOfMonth_;

        Date endDate = terminationDate_;
        if (endDate == Date()) {
            if (usedEndOfMonth)
                endDate = calendar_.advance(startDate,
                                            swapTenor_,
                                            ModifiedFollowing,
                                            usedEndOfMonth);
            else
                endDate = startDate + swapTenor_;
        }

        Schedule schedule(startDate, endDate,
                          Period(paymentFrequency_),
                          calendar_,
                          ModifiedFollowing,
                          ModifiedFollowing,
                          rule_,
                          usedEndOfMonth);

        Rate usedFixedRate = fixedRate_;
        if (fixedRate_ == Null<Rate>()) {
            OiSwapDS temp(type_, nominals_[0], schedule,
            0.0, // fixed rate
            overnightIndex_, overnightSpread_, paymentLag_,  paymentAdjustment_, paymentCalendar_, fixedDayCount_, rateCutoff_, typeOI_, approxOI_);
			
            if (engine_ == 0) {
                Handle<YieldTermStructure> disc =
                                    overnightIndex_->forwardingTermStructure();
                QL_REQUIRE(!disc.empty(),
                           "null term structure set to this instance of " <<
                           overnightIndex_->name());
                bool includeSettlementDateFlows = false;
                ext::shared_ptr<PricingEngine> engine(new
                    DiscountingSwapEngine(disc, includeSettlementDateFlows));
                temp.setPricingEngine(engine);
            } else
                temp.setPricingEngine(engine_);

            usedFixedRate = temp.fairRate();
        }

        ext::shared_ptr<OiSwapDS> ois(new
            OiSwapDS(type_, nominals_[0], schedule,
            usedFixedRate, overnightIndex_, overnightSpread_, paymentLag_, paymentAdjustment_, paymentCalendar_, fixedDayCount_, rateCutoff_, typeOI_, approxOI_));

        if (engine_ == 0) {
            Handle<YieldTermStructure> disc =
                                overnightIndex_->forwardingTermStructure();
            bool includeSettlementDateFlows = false;
            ext::shared_ptr<PricingEngine> engine(new
                DiscountingSwapEngine(disc, includeSettlementDateFlows));
            ois->setPricingEngine(engine);
        } else
            ois->setPricingEngine(engine_);

        return ois;
    }
	
    MakeOiSwapDS& MakeOiSwapDS::receiveFixed(bool flag) {
        type_ = flag ? OiSwapDS::Receiver : OiSwapDS::Payer ;
        return *this;
    }

    MakeOiSwapDS& MakeOiSwapDS::withType(OiSwapDS::Type type) {
        type_ = type;
        return *this;
    }

    MakeOiSwapDS& MakeOiSwapDS::withNominals(std::vector<Real> n) {
        nominals_ = n;
        return *this;
    }

    MakeOiSwapDS& MakeOiSwapDS::withNominal(Real n) {
        nominals_ = std::vector<Real>(1, n);
        return *this;
    }

    MakeOiSwapDS& MakeOiSwapDS::withSettlementDays(Natural settlementDays) {
        settlementDays_ = settlementDays;
        effectiveDate_ = Date();
        return *this;
    }

    MakeOiSwapDS& MakeOiSwapDS::withEffectiveDate(const Date& effectiveDate) {
        effectiveDate_ = effectiveDate;
        return *this;
    }

    MakeOiSwapDS& MakeOiSwapDS::withTerminationDate(const Date& terminationDate) {
        terminationDate_ = terminationDate;
        swapTenor_ = Period();
        return *this;
    }

    MakeOiSwapDS& MakeOiSwapDS::withPaymentFrequency(Frequency f) {
        paymentFrequency_ = f;
        if (paymentFrequency_==Once)
            rule_ = DateGeneration::Zero;
        return *this;
    }

    MakeOiSwapDS& MakeOiSwapDS::withPaymentAdjustment(BusinessDayConvention convention) {
        paymentAdjustment_ = convention;
        return *this;
    }

    MakeOiSwapDS& MakeOiSwapDS::withPaymentLag(Natural lag) {
        paymentLag_ = lag;
        return *this;
    }

    MakeOiSwapDS& MakeOiSwapDS::withPaymentCalendar(const Calendar& cal) {
        paymentCalendar_ = cal;
        return *this;
    }

    MakeOiSwapDS& MakeOiSwapDS::withRule(DateGeneration::Rule r) {
        rule_ = r;
        if (r==DateGeneration::Zero)
            paymentFrequency_ = Once;
        return *this;
    }

    MakeOiSwapDS& MakeOiSwapDS::withDiscountingTermStructure(
                                        const Handle<YieldTermStructure>& d) {
        bool includeSettlementDateFlows = false;
        engine_ = ext::shared_ptr<PricingEngine>(new
            DiscountingSwapEngine(d, includeSettlementDateFlows));
        return *this;
    }

    MakeOiSwapDS& MakeOiSwapDS::withPricingEngine(
                             const ext::shared_ptr<PricingEngine>& engine) {
        engine_ = engine;
        return *this;
    }

    MakeOiSwapDS& MakeOiSwapDS::withFixedLegDayCount(const DayCounter& dc) {
        fixedDayCount_ = dc;
        return *this;
    }

    MakeOiSwapDS& MakeOiSwapDS::withEndOfMonth(bool flag) {
        endOfMonth_ = flag;
        isDefaultEOM_ = false;
        return *this;
    }

    MakeOiSwapDS& MakeOiSwapDS::withOvernightLegSpread(Spread sp) {
        overnightSpread_ = sp;
        return *this;
    }

    MakeOiSwapDS& MakeOiSwapDS::withRateCutoff(Natural rateCutoff) {
        rateCutoff_ = rateCutoff;
        return *this;
    }

    MakeOiSwapDS& MakeOiSwapDS::withTypeOI(OiSwapIndexDS::TypeOI typeOI) {
        typeOI_ = typeOI;
        return *this;
    }

    MakeOiSwapDS& MakeOiSwapDS::withApproxOI(OiSwapIndexDS::ApproxOI approxOI) {
        approxOI_ = approxOI;
        return *this;
    }

}
