#pragma once
#include "String.h"
#include "EditableResource.h"
#include "Defines.h"

namespace rstring {

#define _rstr(stringConstant,...) String<char, char>::Construct(RESOURCE_STRING_CONSTANT(stringConstant), ##__VA_ARGS__)
#define _rstrw(stringConstant,...) String<char, wchar_t>::Construct(RESOURCE_STRING_CONSTANT(stringConstant), ##__VA_ARGS__)
#define _wrstr(stringConstant,...) String<wchar_t, char>::Construct(BOOST_PP_CAT(L, RESOURCE_STRING_CONSTANT(stringConstant)), ##__VA_ARGS__)
#define _wrstrw(stringConstant,...) String<wchar_t, wchar_t>::Construct(BOOST_PP_CAT(L, RESOURCE_STRING_CONSTANT(stringConstant)), ##__VA_ARGS__)


	typedef String<char, char> _rstr_t;
	typedef String<char, wchar_t> _rstrw_t;
	typedef String<wchar_t, char> _wrstr_t;
	typedef String<wchar_t, wchar_t> _wrstrw_t;
}