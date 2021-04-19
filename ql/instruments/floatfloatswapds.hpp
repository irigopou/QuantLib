
#ifndef quantlib_floatfloatds_swap_hpp
#define quantlib_floatfloatds_swap_hpp

#include <ql/instruments/swap.hpp>
#include <ql/instruments/vanillaswap.hpp>
#include <ql/time/daycounter.hpp>
#include <ql/time/schedule.hpp>
#include <boost/optional.hpp>

namespace QuantLib {

    class InterestRateIndex;
	
	//It broadens the original FloatFloatSwap. Not possible to inherit from FloatFloatSwap because the latter's contructor involves init() which sets the types of coupons
    class FloatFloatSwapDS : public Swap {
      public:
        class arguments;
        class results;
        class engine;
        FloatFloatSwapDS(
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
            bool intermediateCapitalExchange = false,
            bool finalCapitalExchange = false,
            Real gearing1 = 1.0,
            Real spread1 = 0.0,
            Real cappedRate1 = Null<Real>(),
            Real flooredRate1 = Null<Real>(),
            Real gearing2 = 1.0,
            Real spread2 = 0.0,
            Real cappedRate2 = Null<Real>(),
            Real flooredRate2 = Null<Real>(),
            const boost::optional<BusinessDayConvention>& pmtConv1 = boost::none,
            const boost::optional<BusinessDayConvention>& pmtConv2 = boost::none,
			Calendar const & pmtCal1 = NullCalendar(),
			Calendar const & pmtCal2 = NullCalendar());

        FloatFloatSwapDS(
            VanillaSwap::Type type,
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
            bool intermediateCapitalExchange = false,
            bool finalCapitalExchange = false,
            const std::vector<Real>& gearing1 = std::vector<Real>(),
            const std::vector<Real>& spread1 = std::vector<Real>(),
            const std::vector<Real>& cappedRate1 = std::vector<Real>(),
            const std::vector<Real>& flooredRate1 = std::vector<Real>(),
            const std::vector<Real>& gearing2 = std::vector<Real>(),
            const std::vector<Real>& spread2 = std::vector<Real>(),
            const std::vector<Real>& cappedRate2 = std::vector<Real>(),
            const std::vector<Real>& flooredRate2 = std::vector<Real>(),
            const boost::optional<BusinessDayConvention>& pmtConv1 = boost::none,
            const boost::optional<BusinessDayConvention>& pmtConv2 = boost::none,
			Calendar const & pmtCal1 = NullCalendar(),
			Calendar const & pmtCal2 = NullCalendar());

        //! \name Inspectors
        //@{
        VanillaSwap::Type type() const;
        const std::vector<Real> &nominal1() const;
        const std::vector<Real> &nominal2() const;
		
		Real const nominalFinal1() const;
		Real const nominalFinal2() const;

        const Schedule &schedule1() const;
        const Schedule &schedule2() const;

        const ext::shared_ptr<InterestRateIndex> &index1() const;
        const ext::shared_ptr<InterestRateIndex> &index2() const;

        std::vector<Real> spread1() const;
        std::vector<Real> spread2() const;

        std::vector<Real> gearing1() const;
        std::vector<Real> gearing2() const;

        std::vector<Rate> cappedRate1() const;
        std::vector<Rate> flooredRate1() const;
        std::vector<Rate> cappedRate2() const;
        std::vector<Rate> flooredRate2() const;

        const DayCounter &pmtDayCount1() const;
        const DayCounter &pmtDayCount2() const;

		Natural const pmtLag1() const;
		Natural const pmtLag2() const;
		
		bool const inArrears1() const;
		bool const inArrears2() const;

        BusinessDayConvention pmtConv1() const;
        BusinessDayConvention pmtConv2() const;
		
        Calendar const & pmtCal1() const;
        Calendar const & pmtCal2() const;

        const Leg &leg1() const;
        const Leg &leg2() const;
        //@}

        //! \name Results
        //@{
        //@}
        // other
        void setupArguments(PricingEngine::arguments *args) const;
        void fetchResults(const PricingEngine::results *) const;

      private:
        void init(boost::optional<BusinessDayConvention> pmtConv1,
                  boost::optional<BusinessDayConvention> pmtConv2);
        void setupExpired() const;
        VanillaSwap::Type type_;
        std::vector<Real> nominal1_, nominal2_;
		Real nominalFinal1_, nominalFinal2_;//Deriscope: Added so that the amort cash flows associated with the last period are also processed
        Schedule schedule1_, schedule2_;
        ext::shared_ptr<InterestRateIndex> index1_, index2_;
        std::vector<Real> gearing1_, gearing2_, spread1_, spread2_;
        std::vector<Real> cappedRate1_, flooredRate1_, cappedRate2_,
            flooredRate2_;
        DayCounter pmtDayCount1_, pmtDayCount2_;
		Natural pmtLag1_, pmtLag2_;
        bool inArrears1_, inArrears2_;
        std::vector<bool> isRedemptionFlow1_, isRedemptionFlow2_;
        BusinessDayConvention pmtConv1_, pmtConv2_;
		Calendar pmtCal1_, pmtCal2_;
        const bool intermediateCapitalExchange_, finalCapitalExchange_;
    };

    //! %Arguments for float float swap calculation
    class FloatFloatSwapDS::arguments : public Swap::arguments {
      public:
        arguments() : type(VanillaSwap::Receiver) {}
        VanillaSwap::Type type;
        std::vector<Real> nominal1, nominal2;
		Real nominalFinal1, nominalFinal2;

        std::vector<Date> leg1ResetDates, leg1FixingDates, leg1PayDates;
        std::vector<Date> leg2ResetDates, leg2FixingDates, leg2PayDates;

        std::vector<Real> leg1Spreads, leg2Spreads, leg1Gearings, leg2Gearings;
        std::vector<Real> leg1CappedRates, leg1FlooredRates, leg2CappedRates,
            leg2FlooredRates;

        std::vector<Real> leg1Coupons, leg2Coupons;
        std::vector<Real> leg1AccrualTimes, leg2AccrualTimes;

        ext::shared_ptr<InterestRateIndex> index1, index2;

        std::vector<bool> leg1IsRedemptionFlow, leg2IsRedemptionFlow;

        void validate() const;
    };

    //! %Results from float float swap calculation
    class FloatFloatSwapDS::results : public Swap::results {
      public:
        void reset();
    };

    class FloatFloatSwapDS::engine
        : public GenericEngine<FloatFloatSwapDS::arguments,
                               FloatFloatSwapDS::results> {};

    // inline definitions

    inline VanillaSwap::Type FloatFloatSwapDS::type() const { return type_; }

    inline const std::vector<Real> &FloatFloatSwapDS::nominal1() const {
        return nominal1_;
    }

    inline const std::vector<Real> &FloatFloatSwapDS::nominal2() const {
        return nominal2_;
    }
	
    inline Real const FloatFloatSwapDS::nominalFinal1() const { return nominalFinal1_; }
    inline Real const FloatFloatSwapDS::nominalFinal2() const { return nominalFinal2_; }

    inline const Schedule &FloatFloatSwapDS::schedule1() const {
        return schedule1_;
    }

    inline const Schedule &FloatFloatSwapDS::schedule2() const {
        return schedule2_;
    }

    inline const ext::shared_ptr<InterestRateIndex> &
    FloatFloatSwapDS::index1() const {
        return index1_;
    }

    inline const ext::shared_ptr<InterestRateIndex> &
    FloatFloatSwapDS::index2() const {
        return index2_;
    }

    inline std::vector<Real> FloatFloatSwapDS::spread1() const { return spread1_; }

    inline std::vector<Real> FloatFloatSwapDS::spread2() const { return spread2_; }

    inline std::vector<Real> FloatFloatSwapDS::gearing1() const { return gearing1_; }

    inline std::vector<Real> FloatFloatSwapDS::gearing2() const { return gearing2_; }

    inline std::vector<Real> FloatFloatSwapDS::cappedRate1() const { return cappedRate1_; }

    inline std::vector<Real> FloatFloatSwapDS::cappedRate2() const { return cappedRate2_; }

    inline std::vector<Real> FloatFloatSwapDS::flooredRate1() const { return flooredRate1_; }

    inline std::vector<Real> FloatFloatSwapDS::flooredRate2() const { return flooredRate2_; }

    inline const DayCounter &FloatFloatSwapDS::pmtDayCount1() const {
        return pmtDayCount1_;
    }

    inline const DayCounter &FloatFloatSwapDS::pmtDayCount2() const {
        return pmtDayCount2_;
    }
	
	inline Natural const FloatFloatSwapDS::pmtLag1() const {
        return pmtLag1_;
	}
	
	inline Natural const FloatFloatSwapDS::pmtLag2() const {
        return pmtLag2_;
	}

	inline bool const FloatFloatSwapDS::inArrears1() const {
        return inArrears1_;
	}

	inline bool const FloatFloatSwapDS::inArrears2() const {
        return inArrears2_;
	}

    inline BusinessDayConvention FloatFloatSwapDS::pmtConv1() const {
        return pmtConv1_;
    }

    inline BusinessDayConvention FloatFloatSwapDS::pmtConv2() const {
        return pmtConv2_;
    }

    inline Calendar const & FloatFloatSwapDS::pmtCal1() const {
        return pmtCal1_;
    }

    inline Calendar const & FloatFloatSwapDS::pmtCal2() const {
        return pmtCal2_;
    }

    inline const Leg &FloatFloatSwapDS::leg1() const { return legs_[0]; }

    inline const Leg &FloatFloatSwapDS::leg2() const { return legs_[1]; }
}

#endif
