#pragma once

#include <boost/container/flat_map.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/algorithm/string/trim_all.hpp>
#include <boost/locale.hpp>

namespace rstring
{
	namespace helpers
	{
		template<class _SrcString, class _DestString >
		inline _DestString convert(const _SrcString & string)
		{
			return string;
		}

		template<>
		inline std::string convert<std::wstring, std::string>(const std::wstring & string)
		{
			return boost::locale::conv::from_utf<wchar_t>(string, std::locale());
		}

		template<>
		inline std::wstring convert<std::string, std::wstring>(const std::string & string)
		{
			return boost::locale::conv::to_utf<wchar_t>(string, std::locale());
		}
	}

	template<class _IdChar, class _TextChar>
	class Resource
	{
	public:

		//////////////////////////////////////////////////////////////////////////
		//type definitions
		//////////////////////////////////////////////////////////////////////////

		typedef std::basic_string<_IdChar> IdString;
		typedef std::basic_string<_TextChar> TextString;
		typedef boost::container::flat_map < IdString, TextString > Map;

		enum class ErrorBehavior{
			THROW_EXCEPTION,
			RETURN_ERROR_TEXT,
			CONVERT_ID_TEXT
		};
		//////////////////////////////////////////////////////////////////////////
		//public methods needed by the String class
		//////////////////////////////////////////////////////////////////////////

		//get the numeric id of the unique format string
		//string id is trimming when searching the numering id
		size_t getId(const IdString & s) const
		{
			auto findResult = find(s);
			if (findResult == _strings.end()) {
				throw std::out_of_range(errorNoString);
			}
			return findResult - _strings.begin();
		}

		//get the format string translation
		//string id is trimming when searching the text
		TextString getText(const IdString & s)	const
		{
			auto findResult = find(s);
			if (findResult == _strings.end()) {
				switch (_errorBehavior())
				{
				case ErrorBehavior::THROW_EXCEPTION: 
					throw std::out_of_range(errorNoString);
					break;
				case  ErrorBehavior::RETURN_ERROR_TEXT:
					return rstring::helpers::convert<std::string, TextString>(errorNoString);
					break;
				case ErrorBehavior::CONVERT_ID_TEXT:
					return rstring::helpers::convert<IdString, TextString>(s);
				}
				
			}
			return findResult->second;
		}

		//get the unique format string using it's numeric id
		const IdString & getStringId(size_t id) const
		{
			return (_strings.begin() + id)->first;
		}

		//////////////////////////////////////////////////////////////////////////
		//other public methods
		//////////////////////////////////////////////////////////////////////////

		static void setErrorBehavior(ErrorBehavior errorBehavior) 
		{
			_errorBehavior() = errorBehavior;
		}

		size_t hash() const
		{
			size_t result = 0;
			std::hash<std::string> hash_fn;
			for (auto & item : _strings)
			{
				result += hash_fn(item.first);
			}
			return result;
		}
		//////////////////////////////////////////////////////////////////////////
		//destructor
		//////////////////////////////////////////////////////////////////////////

		virtual ~Resource(){}

	protected:
		std::string errorNoString = "Error: string is absent in the resource. ";
		static ErrorBehavior & _errorBehavior() {
			static ErrorBehavior retval = ErrorBehavior::RETURN_ERROR_TEXT;
			return retval;
		}
		Map _strings;

		typename Map::const_iterator find(const IdString & s) const
		{
			typename Map::const_iterator retval;
			for (retval = _strings.begin(); retval < _strings.end(); retval++)
			{
				if ( boost::algorithm::trim_all_copy(retval->first) ==
					boost::algorithm::trim_all_copy(s))
				{
					break;
				}
			}
			return retval;
		}

		//////////////////////////////////////////////////////////////////////////
		//boost serialization methods
		//////////////////////////////////////////////////////////////////////////

		friend class boost::serialization::access;

		//boost contains no serialization for flat_map =(
		template<class Archive>
		void save(Archive & ar, const unsigned int version) const
		{
			size_t stringsNumber = _strings.size();
			ar & BOOST_SERIALIZATION_NVP(stringsNumber);
			for (auto & string : _strings)
			{
				ar & BOOST_SERIALIZATION_NVP(string);
			}
		}

		template<class Archive>
		void load(Archive & ar, const unsigned int version)
		{
			_strings.clear();
			size_t stringsNumber;
			ar & BOOST_SERIALIZATION_NVP(stringsNumber);
			for (size_t i = 0; i < stringsNumber; i++)
			{
				typename Map::value_type string;
				ar & BOOST_SERIALIZATION_NVP(string);
				_strings.insert(string);
			}
		}
		BOOST_SERIALIZATION_SPLIT_MEMBER();

	};
}
