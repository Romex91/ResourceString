#pragma once

#include <boost/container/flat_map.hpp>
#include <boost/serialization/split_member.hpp>


namespace rstring
{
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


		//////////////////////////////////////////////////////////////////////////
		//public methods needed by the String class
		//////////////////////////////////////////////////////////////////////////

		//get the numeric id of the unique format string
		virtual size_t getId(const IdString & s) const
		{
			Map::const_iterator keyValuePair = _strings.find(s);
			if (keyValuePair == _strings.end()) {
				throw std::out_of_range("Error: string is absent in the resource. ");
			} 
			return keyValuePair - _strings.begin();
		}

		//get the format string translation
		virtual const TextString & getText(const IdString & s)	const
		{
			return _strings.at(s);
		}

		//get the unique format string using it's numeric id
		virtual const IdString & getStringId(size_t id) const
		{
			return (_strings.begin() + id)->first;
		}

		//////////////////////////////////////////////////////////////////////////
		//destructor
		//////////////////////////////////////////////////////////////////////////

		virtual ~Resource(){}

	protected:
		//////////////////////////////////////////////////////////////////////////
		//protected data
		//////////////////////////////////////////////////////////////////////////

		Map _strings;


		//////////////////////////////////////////////////////////////////////////
		//boost serialization methods
		//////////////////////////////////////////////////////////////////////////

		friend class boost::serialization::access;

		//boost contains no serialization for flat_map =(
		template<class Archive>
		void save(Archive & ar, const unsigned int version) const
		{
			Map::size_type stringsNumber = _strings.size();
			ar & BOOST_SERIALIZATION_NVP(stringsNumber);
			for each (auto string in _strings)
			{
				ar & BOOST_SERIALIZATION_NVP(string);
			}
		}

		template<class Archive>
		void load(Archive & ar, const unsigned int version)
		{
			Map::size_type stringsNumber;
			ar & BOOST_SERIALIZATION_NVP(stringsNumber);
			for (Map::size_type i = 0; i < stringsNumber; i++)
			{
				Map::value_type string;
				ar & BOOST_SERIALIZATION_NVP(string);
				_strings.insert(string);
			}
		}
		BOOST_SERIALIZATION_SPLIT_MEMBER();

	};
}
