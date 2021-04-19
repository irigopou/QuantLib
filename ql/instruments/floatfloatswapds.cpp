
#include <ql/instruments/floatfloatswapds.hpp>
#include <ql/cashflows/iborcoupon.hpp>
#include <ql/cashflows/cmscoupon.hpp>
#include <ql/cashflows/capflooredcoupon.hpp>
#include <ql/cashflows/cashflowvectors.hpp>
#include <ql/cashflows/cashflows.hpp>
#include <ql/cashflows/couponpricer.hpp>
#include <ql/cashflows/simplecashflow.hpp>
#include <ql/experimental/coupons/cmsspreadcoupon.hpp> // internal
#include <ql/cashflows/oicouponds.hpp>
#include <ql/indexes/iborindex.hpp>
#include <ql/indexes/swapindex.hpp>
#include <ql/indexes/oitermindexds.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>

namespace QuantLib {

    FloatFloatSwapDS::FloatFloatSwapDS(
		const VanillaSwap::Type type,
        const Real nominal1,
        const Real nominal2,
        const Schedule& schedule1,
        const ext::shared_ptr<InterestRateIndex>& index1,
        const DayCounter& pmtDayCount1,
        Natural pmtLag1,
		bool inArrears1,
        const Schedule& schedule2,
        const ext::shared_ptr<InterestRateIndex>& index2,
        const DayCounter& pmtDayCount2,
        Natural pmtLag2,
		bool inArrears2,
        const bool intermediateCapitalExchange,
        const bool finalCapitalExchange,
        const Real gearing1,
        const Real spread1,
        const Real cappedRate1,
        const Real flooredRate1,
        const Real gearing2,
        const Real spread2,
        const Real cappedRate2,
        const Real flooredRate2,
        const boost::optional<BusinessDayConvention>& pmtConv1,
        const boost::optional<BusinessDayConvention>& pmtConv2,
		Calendar const & pmtCal1,
		Calendar const & pmtCal2)
    : 
	Swap(2), type_(type), 
	nominal1_(std::vector<Real>(schedule1.size() - 1, nominal1)),
    nominal2_(std::vector<Real>(schedule2.size() - 1, nominal2)),
	nominalFinal1_(nominal1),
	nominalFinal2_(nominal2),
	schedule1_(schedule1),
    schedule2_(schedule2), index1_(index1), index2_(index2),
    gearing1_(std::vector<Real>(schedule1.size() - 1, gearing1)),
    gearing2_(std::vector<Real>(schedule2.size() - 1, gearing2)),
    spread1_(std::vector<Real>(schedule1.size() - 1, spread1)),
    spread2_(std::vector<Real>(schedule2.size() - 1, spread2)),
    cappedRate1_(std::vector<Real>(schedule1.size() - 1, cappedRate1)),
    flooredRate1_(std::vector<Real>(schedule1.size() - 1, flooredRate1)),
    cappedRate2_(std::vector<Real>(schedule2.size() - 1, cappedRate2)),
    flooredRate2_(std::vector<Real>(schedule2.size() - 1, flooredRate2)), 
	pmtDayCount1_(pmtDayCount1), pmtDayCount2_(pmtDayCount2), 
	pmtLag1_(pmtLag1), pmtLag2_(pmtLag2), 
	inArrears1_(inArrears1), inArrears2_(inArrears2), 
	pmtCal1_(pmtCal1), pmtCal2_(pmtCal2), 
	intermediateCapitalExchange_(intermediateCapitalExchange),
    finalCapitalExchange_(finalCapitalExchange) {

        init(pmtConv1, pmtConv2);
    }

    FloatFloatSwapDS::FloatFloatSwapDS(
		const VanillaSwap::Type type,
        const std::vector<Real>& nominal1,
        const std::vector<Real>& nominal2,
        const Real nominalFinal1,
        const Real nominalFinal2,
        const Schedule& schedule1,
        const ext::shared_ptr<InterestRateIndex>& index1,
        const DayCounter& pmtDayCount1,
        Natural pmtLag1,
		bool inArrears1,
        const Schedule& schedule2,
        const ext::shared_ptr<InterestRateIndex>& index2,
        const DayCounter& pmtDayCount2,
        Natural pmtLag2,
		bool inArrears2,
        const bool intermediateCapitalExchange,
        const bool finalCapitalExchange,
        const std::vector<Real>& gearing1,
        const std::vector<Real>& spread1,
        const std::vector<Real>& cappedRate1,
        const std::vector<Real>& flooredRate1,
        const std::vector<Real>& gearing2,
        const std::vector<Real>& spread2,
        const std::vector<Real>& cappedRate2,
        const std::vector<Real>& flooredRate2,
        const boost::optional<BusinessDayConvention>& pmtConv1,
        const boost::optional<BusinessDayConvention>& pmtConv2,
		Calendar const & pmtCal1,
		Calendar const & pmtCal2)
    : 
	Swap(2), type_(type), nominal1_(nominal1), nominal2_(nominal2), nominalFinal1_(nominalFinal1), nominalFinal2_(nominalFinal2),
	schedule1_(schedule1),
    schedule2_(schedule2), index1_(index1), index2_(index2), gearing1_(gearing1),
    gearing2_(gearing2), spread1_(spread1), spread2_(spread2), cappedRate1_(cappedRate1),
    flooredRate1_(flooredRate1), cappedRate2_(cappedRate2), flooredRate2_(flooredRate2),
    pmtDayCount1_(pmtDayCount1), pmtDayCount2_(pmtDayCount2),
	pmtLag1_(pmtLag1), pmtLag2_(pmtLag2), 
	inArrears1_(inArrears1), inArrears2_(inArrears2), 
	pmtCal1_(pmtCal1), pmtCal2_(pmtCal2), 
    intermediateCapitalExchange_(intermediateCapitalExchange),
    finalCapitalExchange_(finalCapitalExchange) {

        init(pmtConv1, pmtConv2);
    }

    void FloatFloatSwapDS::init(
        boost::optional<BusinessDayConvention> pmtConv1,
        boost::optional<BusinessDayConvention> pmtConv2) {

        QL_REQUIRE(nominal1_.size() == schedule1_.size() - 1,
                   "nominal1 size (" << nominal1_.size()
                                     << ") does not match schedule1 size ("
                                     << schedule1_.size() << ")");
        QL_REQUIRE(nominal2_.size() == schedule2_.size() - 1,
                   "nominal2 size (" << nominal2_.size()
                                     << ") does not match schedule2 size ("
                                     << nominal2_.size() << ")");
        QL_REQUIRE(gearing1_.empty() || gearing1_.size() == nominal1_.size(),
                   "nominal1 size (" << nominal1_.size() << ") does not match gearing1 size ("
                                     << gearing1_.size() << ")");
        QL_REQUIRE(gearing2_.empty() || gearing2_.size() == nominal2_.size(),
                   "nominal2 size (" << nominal2_.size() << ") does not match gearing2 size ("
                                     << gearing2_.size() << ")");
        QL_REQUIRE(cappedRate1_.empty() || cappedRate1_.size() == nominal1_.size(),
                   "nominal1 size (" << nominal1_.size() << ") does not match cappedRate1 size ("
                                     << cappedRate1_.size() << ")");
        QL_REQUIRE(cappedRate2_.empty() || cappedRate2_.size() == nominal2_.size(),
                   "nominal2 size (" << nominal2_.size() << ") does not match cappedRate2 size ("
                                     << cappedRate2_.size() << ")");
        QL_REQUIRE(flooredRate1_.empty() || flooredRate1_.size() == nominal1_.size(),
                   "nominal1 size (" << nominal1_.size() << ") does not match flooredRate1 size ("
                                     << flooredRate1_.size() << ")");
        QL_REQUIRE(flooredRate2_.empty() || flooredRate2_.size() == nominal2_.size(),
                   "nominal2 size (" << nominal2_.size() << ") does not match flooredRate2 size ("
                                     << flooredRate2_.size() << ")");

        if (pmtConv1) // NOLINT(readability-implicit-bool-conversion)
            pmtConv1_ = *pmtConv1;
        else
            pmtConv1_ = schedule1_.businessDayConvention();

        if (pmtConv2) // NOLINT(readability-implicit-bool-conversion)
            pmtConv2_ = *pmtConv2;
        else
            pmtConv2_ = schedule2_.businessDayConvention();

        if (gearing1_.empty())
            gearing1_ = std::vector<Real>(nominal1_.size(), 1.0);
        if (gearing2_.empty())
            gearing2_ = std::vector<Real>(nominal2_.size(), 1.0);
        if (spread1_.empty())
            spread1_ = std::vector<Real>(nominal1_.size(), 0.0);
        if (spread2_.empty())
            spread2_ = std::vector<Real>(nominal2_.size(), 0.0);
        if (cappedRate1_.empty())
            cappedRate1_ = std::vector<Real>(nominal1_.size(), Null<Real>());
        if (cappedRate2_.empty())
            cappedRate2_ = std::vector<Real>(nominal2_.size(), Null<Real>());
        if (flooredRate1_.empty())
            flooredRate1_ = std::vector<Real>(nominal1_.size(), Null<Real>());
        if (flooredRate2_.empty())
            flooredRate2_ = std::vector<Real>(nominal2_.size(), Null<Real>());

        bool cappedRate1_exist = false;
        for (Size i = 0; i < cappedRate1_.size(); i++) {
			if( cappedRate1_[i] != Null<Real>() ) {
				cappedRate1_exist = true;
				break;
			}
		}
        bool cappedRate2_exist = false;
        for (Size i = 0; i < cappedRate2_.size(); i++) {
			if( cappedRate2_[i] != Null<Real>() ) {
				cappedRate2_exist = true;
				break;
			}
		}
        bool flooredRate1_exist = false;
        for (Size i = 0; i < flooredRate1_.size(); i++) {
			if( flooredRate1_[i] != Null<Real>() ) {
				flooredRate1_exist = true;
				break;
			}
		}
        bool flooredRate2_exist = false;
        for (Size i = 0; i < flooredRate2_.size(); i++) {
			if( flooredRate2_[i] != Null<Real>() ) {
				flooredRate2_exist = true;
				break;
			}
		}

        // if the gearing is zero then the ibor / cms leg will be set up with
        // fixed coupons which makes trouble here in this context. We therefore
        // use a dirty trick and enforce the gearing to be non zero.
        for (Size i = 0; i < gearing1_.size(); i++)
            if (close(gearing1_[i], 0.0))
                gearing1_[i] = QL_EPSILON;
        for (Size i = 0; i < gearing2_.size(); i++)
            if (close(gearing2_[i], 0.0))
                gearing2_[i] = QL_EPSILON;
		
        QL_REQUIRE(index1_ != NULL || index2_ != NULL,
                   "at least one of the two indices in the swap must be non-null");

        ext::shared_ptr<IborIndex> ibor1 = ext::dynamic_pointer_cast<IborIndex>(index1_);
        ext::shared_ptr<IborIndex> ibor2 = ext::dynamic_pointer_cast<IborIndex>(index2_);
		//Below OiTermIndexDS must precede SwapIndex due to inheritance
        ext::shared_ptr<OiTermIndexDS> oiTermDs1 = ext::dynamic_pointer_cast<OiTermIndexDS>(index1_);
        ext::shared_ptr<OiTermIndexDS> oiTermDs2 = ext::dynamic_pointer_cast<OiTermIndexDS>(index2_);
		ext::shared_ptr<SwapIndex> cms1;
		if( oiTermDs1 == NULL )
			cms1 = ext::dynamic_pointer_cast<SwapIndex>(index1_);
		ext::shared_ptr<SwapIndex> cms2;
		if( oiTermDs2 == NULL )
			cms2 = ext::dynamic_pointer_cast<SwapIndex>(index2_);
        ext::shared_ptr<SwapSpreadIndex> cmsspread1 = ext::dynamic_pointer_cast<SwapSpreadIndex>(index1_);
        ext::shared_ptr<SwapSpreadIndex> cmsspread2 = ext::dynamic_pointer_cast<SwapSpreadIndex>(index2_);
		
		if( index1_ != NULL )
			QL_REQUIRE(ibor1 != NULL || cms1 != NULL || cmsspread1 != NULL || oiTermDs1 != NULL,
					   "since index1 is supplied, it must be ibor or cms or cms spread or overnight term");
		if( index2_ != NULL )
			QL_REQUIRE(ibor2 != NULL || cms2 != NULL || cmsspread2 != NULL || oiTermDs2 != NULL,
					   "since index2 is supplied, it must be ibor or cms or cms spread or overnight term");
		
		//Case of FixedRateLeg
        if (index1_ == 0) {
			FixedRateLeg leg(schedule1_);
            leg.withNotionals(nominal1_)
				.withCouponRates(spread1_, pmtDayCount1_)
				.withPaymentLag(pmtLag1_)
				.withPaymentAdjustment(pmtConv1_)
				.withPaymentCalendar(pmtCal1_);
            legs_[0] = leg;
		}
		
        if (index2_ == 0) {
			FixedRateLeg leg(schedule2_);
            leg.withNotionals(nominal2_)
				.withCouponRates(spread2_, pmtDayCount2_)
				.withPaymentLag(pmtLag2_)
				.withPaymentAdjustment(pmtConv2_)
				.withPaymentCalendar(pmtCal2_);
            legs_[1] = leg;
		}

		//Case of IborLeg
        if (ibor1 != 0) {
            IborLeg leg(schedule1_, ibor1);
			leg.withNotionals(nominal1_)
				.withPaymentDayCounter(pmtDayCount1_)
				.withPaymentLag(pmtLag1_)
				.withPaymentAdjustment(pmtConv1_)
				.withPaymentCalendar(pmtCal1_)
				.withSpreads(spread1_)
				.withGearings(gearing1_)
				.inArrears(inArrears1_);
            if (cappedRate1_exist)
                leg.withCaps(cappedRate1_);
            if (flooredRate1_exist)
                leg.withFloors(flooredRate1_);
            legs_[0] = leg;
        }

        if (ibor2 != 0) {
            IborLeg leg(schedule2_, ibor2);
            leg.withNotionals(nominal2_)
				.withPaymentDayCounter(pmtDayCount2_)
				.withPaymentLag(pmtLag2_)
				.withPaymentAdjustment(pmtConv2_)
				.withPaymentCalendar(pmtCal2_)
				.withSpreads(spread2_)
				.withGearings(gearing2_)
				.inArrears(inArrears2_);
            if (cappedRate2_exist)
                leg.withCaps(cappedRate2_);
            if (flooredRate2_exist)
                leg.withFloors(flooredRate2_);
            legs_[1] = leg;
        }
		
		//Case of CmsLeg
        if (cms1 != 0) {
			QL_REQUIRE(pmtLag1_ == 0, "non-zero payment lag is not supported in a cms leg");
            CmsLeg leg(schedule1_, cms1);
            leg.withNotionals(nominal1_)
				.withPaymentDayCounter(pmtDayCount1_)
				.withPaymentAdjustment(pmtConv1_)
				.withSpreads(spread1_)
				.withGearings(gearing1_);
            if (cappedRate1_exist)
                leg.withCaps(cappedRate1_);
            if (flooredRate1_exist)
                leg.withFloors(flooredRate1_);
            legs_[0] = leg;
        }

        if (cms2 != 0) {
			QL_REQUIRE(pmtLag2_ == 0, "non-zero payment lag is not supported in a cms leg");
            CmsLeg leg(schedule2_, cms2);
            leg.withNotionals(nominal2_)
				.withPaymentDayCounter(pmtDayCount2_)
				.withPaymentAdjustment(pmtConv2_)
				.withSpreads(spread2_)
				.withGearings(gearing2_);
            if (cappedRate2_exist)
                leg.withCaps(cappedRate2_);
            if (flooredRate2_exist)
                leg.withFloors(flooredRate2_);
            legs_[1] = leg;
        }
		
		//Case of CmsSpreadLeg
        if (cmsspread1 != 0) {
			QL_REQUIRE(pmtLag1_ == 0, "non-zero payment lag is not supported in a cms spread leg");
            CmsSpreadLeg leg(schedule1_, cmsspread1);
            leg.withNotionals(nominal1_)
				.withPaymentDayCounter(pmtDayCount1_)
				.withPaymentAdjustment(pmtConv1_)
				.withSpreads(spread1_)
				.withGearings(gearing1_);
            if (cappedRate1_exist)
                leg.withCaps(cappedRate1_);
            if (flooredRate1_exist)
                leg.withFloors(flooredRate1_);
            legs_[0] = leg;
        }

        if (cmsspread2 != 0) {
			QL_REQUIRE(pmtLag2_ == 0, "non-zero payment lag is not supported in a cms spread leg");
            CmsSpreadLeg leg(schedule2_, cmsspread2);
            leg.withNotionals(nominal2_)
				.withPaymentDayCounter(pmtDayCount2_)
				.withPaymentAdjustment(pmtConv2_)
				.withSpreads(spread2_)
				.withGearings(gearing2_);
            if (cappedRate2_exist)
                leg.withCaps(cappedRate2_);
            if (flooredRate2_exist)
                leg.withFloors(flooredRate2_);
            legs_[1] = leg;
        }
		
		//Case of OvernightLegDS
        if (oiTermDs1 != 0) {
            OvernightLegDS leg(schedule1_, oiTermDs1);
            leg.withNotionals(nominal1_)
				.withPaymentDayCounter(pmtDayCount1_)
				.withPaymentLag(pmtLag1_)
				.withPaymentAdjustment(pmtConv1_)
				.withPaymentCalendar(pmtCal1_)
				.withSpreads(spread1_)
				.withGearings(gearing1_);
            legs_[0] = leg;
        }

        if (oiTermDs2 != 0) {
            OvernightLegDS leg(schedule2_, oiTermDs2);
            leg.withNotionals(nominal2_)
				.withPaymentDayCounter(pmtDayCount2_)
				.withPaymentLag(pmtLag2_)
				.withPaymentAdjustment(pmtConv2_)
				.withPaymentCalendar(pmtCal2_)
				.withSpreads(spread2_)
				.withGearings(gearing2_);
            legs_[1] = leg;
        }

        if (intermediateCapitalExchange_) {
			//The legs_[0].size() should not be replaced by a fixed integer below because the size of legs_[0] may change in each iteration!
            for (Size i = 0; i < legs_[0].size(); i++) {//Deriscope: Changed the original terminal bound sz1-1 to sz1 so that the last period is also processed
				Real nextNominal = i == legs_[0].size() - 1 ? nominalFinal1_ : nominal1_[i + 1];
                Real cap = nominal1_[i] - nextNominal;
                if (!close(cap, 0.0)) {
                    std::vector<ext::shared_ptr<CashFlow> >::iterator it1 =
                        legs_[0].begin();
                    std::advance(it1, i + 1);
                    legs_[0].insert(
                        it1, ext::shared_ptr<CashFlow>(
                                 new AmortizingPayment(cap, legs_[0][i]->date())));//use AmortizingPayment as it better reflects this type of payment
                    std::vector<Real>::iterator it2 = nominal1_.begin();
                    std::advance(it2, i + 1);
                    nominal1_.insert(it2, nominal1_[i]);
                    i++;
                }
            }
			//The legs_[1].size() should not be replaced by a fixed integer below because the size of legs_[1] may change in each iteration!
            for (Size i = 0; i < legs_[1].size(); i++) {//Deriscope: Changed the original terminal bound sz2-1 to sz2 so that the last period is also processed
				Real nextNominal = i == legs_[1].size() - 1 ? nominalFinal2_ : nominal2_[i + 1];
                Real cap = nominal2_[i] - nextNominal;
                if (!close(cap, 0.0)) {
                    std::vector<ext::shared_ptr<CashFlow> >::iterator it1 =
                        legs_[1].begin();
                    std::advance(it1, i + 1);
                    legs_[1].insert(
                        it1, ext::shared_ptr<CashFlow>(
                                 new AmortizingPayment(cap, legs_[1][i]->date())));//use AmortizingPayment as it better reflects this type of payment
                    std::vector<Real>::iterator it2 = nominal2_.begin();
                    std::advance(it2, i + 1);
                    nominal2_.insert(it2, nominal2_[i]);
                    i++;
                }
            }
        }

        if (finalCapitalExchange_) {
			//Deriscope: Changed the original nominal1_.back() to nominalFinal1_
            legs_[0].push_back(ext::shared_ptr<CashFlow>(
                new Redemption(nominalFinal1_, legs_[0].back()->date())));
            nominal1_.push_back(nominalFinal1_);
			//Deriscope: Changed the original nominal2_.back() to nominalFinal2_
            legs_[1].push_back(ext::shared_ptr<CashFlow>(
                new Redemption(nominalFinal2_, legs_[1].back()->date())));
            nominal2_.push_back(nominalFinal2_);
        }

        for (Leg::const_iterator i = legs_[0].begin(); i < legs_[0].end(); ++i)
            registerWith(*i);

        for (Leg::const_iterator i = legs_[1].begin(); i < legs_[1].end(); ++i)
            registerWith(*i);

        switch (type_) {
        case VanillaSwap::Payer:
            payer_[0] = -1.0;
            payer_[1] = +1.0;
            break;
        case VanillaSwap::Receiver:
            payer_[0] = +1.0;
            payer_[1] = -1.0;
            break;
        default:
            QL_FAIL("Unknown float float - swap type");
        }
    }

    void FloatFloatSwapDS::setupArguments(PricingEngine::arguments *args) const {

        Swap::setupArguments(args);

        FloatFloatSwapDS::arguments *arguments =
            dynamic_cast<FloatFloatSwapDS::arguments *>(args);

        if (arguments == 0)
            return; // swap engine ... // QL_REQUIRE(arguments != 0, "argument type does not match");

        arguments->type = type_;
        arguments->nominal1 = nominal1_;
        arguments->nominal2 = nominal2_;
        arguments->nominalFinal1 = nominalFinal1_;
        arguments->nominalFinal2 = nominalFinal2_;
        arguments->index1 = index1_;
        arguments->index2 = index2_;

        const Leg &leg1Coupons = leg1();
        const Leg &leg2Coupons = leg2();

        arguments->leg1ResetDates = arguments->leg1PayDates =
            arguments->leg1FixingDates = std::vector<Date>(leg1Coupons.size());
        arguments->leg2ResetDates = arguments->leg2PayDates =
            arguments->leg2FixingDates = std::vector<Date>(leg2Coupons.size());

        arguments->leg1Spreads = arguments->leg1AccrualTimes =
            arguments->leg1Gearings = std::vector<Real>(leg1Coupons.size());
        arguments->leg2Spreads = arguments->leg2AccrualTimes =
            arguments->leg2Gearings = std::vector<Real>(leg2Coupons.size());

        arguments->leg1Coupons =
            std::vector<Real>(leg1Coupons.size(), Null<Real>());
        arguments->leg2Coupons =
            std::vector<Real>(leg2Coupons.size(), Null<Real>());

        arguments->leg1IsRedemptionFlow =
            std::vector<bool>(leg1Coupons.size(), false);
        arguments->leg2IsRedemptionFlow =
            std::vector<bool>(leg2Coupons.size(), false);

        arguments->leg1CappedRates = arguments->leg1FlooredRates =
            std::vector<Real>(leg1Coupons.size(), Null<Real>());
        arguments->leg2CappedRates = arguments->leg2FlooredRates =
            std::vector<Real>(leg2Coupons.size(), Null<Real>());

        for (Size i = 0; i < leg1Coupons.size(); ++i) {
            ext::shared_ptr<FloatingRateCoupon> coupon =
                ext::dynamic_pointer_cast<FloatingRateCoupon>(leg1Coupons[i]);
            if (coupon != 0) {
                arguments->leg1AccrualTimes[i] = coupon->accrualPeriod();
                arguments->leg1PayDates[i] = coupon->date();
                arguments->leg1ResetDates[i] = coupon->accrualStartDate();
                arguments->leg1FixingDates[i] = coupon->fixingDate();
                arguments->leg1Spreads[i] = coupon->spread();
                arguments->leg1Gearings[i] = coupon->gearing();
                try {
                    arguments->leg1Coupons[i] = coupon->amount();
                }
                catch (Error &) {
                    arguments->leg1Coupons[i] = Null<Real>();
                }
                ext::shared_ptr<CappedFlooredCoupon> cfcoupon =
                    ext::dynamic_pointer_cast<CappedFlooredCoupon>(
                        leg1Coupons[i]);
                if (cfcoupon != 0) {
                    arguments->leg1CappedRates[i] = cfcoupon->cap();
                    arguments->leg1FlooredRates[i] = cfcoupon->floor();
                }
            } else {
                ext::shared_ptr<CashFlow> cashflow =
                    ext::dynamic_pointer_cast<CashFlow>(leg1Coupons[i]);
                std::vector<Date>::const_iterator j =
                    std::find(arguments->leg1PayDates.begin(),
                              arguments->leg1PayDates.end(), cashflow->date());
                QL_REQUIRE(j != arguments->leg1PayDates.end(),
                           "nominal redemption on "
                               << cashflow->date()
                               << "has no corresponding coupon");
                Size jIdx = j - arguments->leg1PayDates.begin();
                arguments->leg1IsRedemptionFlow[i] = true;
                arguments->leg1Coupons[i] = cashflow->amount();
                arguments->leg1ResetDates[i] = arguments->leg1ResetDates[jIdx];
                arguments->leg1FixingDates[i] =
                    arguments->leg1FixingDates[jIdx];
                arguments->leg1AccrualTimes[i] = 0.0;
                arguments->leg1Spreads[i] = 0.0;
                arguments->leg1Gearings[i] = 1.0;
                arguments->leg1PayDates[i] = cashflow->date();
            }
        }

        for (Size i = 0; i < leg2Coupons.size(); ++i) {
            ext::shared_ptr<FloatingRateCoupon> coupon =
                ext::dynamic_pointer_cast<FloatingRateCoupon>(leg2Coupons[i]);
            if (coupon != 0) {
                arguments->leg2AccrualTimes[i] = coupon->accrualPeriod();
                arguments->leg2PayDates[i] = coupon->date();
                arguments->leg2ResetDates[i] = coupon->accrualStartDate();
                arguments->leg2FixingDates[i] = coupon->fixingDate();
                arguments->leg2Spreads[i] = coupon->spread();
                arguments->leg2Gearings[i] = coupon->gearing();
                try {
                    arguments->leg2Coupons[i] = coupon->amount();
                }
                catch (Error &) {
                    arguments->leg2Coupons[i] = Null<Real>();
                }
                ext::shared_ptr<CappedFlooredCoupon> cfcoupon =
                    ext::dynamic_pointer_cast<CappedFlooredCoupon>(
                        leg2Coupons[i]);
                if (cfcoupon != 0) {
                    arguments->leg2CappedRates[i] = cfcoupon->cap();
                    arguments->leg2FlooredRates[i] = cfcoupon->floor();
                }
            } else {
                ext::shared_ptr<CashFlow> cashflow =
                    ext::dynamic_pointer_cast<CashFlow>(leg2Coupons[i]);
                std::vector<Date>::const_iterator j =
                    std::find(arguments->leg2PayDates.begin(),
                              arguments->leg2PayDates.end(), cashflow->date());
                QL_REQUIRE(j != arguments->leg2PayDates.end(),
                           "nominal redemption on "
                               << cashflow->date()
                               << "has no corresponding coupon");
                Size jIdx = j - arguments->leg2PayDates.begin();
                arguments->leg2IsRedemptionFlow[i] = true;
                arguments->leg2Coupons[i] = cashflow->amount();
                arguments->leg2ResetDates[i] = arguments->leg2ResetDates[jIdx];
                arguments->leg2FixingDates[i] =
                    arguments->leg2FixingDates[jIdx];
                arguments->leg2AccrualTimes[i] = 0.0;
                arguments->leg2Spreads[i] = 0.0;
                arguments->leg2Gearings[i] = 1.0;
                arguments->leg2PayDates[i] = cashflow->date();
            }
        }
    }

    void FloatFloatSwapDS::setupExpired() const { Swap::setupExpired(); }

    void FloatFloatSwapDS::fetchResults(const PricingEngine::results *r) const {
        Swap::fetchResults(r);
    }

    void FloatFloatSwapDS::arguments::validate() const {

        Swap::arguments::validate();

        QL_REQUIRE(nominal1.size() == leg1ResetDates.size(),
                   "nominal1 size is different from resetDates1 size");
        QL_REQUIRE(nominal1.size() == leg1FixingDates.size(),
                   "nominal1 size is different from fixingDates1 size");
        QL_REQUIRE(nominal1.size() == leg1PayDates.size(),
                   "nominal1 size is different from payDates1 size");
        QL_REQUIRE(nominal1.size() == leg1Spreads.size(),
                   "nominal1 size is different from spreads1 size");
        QL_REQUIRE(nominal1.size() == leg1Gearings.size(),
                   "nominal1 size is different from gearings1 size");
        QL_REQUIRE(nominal1.size() == leg1CappedRates.size(),
                   "nominal1 size is different from cappedRates1 size");
        QL_REQUIRE(nominal1.size() == leg1FlooredRates.size(),
                   "nominal1 size is different from flooredRates1 size");
        QL_REQUIRE(nominal1.size() == leg1Coupons.size(),
                   "nominal1 size is different from coupons1 size");
        QL_REQUIRE(nominal1.size() == leg1AccrualTimes.size(),
                   "nominal1 size is different from accrualTimes1 size");
        QL_REQUIRE(nominal1.size() == leg1IsRedemptionFlow.size(),
                   "nominal1 size is different from redemption1 size");

        QL_REQUIRE(nominal2.size() == leg2ResetDates.size(),
                   "nominal2 size is different from resetDates2 size");
        QL_REQUIRE(nominal2.size() == leg2FixingDates.size(),
                   "nominal2 size is different from fixingDates2 size");
        QL_REQUIRE(nominal2.size() == leg2PayDates.size(),
                   "nominal2 size is different from payDates2 size");
        QL_REQUIRE(nominal2.size() == leg2Spreads.size(),
                   "nominal2 size is different from spreads2 size");
        QL_REQUIRE(nominal2.size() == leg2Gearings.size(),
                   "nominal2 size is different from gearings2 size");
        QL_REQUIRE(nominal2.size() == leg2CappedRates.size(),
                   "nominal2 size is different from cappedRates2 size");
        QL_REQUIRE(nominal2.size() == leg2FlooredRates.size(),
                   "nominal2 size is different from flooredRates2 size");
        QL_REQUIRE(nominal2.size() == leg2Coupons.size(),
                   "nominal2 size is different from coupons2 size");
        QL_REQUIRE(nominal2.size() == leg2AccrualTimes.size(),
                   "nominal2 size is different from accrualTimes2 size");
        QL_REQUIRE(nominal2.size() == leg2IsRedemptionFlow.size(),
                   "nominal2 size is different from redemption2 size");

        QL_REQUIRE(index1 != NULL, "index1 is null");
        QL_REQUIRE(index2 != NULL, "index2 is null");
    }

    void FloatFloatSwapDS::results::reset() { Swap::results::reset(); }
}
