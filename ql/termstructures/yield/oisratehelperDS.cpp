
#include <ql/termstructures/yield/oisratehelperds.hpp>
#include <ql/instruments/makeoiswapds.hpp>
#include <ql/pricingengines/swap/discountingswapengine.hpp>
#include <ql/utilities/null_deleter.hpp>

namespace QuantLib {

    OISRateHelperDS::OISRateHelperDS(
		Natural settlementDays,
		const Period& tenor, // swap maturity
		const Handle<Quote>& fixedRate,
		const ext::shared_ptr<OvernightIndex>& overnightIndex,
		const Handle<YieldTermStructure>& discount,
		Natural paymentLag,
		BusinessDayConvention paymentConvention,
		Frequency paymentFrequency,
		const Calendar& paymentCalendar,
		const Period& forwardStart, 
		const Spread overnightSpread,
		//special on flags
		Natural rateCutoff, 
		OiSwapIndexDS::TypeOI typeOI, 
		OiSwapIndexDS::ApproxOI approxOI,
        Pillar::Choice pillar,
        Date customPillarDate)
    : 
		RelativeDateRateHelper(fixedRate),
		pillarChoice_(pillar),
		settlementDays_(settlementDays), tenor_(tenor),
		overnightIndex_(overnightIndex), discountHandle_(discount),
		paymentLag_(paymentLag), paymentConvention_(paymentConvention),
		paymentFrequency_(paymentFrequency),
		paymentCalendar_(paymentCalendar),
		forwardStart_(forwardStart), overnightSpread_(overnightSpread),
		rateCutoff_(rateCutoff), typeOI_(typeOI), approxOI_(approxOI) {
        registerWith(overnightIndex_);
        registerWith(discountHandle_);

        pillarDate_ = customPillarDate;
        initializeDates();
    }

    void OISRateHelperDS::initializeDates() {

        // dummy OvernightIndex with curve/swap arguments
        // review here
        ext::shared_ptr<IborIndex> clonedIborIndex =
            overnightIndex_->clone(termStructureHandle_);
        ext::shared_ptr<OvernightIndex> clonedOvernightIndex =
            ext::dynamic_pointer_cast<OvernightIndex>(clonedIborIndex);

        // input discount curve Handle might be empty now but it could
        //    be assigned a curve later; use a RelinkableHandle here
        swap_ = MakeOiSwapDS(tenor_, clonedOvernightIndex, 0.0, forwardStart_)
            .withDiscountingTermStructure(discountRelinkableHandle_)
            .withSettlementDays(settlementDays_)
            .withPaymentLag(paymentLag_)
            .withPaymentAdjustment(paymentConvention_)
            .withPaymentFrequency(paymentFrequency_)
            .withPaymentCalendar(paymentCalendar_)
            .withOvernightLegSpread(overnightSpread_)
			.withRateCutoff(rateCutoff_)
			.withTypeOI(typeOI_)
			.withApproxOI(approxOI_);

        earliestDate_ = swap_->startDate();
        maturityDate_ = swap_->maturityDate();

        Date lastPaymentDate = std::max(swap_->overnightLeg().back()->date(),
                                        swap_->fixedLeg().back()->date());
        latestRelevantDate_ = std::max(maturityDate_, lastPaymentDate);

        switch (pillarChoice_) {
          case Pillar::MaturityDate:
            pillarDate_ = maturityDate_;
            break;
          case Pillar::LastRelevantDate:
            pillarDate_ = latestRelevantDate_;
            break;
          case Pillar::CustomDate:
            // pillarDate_ already assigned at construction time
            QL_REQUIRE(pillarDate_ >= earliestDate_,
                       "pillar date (" << pillarDate_ << ") must be later "
                       "than or equal to the instrument's earliest date (" <<
                       earliestDate_ << ")");
            QL_REQUIRE(pillarDate_ <= latestRelevantDate_,
                       "pillar date (" << pillarDate_ << ") must be before "
                       "or equal to the instrument's latest relevant date (" <<
                       latestRelevantDate_ << ")");
            break;
          default:
            QL_FAIL("unknown Pillar::Choice(" << Integer(pillarChoice_) << ")");
        }

        latestDate_ = std::max(swap_->maturityDate(), lastPaymentDate);
    }

    void OISRateHelperDS::setTermStructure(YieldTermStructure* t) {
        // do not set the relinkable handle as an observer -
        // force recalculation when needed
        bool observer = false;

        ext::shared_ptr<YieldTermStructure> temp(t, null_deleter());
        termStructureHandle_.linkTo(temp, observer);

        if (discountHandle_.empty())
            discountRelinkableHandle_.linkTo(temp, observer);
        else
            discountRelinkableHandle_.linkTo(*discountHandle_, observer);

        RelativeDateRateHelper::setTermStructure(t);
    }

    Real OISRateHelperDS::impliedQuote() const {
        QL_REQUIRE(termStructure_ != 0, "term structure not set");
        // we didn't register as observers - force calculation
        swap_->recalculate();
        return swap_->fairRate();
    }

    void OISRateHelperDS::accept(AcyclicVisitor& v) {
        Visitor<OISRateHelperDS>* v1 =
            dynamic_cast<Visitor<OISRateHelperDS>*>(&v);
        if (v1 != 0)
            v1->visit(*this);
        else
            RateHelper::accept(v);
    }

}
