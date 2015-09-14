#include <boost/preprocessor/stringize.hpp>
#if !defined(RESOURCE_STRING_PREFIX)
//the prefix to add to a string constant when printing to the compiler output
#define RESOURCE_STRING_PREFIX _rstr_begin---:
#endif

#if !defined(RESOURCE_STRING_POSTFIX)
//the postfix to add to a string constant when printing to the compiler output
#define RESOURCE_STRING_POSTFIX :---_rstr_end
#endif


#if defined(PRINT_RESOURCE_STRINGS) 

#define RESOURCE_STRING_FORMAT(x)\
	BOOST_PP_STRINGIZE(RESOURCE_STRING_PREFIX) x BOOST_PP_STRINGIZE(RESOURCE_STRING_POSTFIX)

#if defined(_MSC_VER)
//WINDOWS
//the macro to print a string constant before using it
#define RESOURCE_STRING_CONSTANT(x) \
	__pragma(message(RESOURCE_STRING_FORMAT(x)))\
	x

#else//defined(_MSC_VER)
//on the other operating systems we have no better alternative than generating a warning

#define DEPRECATE(foo, msg) foo __attribute__((deprecated(msg)))
//the macro to generate a warning with a string constant before using it
#define RESOURCE_STRING_CONSTANT(x) [](){ struct __rstr_print {\
	DEPRECATE(void _(), RESOURCE_STRING_FORMAT(x)){};\
	void __(){_();}};return x;}()

#endif//defined(_MSC_VER)

#else //defined(PRINT_RESOURCE_STRINGS) 
//do nothing. Define PRINT_RESOURCE_STRINGS to print string constants to the compiler output.
#define RESOURCE_STRING_CONSTANT(x) x
#endif //defined(PRINT_RESOURCE_STRINGS)