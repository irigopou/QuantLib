//Added by Deriscope

#include <ql/cashflows/oicouponds.hpp>
#include <ql/cashflows/couponpricer.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <ql/utilities/vectors.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>

using std::vector;

namespace QuantLib {

    namespace {

        class OiCouponPricerDS : public FloatingRateCouponPricer {
          public:
            void initialize(const FloatingRateCoupon& coupon) {
                coupon_ = dynamic_cast<const OiCouponDS*>(&coupon);
                QL_ENSURE(coupon_, "wrong coupon type");
            }
            Rate swapletRate() const {
				Natural rateCutoff = coupon_->rateCutoff();
				OiSwapIndexDS::TypeOI typeOI = coupon_->typeOI();
				OiSwapIndexDS::ApproxOI approxOI = coupon_->approxOI();
                ext::shared_ptr<OvernightIndex> index = ext::dynamic_pointer_cast<OvernightIndex>(coupon_->index());
                const vector<Date>& fixingDates = coupon_->fixingDates();
                const vector<Time>& dt = coupon_->dt();

                Size n = dt.size(), i = 0;
				QL_REQUIRE(rateCutoff < n, "rate cutoff (" << rateCutoff << ") must be less than number of fixings in period (" << n << ")");
				Size nMax = n - rateCutoff - 1;//index of last overnight period that carries its own rate

				bool isComp = typeOI == OiSwapIndexDS::CompoundingOI;
                Real compoundFactor = 1.0;//used only if isComp = true
                Real avg = 0.0;//used only if isComp = false

                // already fixed part
                Handle<YieldTermStructure> curve = index->forwardingTermStructure();
                QL_REQUIRE(!curve.empty(), "null term structure set to this instance of " << index->name());
				Date const & today = curve->getHistHorDate();
                while (i<n && fixingDates[std::min(i, nMax)]<today) {
                    // rate must have been fixed
                    Rate pastFixing = IndexManager::instance().getHistory(
                                                index->name())[fixingDates[std::min(i, nMax)]];
                    QL_REQUIRE(pastFixing != Null<Real>(),
                               "Missing " << index->name() <<
                               " fixing for " << fixingDates[std::min(i, nMax)]);
					if( isComp )
						compoundFactor *= (1.0 + pastFixing*dt[i]);
					else
						avg += pastFixing*dt[i];
                    ++i;
                }

                // today is a border case
                if (i<n && fixingDates[std::min(i, nMax)] == today) {
                    // might have been fixed
                    try {
                        Rate pastFixing = IndexManager::instance().getHistory(
                                                index->name())[fixingDates[std::min(i, nMax)]];
                        if (pastFixing != Null<Real>()) {
							if( isComp )
								compoundFactor *= (1.0 + pastFixing*dt[i]);
							else
								avg += pastFixing*dt[i];
                            ++i;
                        } else {
                            ;   // fall through and forecast
                        }
                    } catch (Error&) {
                        ;       // fall through and forecast
                    }
                }

                // forward part using telescopic property in order
                // to avoid the evaluation of multiple forward fixings
                if (i<n) {
					if( isComp || approxOI == OiSwapIndexDS::TakadaOI ) {
						const vector<Date>& dates = coupon_->valueDates();//number of value dates = n+1 because the last value date is needed for defining the last overnight accrual period
						//The dates reflect the approxOI. If the latter is telescopic => there will be only a few date elements as set in the OvernightIndexedCoupon constructor
						DiscountFactor startDiscount = curve->discount(dates[i]);
						Date lastFixDate = dates[nMax];
						DiscountFactor endDiscount = rateCutoff > 0 ? curve->discount(lastFixDate) : curve->discount(dates[n]);
						QL_REQUIRE(endDiscount != 0.0, "End discount factor cannot be 0");
						// handle the rate cutoff period (if there is any, i.e. if rateCutoff > 0)
						if (rateCutoff > 0) {
							// forward discount factor for one calendar day on the cutoff date
							DiscountFactor fwdDF = curve->discount(dates[nMax] + 1) / endDiscount;
							// keep the above forward discount factor constant during the cutoff period
							Natural numDays = dates[n] - dates[nMax];
							endDiscount *= std::pow(fwdDF, numDays);
						}
						if( isComp )
							compoundFactor *= startDiscount/endDiscount;
						else
							avg += log(startDiscount / endDiscount);
					}
					else {
						const vector<Rate>& fixings = coupon_->indexFixings();
						const std::vector<Time>& dts = coupon_->dt();
						while (i<n) {
							avg += fixings[std::min(i, nMax)] * dts[i];
							++i;
						}
					}
                }
				Time t = coupon_->accrualPeriod();
				QL_REQUIRE(t > 0.0, "Coupon accrual period should be positive");
				Real amt = isComp ? (compoundFactor - 1.0) : avg;
                Rate rate = amt / t;
                return coupon_->gearing() * rate + coupon_->spread();
            }

            Real swapletPrice() const { QL_FAIL("swapletPrice not available");  }
            Real capletPrice(Rate) const { QL_FAIL("capletPrice not available"); }
            Rate capletRate(Rate) const { QL_FAIL("capletRate not available"); }
            Real floorletPrice(Rate) const { QL_FAIL("floorletPrice not available"); }
            Rate floorletRate(Rate) const { QL_FAIL("floorletRate not available"); }
          protected:
            const OiCouponDS* coupon_;
        };
    }

    OiCouponDS::OiCouponDS(
		const Date& paymentDate,
		Real nominal,
		const Date& startDate,
		const Date& endDate,
		const ext::shared_ptr<OvernightIndex>& overnightIndex,
		Real gearing,
		Spread spread,
		const Date& refPeriodStart,
		const Date& refPeriodEnd,
		const DayCounter& dayCounter,
		Natural rateCutoff,
		OiSwapIndexDS::TypeOI typeOI,
		OiSwapIndexDS::ApproxOI approxOI)
    : OvernightIndexedCoupon(paymentDate, nominal, startDate, endDate,
        overnightIndex,
        gearing, spread,
        refPeriodStart, refPeriodEnd,
        dayCounter, approxOI != OiSwapIndexDS::NoneOI )
		, rateCutoff_(rateCutoff), typeOI_(typeOI), approxOI_(approxOI)
	{

        setPricer(ext::shared_ptr<FloatingRateCouponPricer>(new OiCouponPricerDS));
    }

    void OiCouponDS::accept(AcyclicVisitor& v) {
        Visitor<OiCouponDS>* v1 =
            dynamic_cast<Visitor<OiCouponDS>*>(&v);
        if (v1 != 0) {
            v1->visit(*this);
        } else {
            FloatingRateCoupon::accept(v);
        }
    }

	//OvernightLegDS

    OvernightLegDS::OvernightLegDS(OvernightLeg const & oiLeg)
    : OvernightLeg(oiLeg) {}
	
	OvernightLegDS::OvernightLegDS( Schedule const & schedule, ext::shared_ptr<OiSwapIndexDS> const & oiSwapIndexDs)
    : OvernightLeg(schedule, oiSwapIndexDs->overnightIndex()) {
		rateCutoff_ = oiSwapIndexDs->rateCutoff();
		typeOI_ = oiSwapIndexDs->typeOI();
		approxOI_ = oiSwapIndexDs->approxOI();
	}
	
    OvernightLegDS& OvernightLegDS::withNotionals(Real notional) {
        notionals_ = vector<Real>(1, notional);
        return *this;
    }

    OvernightLegDS& OvernightLegDS::withNotionals(const vector<Real>& notionals) {
        notionals_ = notionals;
        return *this;
    }

    OvernightLegDS& OvernightLegDS::withPaymentDayCounter(const DayCounter& dc) {
        paymentDayCounter_ = dc;
        return *this;
    }

    OvernightLegDS&
    OvernightLegDS::withPaymentAdjustment(BusinessDayConvention convention) {
        paymentAdjustment_ = convention;
        return *this;
    }

    OvernightLegDS& OvernightLegDS::withPaymentCalendar(const Calendar& cal) {
        paymentCalendar_ = cal;
        return *this;
    }

    OvernightLegDS& OvernightLegDS::withPaymentLag(Natural lag) {
        paymentLag_ = lag;
        return *this;
    }

    OvernightLegDS& OvernightLegDS::withGearings(Real gearing) {
        gearings_ = vector<Real>(1,gearing);
        return *this;
    }

    OvernightLegDS& OvernightLegDS::withGearings(const vector<Real>& gearings) {
        gearings_ = gearings;
        return *this;
    }

    OvernightLegDS& OvernightLegDS::withSpreads(Spread spread) {
        spreads_ = vector<Spread>(1,spread);
        return *this;
    }

    OvernightLegDS& OvernightLegDS::withSpreads(const vector<Spread>& spreads) {
        spreads_ = spreads;
        return *this;
    }

    OvernightLegDS& OvernightLegDS::withRateCutoff(Natural rateCutoff) {
        rateCutoff_ = rateCutoff;
        return *this;
    }

    OvernightLegDS& OvernightLegDS::withTypeOI(OiSwapIndexDS::TypeOI typeOI) {
        typeOI_ = typeOI;
        return *this;
    }

    OvernightLegDS& OvernightLegDS::withApproxOI(OiSwapIndexDS::ApproxOI approxOI) {
        approxOI_ = approxOI;
        return *this;
    }

    OvernightLegDS::operator Leg() const {

        QL_REQUIRE(!notionals_.empty(), "no notional given");

        Leg cashflows;

        // the following is not always correct
        Calendar calendar = schedule_.calendar();

        Date refStart, start, refEnd, end;
        Date paymentDate;

        Size n = schedule_.size()-1;
        for (Size i=0; i<n; ++i) {
            refStart = start = schedule_.date(i);
            refEnd   =   end = schedule_.date(i+1);
            paymentDate = paymentCalendar_.advance(end, paymentLag_, Days, paymentAdjustment_);

            if (i == 0 && schedule_.hasIsRegular() && !schedule_.isRegular(i+1))
                refStart = calendar.adjust(end - schedule_.tenor(),
                                           paymentAdjustment_);
            if (i == n-1 && schedule_.hasIsRegular() && !schedule_.isRegular(i+1))
                refEnd = calendar.adjust(start + schedule_.tenor(),
                                         paymentAdjustment_);

            cashflows.push_back( ext::shared_ptr<CashFlow>(new OiCouponDS(paymentDate,
                                    detail::get(notionals_, i, notionals_.back()),
                                    start, end,
                                    overnightIndex_,
                                    detail::get(gearings_, i, 1.0),
                                    detail::get(spreads_, i, 0.0),
                                    refStart, refEnd,
                                    paymentDayCounter_,
									rateCutoff_,
									typeOI_,
									approxOI_)) );
        }
        return cashflows;
    }

}
