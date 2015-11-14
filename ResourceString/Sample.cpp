#include <fstream>
#include <boost/archive/xml_woarchive.hpp>
#include <boost/archive/xml_wiarchive.hpp>
#include <boost/archive/text_woarchive.hpp>
#include <boost/archive/text_wiarchive.hpp>
#include <boost/log/trivial.hpp>

#define PRINT_RESOURCE_STRINGS
//to prevent serialization of std::wstring, uncomment the macro definition below
//in this case std::wstring arguments would be converted to std::string
//#define RESOURCE_STRING_DISABLE_WSTRING_ARGUMENTS

#include "rstring.h"
using namespace rstring;

//multiline strings are not recommended
//unfortunately VS build log contains extra formatting spaces on the left side
//boost xml archive also adds extra spaces to multiline strings
//so if you do not want to face extra spaces after each line ending when using multiline strings
//delete the spaces  manually from the resource

//_rstr_t SourceCode = _rstr(R"d(
//#include <iostream>
//#include <string>
//#include <regex>
//#include"stringize.hpp"
//int main()
//{
//	std::string lines[] = {USE_STRING_CONSTANT("Roses are #ff0000"),
//		USE_STRING_CONSTANT("(violets are
//							#0000ff)"),
//							"all of my base are belong to you"
//};
//}
//)d");


void usageSample()
{
	//first we load a resource from an xml file
	{
		std::wifstream xmlFs("strings.xml");
		boost::archive::xml_wiarchive archive(xmlFs, boost::archive::no_header);
		archive >> boost::serialization::make_nvp("strings", _rstrw_t::resource());
	}

	//_rstrw_t is a type with hidden constructors and public static 'Construct' methods
	//so the only correct way to create a string is using macro _rstrw
	//it would print the string at the compile time if PRINT_RESOURCE_STRINGS is defined
	auto string1 = _rstrw("simple string without arguments");
	auto string2 = _rstrw("string containing some arguments : int {0}, float {1}, string {2}", 2, 2.1f, std::string("foo"));
	auto string3 = _rstrw(R"(string containing another resource string : "{0}")", string2);

	//to get strings translated we call the str() method
	BOOST_LOG_TRIVIAL(trace) << _rstrw("strings before serialization: \n {0} \n{1} \n{2}\n\n", string1, string2, string3).str();
	
	//now we can serialize the strings. xml and binary archives are also available
	std::wstringstream ss;
	boost::archive::text_woarchive outputArchive(ss);
	outputArchive << string1;
	outputArchive << string2;
	outputArchive << string3;

	//the format string constant will be replaced with it's resource id
	//the arguments will be converted to strings
	BOOST_LOG_TRIVIAL(trace) << _rstrw("serialized strings:\n{0}\n", ss.str()).str();

	//now we can switch the language
	auto resourceBackup = _rstrw_t::resource();
	try {
		std::wifstream xmlFs("translated_strings.xml");
		boost::archive::xml_wiarchive archive(xmlFs, boost::archive::no_header);
		archive >> boost::serialization::make_nvp("strings", _rstrw_t::resource());
	} catch (const std::exception & e) {
		BOOST_LOG_TRIVIAL(trace) << _rstrw("cannot change the resource file ({0}), lets continue with the same language\n", e.what()).str();
		_rstrw_t::resource() = resourceBackup;
	}

	//deserialization
	boost::archive::text_wiarchive inputArcchive(ss);
	_rstrw_t string4, string5, string6;
	inputArcchive >> string4;
	inputArcchive >> string5;
	inputArcchive >> string6;

	//okey, here are the deserialized strings 
	BOOST_LOG_TRIVIAL(trace) << _rstrw("strings after deserialization: \n {0} \n{1} \n{2}\n", string4, string5, string6).str();
}

void updateResourceFile() 
{
	//open old resource
	EditableResource<wchar_t, wchar_t> oldResource;
	try {
		std::wifstream xmlFs("strings.xml");
		boost::archive::xml_wiarchive archive(xmlFs, boost::archive::no_header);
		archive >> boost::serialization::make_nvp("strings", oldResource);
	} catch (std::exception ex) {
		//in this method we cannot use resource strings because the resorce is not ready
		//so using a raw string constant
		std::cout << "cannot load old resource: " << ex.what() << std::endl;
	}

	//create a resource using the build log
	EditableResource<wchar_t, wchar_t> newResource;
	std::wifstream buildLogFile(BOOST_PP_STRINGIZE(BUILD_LOG_FILE));
	std::wstring buildLog((std::istreambuf_iterator<wchar_t>(buildLogFile)),
		std::istreambuf_iterator<wchar_t>());
	newResource.addStringsFromCompilerOutput(buildLog);

	//merge resources
	oldResource.update(newResource);
	oldResource.merge(newResource);
	oldResource.printOrphanedStrings(newResource);

	//suggest to save the updated resource
	std::cout << "would you like to save the resource(y/n)?" << std::endl;
	char c;
	std::cin >> c;
	if (c == 'y')
	{
		std::cout << std::endl << "saving the archive..." << std::endl;
		std::wofstream xmlFs("strings.xml");
		boost::archive::xml_woarchive archive(xmlFs, boost::archive::no_header);
		archive << boost::serialization::make_nvp("strings", oldResource);
	}
}

int main()
{
#ifdef _MSC_VER
	system("chcp 65001");
#endif
	boost::locale::generator gen;
	std::locale::global( gen.generate(std::locale(), ""));
	std::wcout.imbue(std::locale());

	std::cout << R"(1 - update strings resource
2 - run usage sample
)";
	int choise;
	std::cin >> choise;
	if (choise == 1) {
		updateResourceFile();
	}
	if (choise == 2) {
		usageSample();
	}

	system("Pause");
}