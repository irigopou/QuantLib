//Added by Deriscope

#include <ql/indexes/oiswapindexds.hpp>
#include <ql/instruments/oiswapds.hpp>
#include <ql/instruments/makeoiswapds.hpp>
#include <ql/indexes/iborindex.hpp>

namespace QuantLib {

	OiSwapIndexDS::OiSwapIndexDS( 
        const std::string& familyName,
        const Period& tenor,
        Natural settlementDays,
        const Currency& currency,
        const ext::shared_ptr<OvernightIndex>& overnightIndex,
		Period const & oiLegPeriod,
		Natural rateCutoff,
        TypeOI typeOI, 
		ApproxOI approxOI ) 
    : SwapIndex(familyName,
                tenor,
                settlementDays,
                currency,
                overnightIndex->fixingCalendar(),
                oiLegPeriod,
                ModifiedFollowing,
                overnightIndex->dayCounter(),
                overnightIndex),
				overnightIndex_(overnightIndex), rateCutoff_(rateCutoff), typeOI_(typeOI), approxOI_(approxOI) {}
	
    ext::shared_ptr<OiSwapDS> OiSwapIndexDS::underlyingSwap(const Date& fixingDate) const {
        QL_REQUIRE(fixingDate!=Date(), "null fixing date");
        // caching mechanism
        if (lastFixingDate_!=fixingDate) {
            Rate fixedRate = 0.0;
			lastSwap_ = MakeOiSwapDS(tenor_, overnightIndex_, fixedRate)
				.withRateCutoff(rateCutoff_).withTypeOI(typeOI_).withApproxOI(approxOI_)
                .withEffectiveDate(valueDate(fixingDate))
                .withFixedLegDayCount(dayCounter_);
            lastFixingDate_ = fixingDate;
        }
        return lastSwap_;
    }

}
