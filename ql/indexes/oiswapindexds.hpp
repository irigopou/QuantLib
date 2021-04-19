//Added by Deriscope

#ifndef quantlib_oiswapindexds_hpp
#define quantlib_oiswapindexds_hpp

#include <ql/indexes/swapindex.hpp>

namespace QuantLib {

    class OiSwapDS;
	
	//It is meant to be used instead of the original OvernightIndexedSwapIndex
	//It is not derived from it because the latter's constructor hard codes a one-year period for the overnight leg!
	//It represents the fair fixed rate of a generic OIS where the overnight index is either compounded or averaged over each overnight leg period
    class OiSwapIndexDS : public SwapIndex {
      public:
		enum TypeOI { AveragingOI, CompoundingOI };
		enum ApproxOI { TelescopicOI, TakadaOI, NoneOI };
		
        OiSwapIndexDS(
            const std::string& familyName,
            const Period& tenor,
            Natural settlementDays,
            const Currency& currency,
            const ext::shared_ptr<OvernightIndex>& overnightIndex,
			Period const & oiLegPeriod,
			Natural rateCutoff,
            TypeOI typeOI, 
			ApproxOI approxOI);

        //name Inspectors
		ext::shared_ptr<OvernightIndex> overnightIndex() const { return overnightIndex_; }
		const Natural rateCutoff() const { return rateCutoff_; }
		const TypeOI typeOI() const { return typeOI_; }
		const ApproxOI approxOI() const { return approxOI_; }
		
        ext::shared_ptr<OiSwapDS> underlyingSwap(const Date& fixingDate) const;

      protected:
        ext::shared_ptr<OvernightIndex> overnightIndex_;
		Natural rateCutoff_;
        TypeOI typeOI_;
        ApproxOI approxOI_;
        // cache data to avoid swap recreation when the same fixing date
        // is used multiple time to forecast changing fixing
        mutable ext::shared_ptr<OiSwapDS> lastSwap_;
        mutable Date lastFixingDate_;
    };

}

#endif
