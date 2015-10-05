#pragma once
#include <regex>
#include <type_traits>
#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/stringize.hpp>
#include <boost/preprocessor/wstringize.hpp>
#include "Defines.h"


namespace rstring
{
	namespace helpers
	{
		//////////////////////////////////////////////////////////////////////////
		//type-specific helpers
		//////////////////////////////////////////////////////////////////////////
		template<class _Char>
		inline std::basic_regex<_Char> getQoutedStringRegex();

		template<>
		inline std::basic_regex<char> getQoutedStringRegex<char>()
		{
			return std::regex(R"d("(?:[^"\\]|\\.|\\\n|\\\r\n)*")d");
		}

		template<>
		inline std::basic_regex<wchar_t> getQoutedStringRegex<wchar_t>()
		{
			return std::wregex(LR"d("(?:[^"\\]|\\.|\\\n|\\\r\n)*")d");
		}

		template<class _Char>
		inline std::pair<std::basic_string<_Char>, std::basic_string<_Char>> getPrefixPostfixPair();

		template<>
		inline std::pair<std::basic_string<char>, std::basic_string<char>> getPrefixPostfixPair<char>()
		{
			return std::pair<std::basic_string<char>, std::basic_string<char>>(BOOST_PP_STRINGIZE(RESOURCE_STRING_PREFIX), BOOST_PP_STRINGIZE(RESOURCE_STRING_POSTFIX));
		}

		template<>
		inline std::pair<std::basic_string<wchar_t>, std::basic_string<wchar_t>> getPrefixPostfixPair<wchar_t>()
		{
			return std::pair<std::basic_string<wchar_t>, std::basic_string<wchar_t>>(BOOST_PP_WSTRINGIZE(RESOURCE_STRING_PREFIX), BOOST_PP_WSTRINGIZE(RESOURCE_STRING_POSTFIX));
		}
	}

	template<class _IdChar, class _TextChar>
	class EditableResource : public Resource < _IdChar, _TextChar >
	{
	public:
		//////////////////////////////////////////////////////////////////////////
		//type definitions
		//////////////////////////////////////////////////////////////////////////

		typedef std::basic_string<_IdChar> IdString;
		typedef std::basic_string<_TextChar> TextString;
		typedef boost::container::flat_map < IdString, TextString > Map;

		//////////////////////////////////////////////////////////////////////////
		//public methods
		//////////////////////////////////////////////////////////////////////////

		//run merging console dialog
		//looks for strings with an identical key and different texts and suggest user to choose which one to save  
		void merge(const EditableResource<_IdChar, _TextChar> & resource)
		{
			//suggest what to do about conflicting strings
			std::cout << "Conflicting strings:" << std::endl;
			for (auto & rightStringsPair : resource._strings) {
				typename Map::const_iterator leftStringsPair;
				if ((leftStringsPair = find(rightStringsPair.first)) != this->_strings.end()) {
					if (leftStringsPair->second != rightStringsPair.second && rightStringsPair.second != TextString()) {
						const int saveLeft = 1, saveRight = 2;
						int choise = 0;
						while (choise != saveLeft && choise != saveRight)
						{
							std::cout << "two items has the same id and different texts" <<std::endl;
							std::cout << "select the strings pair to save in the resource:" << std::endl << saveLeft;
							print(*leftStringsPair);
							std::cout << saveRight;
							print(rightStringsPair);
							std::cin >> choise;
						}

						if (choise == saveRight)
						{
							this->_strings.erase(leftStringsPair);
							this->_strings.insert(NewStringsPair(rightStringsPair));
						}
					}
				}
			}
		}

		//copy the strings which do not exist in this resource
		void update(const EditableResource<_IdChar, _TextChar> & resource)
		{
			
			std::cout << "New strings:" << std::endl;
			for (auto rightStringPair : resource._strings) {
				auto leftStringPair = find(rightStringPair.first);
				if (leftStringPair != this->_strings.end()) {
					//check if the currently persistent pair has an empty text value
					//and we have a text on the right side
					if (leftStringPair->second == TextString() && rightStringPair.second != TextString()) {
						this->_strings.erase(leftStringPair);
					}
					else {
						continue;
					}
				}
				this->_strings.insert(print(NewStringsPair(rightStringPair)));
			}
		}

		//print the strings which exist in this resource but not in the resource passed as an argument
		void printOrphanedStrings(const EditableResource<_IdChar, _TextChar> & resource)
		{
			std::cout << "Orphaned strings:" << std::endl;
			for (auto leftStringsPair : this->_strings) {
				if (resource.find(leftStringsPair.first) == resource._strings.end()) {
					print(leftStringsPair);
				}
			}
		}

		//parses the text to extract all the qouted strings
		void addQoutedStringsFromText(const IdString & source)
		{
			std::match_results<typename IdString::const_iterator> m;
			IdString text(source);
			while (std::regex_search(text, m, helpers::getQoutedStringRegex<_IdChar>())) {
				for (auto x : m) {
					if (this->_strings.count(x) == 0) {
						this->_strings.insert(NewStringsPair(x, TextString()));
					}
				}

				text = m.suffix().str();
			}

		}

		void addStringsFromCompilerOutput(const IdString & compilerOutput)
		{
			size_t prefixPosition = 0, postfixPosition = 0;
			IdString prefix = helpers::getPrefixPostfixPair<_IdChar>().first;
			IdString postfix = helpers::getPrefixPostfixPair<_IdChar>().second;
			while ((prefixPosition = compilerOutput.find(prefix, postfixPosition + postfix.size())) != compilerOutput.npos)
			{
				postfixPosition = compilerOutput.find(postfix, prefixPosition);
				IdString newString = compilerOutput.substr(prefixPosition + prefix.size(), postfixPosition - prefixPosition - prefix.size());

				if (this->_strings.count(newString) == 0) {
					this->_strings.insert(NewStringsPair(newString, TextString()));
				}
			}
		}


	protected:

		static const typename Map::value_type & print(const typename Map::value_type & value)
		{
			std::wcout << "\"" << value.first << "\" - \"" << value.second << "\"" << std::endl;
			return value;
		}

		//a strings pair type to insert into the map
		//if the text value is empty it initializes the text with the key value
		class NewStringsPair : public std::pair<IdString, TextString>
		{
		public:
			typedef std::pair<IdString, TextString> Pair;
			NewStringsPair(const IdString & first, const TextString & second) : 
				Pair(first, second.size() == 0 ? helpers::convert<IdString, TextString>(first) : second)
			{}
			NewStringsPair(const Pair & pair) : NewStringsPair(pair.first, pair.second)
			{}
		};
	};
}