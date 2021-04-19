//Added by Deriscope

#include <ql/indexes/oitermindexds.hpp>

namespace QuantLib {
	
	OiTermIndexDS::OiTermIndexDS( 
        const std::string& familyName,
        const Period& tenor,
        Natural settlementDays,
        const Currency& currency,
        const ext::shared_ptr<OvernightIndex>& overnightIndex,
		Natural rateCutoff,
        TypeOI typeOI, 
		ApproxOI approxOI ) 
    : OiSwapIndexDS(familyName,
                tenor,
                settlementDays,
                currency,
                overnightIndex,
                tenor,
				rateCutoff,
                typeOI,
                approxOI) {}

}
