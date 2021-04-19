//Added by Deriscope

#ifndef quantlib_oitermindexds_hpp
#define quantlib_oitermindexds_hpp

#include <ql/indexes/oiswapindexds.hpp>

namespace QuantLib {
	
	//It restricts the base OiSwapIndexDS to represent the fair fixed rate of a single-period OIS.
	//In effect, it represents the compounded or averaged overnight rate over that period
    class OiTermIndexDS : public OiSwapIndexDS {
      public:

        OiTermIndexDS(
            const std::string& familyName,
			const Period& tenor,
            Natural settlementDays,
            const Currency& currency,
            const ext::shared_ptr<OvernightIndex>& overnightIndex,
			Natural rateCutoff,
            TypeOI typeOI, 
			ApproxOI approxOI);


    };

}

#endif
