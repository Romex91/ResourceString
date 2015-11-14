#pragma once
#include <sstream>
#include <boost/container/flat_map.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/map.hpp>
#include <type_traits>


#include "Resource.h"
namespace rstring
{

	//resource string
	//purposes of this class:
	// - simple text formatting
	// - easy language selection
	// - serialization using numeric ids
	// - avoid non-ASCII characters in the source code
	//when using this class in a client-server application it is important to keep the resources identical on the both sides
	//
	// the _Resource class should implement the next methods:
	//size_t getId(const std::basic_string<_IdChar> & s) const
	//const std::basic_string<_TextChar> & getText(const std::basic_string<_IdChar> & s) const
	//const std::basic_string<_IdChar> & getStringId(size_t id) const
	template<class _IdChar, class _TextChar, class _Resource = Resource<_IdChar, _TextChar> >
	class String
	{
	public:

		//////////////////////////////////////////////////////////////////////////
		//type definitions
		//////////////////////////////////////////////////////////////////////////

		typedef String<_IdChar, _TextChar, _Resource> This;
		typedef std::basic_string<_IdChar> IdString;
		typedef std::basic_string<_TextChar> TextString;
		typedef std::basic_stringstream<_TextChar> TextStringStream;

		//////////////////////////////////////////////////////////////////////////
		//constructors and destructor
		//////////////////////////////////////////////////////////////////////////

		//idString should contain placeholders similar to {0}
		//it is suitable to pass another resource string as an argument
		template < typename... Args>
		static This Construct(const IdString & idString, Args ... args)
		{
			return This(idString, args...);
		}

		static This Construct(const IdString & idString)
		{
			return This(idString);
		}

		String() = default;
		String(const String &) = default;
		virtual ~String(){};


		//////////////////////////////////////////////////////////////////////////
		//public methods
		//////////////////////////////////////////////////////////////////////////

		//translate format string and fill it with the arguments earlier passed to the constructor
		TextString str()const
		{
			TextString format = resource().getText(_idString);
			
			replaceArguments<This>(format, _nestedStringArguments);
			replaceArguments< std::string>(format, _stringArguments);
			replaceArguments<double>(format, _floatingPointArguments);
			replaceArguments<long long>(format, _integerArguments);
#if !defined(RESOURCE_STRING_DISABLE_WSTRING_ARGUMENTS)
			replaceArguments<std::wstring>(format, _wstringArguments);
#endif
			return format;
		}
		
		//a resource containing format strings translations 
		static _Resource & resource()
		{
			static _Resource _resource;
			return _resource;
		}

	protected:


		//////////////////////////////////////////////////////////////////////////
		//protected data
		//////////////////////////////////////////////////////////////////////////

		//nested resource string
		//contains arguments of the same type
		std::map<size_t, This> _nestedStringArguments;

		std::map<size_t, std::string> _stringArguments;
		std::map<size_t, double> _floatingPointArguments;
		std::map<size_t, long long> _integerArguments;

#if !defined(RESOURCE_STRING_DISABLE_WSTRING_ARGUMENTS)
		std::map<size_t, std::wstring> _wstringArguments;
#endif
		//unique format string
		IdString _idString;

		//////////////////////////////////////////////////////////////////////////
		//hidden constructors
		//////////////////////////////////////////////////////////////////////////

		//the problem is there are two ways of calling constructors in C++:
		// - String<char,char> foo([some arguments]);
		// - String<char,char> foo = String<char,char>([some arguments]);
		//since we want to print string constants at the compile time the first one is inappropriate
		//so we hide constructors at all and use static function Construct instead

		//constructor with multiple arguments
		//idString should contain placeholders similar to {0}
		//it is suitable to pass another resource string as an argument
		template < typename... Args>
		explicit String(const IdString & idString, Args ... args) : String(idString)
		{
			initializeArguments(0, args...);
		}

		//constructor without arguments
		explicit String(const IdString & idString) : _idString(idString)
		{
		}

		//////////////////////////////////////////////////////////////////////////
		//methods used in constructors
		//////////////////////////////////////////////////////////////////////////

		//save arguments recursively
		template < typename First, typename... Other>
		void initializeArguments(size_t index, const First & first, Other ... other)
		{
			initializeArguments(index++, first);
			initializeArguments(index, other...);
		}

		//save an argument of a simple type
		template <typename Argument>
		void initializeArguments(size_t index, const Argument & argument)
		{
			static_assert(std::is_arithmetic<Argument>::value || std::is_enum<Argument>::value, 
				"unsupported argument type.");
			if (std::is_integral<Argument>::value) {
				_integerArguments[index] = static_cast<long long>(argument);
			} else {
				_floatingPointArguments[index] = static_cast<double>(argument);
			}
		}

		//save a nested resource string as an argument
		void initializeArguments(size_t index, const This & argument)
		{
			_nestedStringArguments[index] = argument;
		}

		void initializeArguments(size_t index, const std::string & argument)
		{
			_stringArguments[index] = argument;
		}

		void initializeArguments(size_t index, const char * argument)
		{
			initializeArguments(index, std::string(argument));
		}

		void initializeArguments(size_t index, const wchar_t * argument)
		{
			initializeArguments(index, std::wstring(argument));
		}

		void initializeArguments(size_t index, const std::wstring & argument)
		{
#if !defined(RESOURCE_STRING_DISABLE_WSTRING_ARGUMENTS)
			_wstringArguments[index] = argument;
#else
			_stringArguments[index] = helpers::convert<std::string, std::wstring>(argument)
#endif
		}

		//////////////////////////////////////////////////////////////////////////
		//methods to convert arguments to TextString when str() is called
		//////////////////////////////////////////////////////////////////////////
		template<class T>
		static TextString convertToTextString(const T & value)
		{
			TextStringStream ss;
			ss << value;
			return ss.str();
		}

		static TextString convertToTextString(const std::string & value)
		{
			return helpers::convert<std::string, TextString >(value);
		}

		static TextString convertToTextString(const std::wstring & value)
		{
			return helpers::convert<std::wstring, TextString>(value);
		}

		static TextString convertToTextString(const This & value)
		{
			return value.str();
		}

		template < class T >
		static void replaceArguments(TextString & text, const std::map<size_t, T> & argumentsList)
		{
			for (auto simpleArgument : argumentsList)
			{
				TextStringStream ss;
				ss << "{" << simpleArgument.first << "}";
				boost::replace_all(text, ss.str(), convertToTextString(simpleArgument.second));
			}
		}

		//////////////////////////////////////////////////////////////////////////
		//boost serialization methods
		//////////////////////////////////////////////////////////////////////////
		friend class boost::serialization::access;

		template<class Archive>
		void save(Archive & ar, const unsigned int version) const
		{
			auto id = resource().getId(_idString);
			ar << BOOST_SERIALIZATION_NVP(id);
			ar << BOOST_SERIALIZATION_NVP(_nestedStringArguments);

			ar << BOOST_SERIALIZATION_NVP(_stringArguments);
			ar << BOOST_SERIALIZATION_NVP(_floatingPointArguments);
			ar << BOOST_SERIALIZATION_NVP(_integerArguments);

#if !defined(RESOURCE_STRING_DISABLE_WSTRING_ARGUMENTS)
			ar << BOOST_SERIALIZATION_NVP(_wstringArguments);
#endif
		}

		template<class Archive>
		void load(Archive & ar, const unsigned int version)
		{
			size_t id;
			ar >> BOOST_SERIALIZATION_NVP(id);
			_idString = resource().getStringId(id);

			ar >> BOOST_SERIALIZATION_NVP(_nestedStringArguments);
			ar >> BOOST_SERIALIZATION_NVP(_stringArguments);
			ar >> BOOST_SERIALIZATION_NVP(_floatingPointArguments);
			ar >> BOOST_SERIALIZATION_NVP(_integerArguments);

#if !defined(RESOURCE_STRING_DISABLE_WSTRING_ARGUMENTS)
			ar >> BOOST_SERIALIZATION_NVP(_wstringArguments);
#endif
		}
		BOOST_SERIALIZATION_SPLIT_MEMBER();

	};


}
