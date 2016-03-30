#include "stdafx.h"
#include "wascore/constants.h"

namespace azure { namespace storage { namespace protocol {

    #define DAT(a, b) WASTORAGE_API const utility::string_t a(b);
	#include "wascore/constants.dat"
	#undef DAT

}}} // namespace azure::storage::protocol
