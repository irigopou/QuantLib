//Added by Deriscope

#ifndef quantlib_oiswapds_hpp
#define quantlib_oiswapds_hpp

#include <ql/instruments/swap.hpp>
#include <ql/time/daycounter.hpp>
#include <ql/time/businessdayconvention.hpp>
#include <ql/time/calendar.hpp>
#include <ql/time/schedule.hpp>
#include <ql/indexes/oiswapindexds.hpp>

namespace QuantLib {

    class Schedule;
    class OvernightIndex;
	
	//It broadens the original OvernightIndexedSwap. Not possible to inherit from OvernightIndexedSwap because the latter's contructor involves initialize(schedule) which sets the types of coupons
	//It represents a generic OIS where the overnight index is either compounded or averaged over each overnight leg period
    class OiSwapDS : public Swap {
    public:
        enum Type { Receiver = -1, Payer = 1 };
		
        OiSwapDS(
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
			OiSwapIndexDS::ApproxOI approxOI);

        OiSwapDS(
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
			OiSwapIndexDS::ApproxOI approxOI);

        //name Inspectors
		//both legs
        Type const type() const { return type_; }
        std::vector<Real> const & nominals() const { return nominals_; }
		Real nominal() const { QL_REQUIRE(flat_nominals_, "no single nominal"); return nominals_[0]; }
		//fixed leg
        std::vector<Real> const & fxdRates() const { return fxdRates_; }
		Real const fxdRate() const { QL_REQUIRE(flat_fxdRates_, "no single fixed rate"); return fxdRates_[0]; }
        Schedule const & fxdSchedule() const { return fxdSchedule_; }
		Natural const fxdPmtLag() const { return fxdPmtLag_; }
		BusinessDayConvention const fxdPmtAdj() const { return fxdPmtAdj_; }
		Calendar const & fxdPmtCal() const { return fxdPmtCal_; }
		DayCounter const & fxdPmtDC() const { return fxdPmtDC_; }
		bool const hasFlatFxdRate() const { return flat_fxdRates_; }
		//on leg
        ext::shared_ptr<OvernightIndex> const & index() const { return index_; }
        std::vector<Real> const & gearings() const { return gearings_; }
		Real gearing() const { QL_REQUIRE(flat_gearings_, "no single gearing"); return gearings_[0]; }
        std::vector<Spread> const & spreads() const { return spreads_; }
		Spread spread() const { QL_REQUIRE(flat_spreads_, "no single spread"); return spreads_[0]; }
        Schedule const & onSchedule() const { return onSchedule_; }
		Natural const onPmtLag() const { return onPmtLag_; }
		BusinessDayConvention const onPmtAdj() const { return onPmtAdj_; }
		Calendar const & onPmtCal() const { return onPmtCal_; }
		DayCounter const & onPmtDC() const { return onPmtDC_; }
		//special on flags
		const Natural rateCutoff() const { return rateCutoff_; }
        OiSwapIndexDS::TypeOI typeOI() const { return typeOI_; }
        OiSwapIndexDS::ApproxOI approxOI() const { return approxOI_; }
		bool const hasFlatSpread() const { return flat_spreads_; }
		//legs
        const Leg& fixedLeg() const { return legs_[0]; }
        const Leg& overnightLeg() const { return legs_[1]; }

        //name Results
        Real fixedLegBPS() const;
        Real fixedLegNPV() const;
        Real fairRate() const;//throws if flat_fxdRates_ == false

        Real overnightLegBPS() const;
        Real overnightLegNPV() const;
        Spread fairSpread() const;//throws if flat_spreads_ == false

      private:
        void initialize();
		//both legs
        Type type_;
        std::vector<Real> nominals_;
		bool flat_nominals_;
		//fixed leg
        std::vector<Real> fxdRates_;
		bool flat_fxdRates_;
		Schedule fxdSchedule_;
        Natural fxdPmtLag_;
		BusinessDayConvention fxdPmtAdj_;
        Calendar fxdPmtCal_;
        DayCounter fxdPmtDC_;
		//on leg
        ext::shared_ptr<OvernightIndex> index_;
        std::vector<Real> gearings_;
		bool flat_gearings_;
        std::vector<Spread> spreads_;
		bool flat_spreads_;
		Schedule onSchedule_;
        Natural onPmtLag_;
		BusinessDayConvention onPmtAdj_;
        Calendar onPmtCal_;
        DayCounter onPmtDC_;
		//special on flags
		Natural rateCutoff_;
        OiSwapIndexDS::TypeOI typeOI_;
		OiSwapIndexDS::ApproxOI approxOI_;
    };


}

#endif
