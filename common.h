#pragma once

namespace gtree {

	struct Void {
		Void operator+ (const Void& d) const {
			return *this;
		}
	};

	template<class ValueT, class CumulativeValueT>
	struct NoUpdate {
		NoUpdate operator+ (const NoUpdate& d) const {
			return *this;
		}
		ValueT operator+ (const ValueT& d) const {
			return d;
		}
		CumulativeValueT operator+ (const CumulativeValueT& d) const {
			return d;
		}
	};

	template<class T1, class T2, class Result>
	struct _gtree_plus {
		Result operator() (const T1& a, const T2& b) const {
			return a + b;
		}
	};

	template<class IndexT>
	struct Range {
		// 0 - no bound; 1 - inclusive; 2 - exclusive
		IndexT l_val;
		IndexT r_val;
		char l_type;
		char r_type;

		Range(char l_type, IndexT l_val, char r_type, IndexT r_val) :
			l_val(l_val), r_val(r_val), l_type(l_type), r_type(r_type) {}
	};





}