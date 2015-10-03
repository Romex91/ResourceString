#pragma once
#include <regex>
#include <type_traits>
#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/stringize.hpp>
#include <boost/preprocessor/wstringize.hpp>
#include "Defines.h"


namespace rstring
{
	template<class _IdChar, class _TextChar>
	class EditableResource : public Resource < _IdChar, _TextChar >
	{
	public:
		//run merging console dialog
		//looks for strings with an identical key and different texts and suggest user to choose which one to save  
		void merge(const EditableResource<_IdChar, _TextChar> & resource)
		{
			//suggest what to do about conflicting strings
			std::cout << "Conflicting strings:" << std::endl;
			for (auto & rightStringsPair : resource._strings) {
				Map::const_iterator leftStringsPair;
				if ((leftStringsPair = find(rightStringsPair.first)) != _strings.end()) {
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
							_strings.erase(leftStringsPair);
							_strings.insert(NewStringsPair(rightStringsPair));
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
				if (leftStringPair != _strings.end()) {
					//check if the currently persistent pair has an empty text value
					//and we have a text on the right side
					if (leftStringPair->second == TextString() && rightStringPair.second != TextString()) {
						Resource::_strings.erase(leftStringPair);
					}
					else {
						continue;
					}
				}
				_strings.insert(print(NewStringsPair(rightStringPair)));
			}
		}

		//print the strings which exist in this resource but not in the resource passed as an argument
		void printOrphanedStrings(const EditableResource<_IdChar, _TextChar> & resource)
		{
			std::cout << "Orphaned strings:" << std::endl;
			for (auto leftStringsPair : _strings) {
				if (resource.find(leftStringsPair.first) == resource._strings.end()) {
					print(leftStringsPair);
				}
			}
		}

		//parses the text to extract all the qouted strings
		void addQoutedStringsFromText(const IdString & text)
		{
			std::match_results<IdString::const_iterator> m;
			IdString text(source);
			while (std::regex_search(text, m, getQoutedStringRegex<_IdChar>())) {
				for (auto x : m) {
					if (_strings.count(x) == 0) {
						_strings.insert(NewStringsPair(x, TextString()));
					}
				}

				text = m.suffix().str();
			}

		}

		void addStringsFromCompilerOutput(const IdString & compilerOutput)
		{
			size_t prefixPosition = 0, postfixPosition = 0;
			IdString prefix = getPrefixPostfixPair<_IdChar>().first;
			IdString postfix = getPrefixPostfixPair<_IdChar>().second;
			while ((prefixPosition = compilerOutput.find(prefix, postfixPosition + postfix.size())) != compilerOutput.npos)
			{
				postfixPosition = compilerOutput.find(postfix, prefixPosition);
				IdString newString = compilerOutput.substr(prefixPosition + prefix.size(), postfixPosition - prefixPosition - prefix.size());

				if (_strings.count(newString) == 0) {
					_strings.insert(NewStringsPair(newString, TextString()));
				}
			}
		}


	protected:

		//////////////////////////////////////////////////////////////////////////
		//type-specific helpers
		//////////////////////////////////////////////////////////////////////////
		template<class _Char>
		static std::basic_regex<_Char> getQoutedStringRegex();

		template<>
		static std::basic_regex<char> getQoutedStringRegex<char>()
		{
			return std::regex(R"d("(?:[^"\\]|\\.|\\\n|\\\r\n)*")d");
		}

		template<>
		static std::basic_regex<wchar_t> getQoutedStringRegex<wchar_t>()
		{
			return std::wregex(LR"d("(?:[^"\\]|\\.|\\\n|\\\r\n)*")d");
		}

		template<class _Char>
		static std::pair<std::basic_string<_Char>, std::basic_string<_Char>> getPrefixPostfixPair();

		template<>
		static std::pair<std::basic_string<char>, std::basic_string<char>> getPrefixPostfixPair<char>()
		{
			return std::pair<std::basic_string<char>, std::basic_string<char>>(BOOST_PP_STRINGIZE(RESOURCE_STRING_PREFIX), BOOST_PP_STRINGIZE(RESOURCE_STRING_POSTFIX));
		}

		template<>
		static std::pair<std::basic_string<wchar_t>, std::basic_string<wchar_t>> getPrefixPostfixPair<wchar_t>()
		{
			return std::pair<std::basic_string<wchar_t>, std::basic_string<wchar_t>>(BOOST_PP_WSTRINGIZE(RESOURCE_STRING_PREFIX), BOOST_PP_WSTRINGIZE(RESOURCE_STRING_POSTFIX));
		}

		static typename const Map::value_type & print(typename const Map::value_type & value)
		{
			std::wcout << "\"" << value.first << "\" - \"" << value.second << "\"" << std::endl;
			return value;
		}

		class DuplicatesPair : public std::pair<IdString, IdString>
		{
		public:
			typedef std::pair<IdString, IdString> Pair;
			DuplicatesPair(const IdString & first,
				const IdString & second) : Pair(first, second.size() == 0 ? first : second)
			{}
			DuplicatesPair(const Pair & pair) : DuplicatesPair(pair.first, pair.second)
			{}
		};

		//a strings pair type to insert into the map
		//if key and text types are the same it initializes the text with the key value
		typedef typename std::conditional<std::is_same<_IdChar, _TextChar>::value,
			DuplicatesPair, std::pair<IdString, TextString>>::type NewStringsPair;
	};
}