#pragma once
#include <boost/container/flat_map.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/map.hpp>
#include <sstream>
#include "Resource.h"
namespace rstring
{
	namespace helpers
	{
		template<class _SrcString, class _DestString >
		_DestString convert(const _SrcString & string)
		{
			return string;
		}

		template<>
		std::string convert<std::wstring, std::string>(const std::wstring & string)
		{
			return std::wstring_convert<std::codecvt<wchar_t, char, std::mbstate_t>>().to_bytes(string);
		}

		template<>
		std::wstring convert<std::string, std::wstring>(const std::string & string)
		{
			return std::wstring_convert<std::codecvt<wchar_t, char, std::mbstate_t>>().from_bytes(string);
		}


	}

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
			for (auto simpleArgument : _simpleArguments) 
			{
				TextStringStream ss;
				ss << "{" << simpleArgument.first << "}";
				boost::replace_all(format, ss.str(), simpleArgument.second);
			}

			for (auto nestedStringArgument : _nestedStringArguments)
			{
				TextStringStream ss;
				ss << "{" << nestedStringArgument.first << "}";
				boost::replace_all(format, ss.str(), nestedStringArgument.second.str());
			}
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

		//contains arguments of other types converted to the type TextString
		std::map<size_t, TextString> _simpleArguments;

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
			TextStringStream ss;
			ss << argument;
			_simpleArguments[index] = ss.str();
		}

		//save a nested resource string as an argument
		void initializeArguments(size_t index, const This & argument)
		{
			_nestedStringArguments[index] = argument;
		}

		void initializeArguments(size_t index, const IdString & argument)
		{
			_simpleArguments[index] = helpers::convert<IdString, TextString>(argument);
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
			ar << BOOST_SERIALIZATION_NVP(_simpleArguments);
			ar << BOOST_SERIALIZATION_NVP(_nestedStringArguments);
		}

		template<class Archive>
		void load(Archive & ar, const unsigned int version)
		{
			size_t id;
			ar >> BOOST_SERIALIZATION_NVP(id);
			_idString = resource().getStringId(id);
			ar >> BOOST_SERIALIZATION_NVP(_simpleArguments);
			ar >> BOOST_SERIALIZATION_NVP(_nestedStringArguments);
		}
		BOOST_SERIALIZATION_SPLIT_MEMBER();

	};


}
