#pragma once
#include <regex>
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
		void merge(const EditableResource<_IdChar, _TextChar> & resource)
		{
			//first copy the strings which do not exist in this resource
			std::cout << "New strings:" << std::endl;
			std::copy_if(resource._strings.begin(), resource._strings.end(), std::inserter(_strings, _strings.end()),
				[this](const Map::value_type & value) {
				if (_strings.count(value.first) != 0)
					if (_strings[value.first] != TextString())
						return false;
				print(value);
				return true;
			});

			//then suggest what to do about conflicting strings
			std::cout << "Conflicting strings:" << std::endl;
			std::copy_if(resource._strings.begin(), resource._strings.end(), std::inserter(_strings, _strings.end()),
				[this](const Map::value_type & value) {
				Map::iterator foundString;
				if ((foundString = _strings.find(value.first)) != _strings.end()) {
					if (foundString->second != value.second && value.second != TextString()) {
						const int saveLeft = 1, saveRight = 2;
						int choise = 0;
						while (choise != saveLeft && choise != saveRight)
						{
							std::cout << "select the strings pair to save in the resource:" << std::endl << saveLeft;
							print(*foundString);
							print(value);
							std::cin >> choise;
						}

						if (choise == saveRight)
						{
							return true;
						}
					}
				}
				return false;
			});

			//print the strings which exist in this resource but not in the right one
			std::cout << "Orphaned strings:" << std::endl;
			for (auto stringsPair : _strings) {
				if (resource._strings.count(stringsPair.first)==0) {
					print(stringsPair);
				}
			}
		}

		void addQoutedStringsFromText(const IdString & text)
		{
			std::match_results<IdString::const_iterator> m;
			IdString text(source);
			while (std::regex_search(text, m, getQoutedStringRegex<_IdChar>())) {
				for (auto x : m) {
					if (_strings.count(x) == 0) {
						_strings[x] = TextString();
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
					_strings[newString] = TextString();
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

		static void print(std::string string)
		{
			std::cout << string;
		}

		static void print(std::wstring string)
		{
			std::wcout << string;
		}

		static void print(typename Map::value_type value)
		{
			std::cout << "\"";
			print(value.first);
			std::cout << "\" - \"";
			print(value.second);
			std::cout << "\"" << std::endl;
		}
	};
}