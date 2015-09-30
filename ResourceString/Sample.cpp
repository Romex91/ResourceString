#include <fstream>
#include <boost/archive/xml_woarchive.hpp>
#include <boost/archive/xml_wiarchive.hpp>
#include <boost/archive/text_woarchive.hpp>
#include <boost/archive/text_wiarchive.hpp>

#define PRINT_RESOURCE_STRINGS
#include "rstring.h"
using namespace rstring;

EditableResource<char, wchar_t> resource;

//unfortunately VS build log contains extra formatting spaces on the left side
//do not use multi-lines strings until you are going to manually delete the spaces from the log file
_rstr_t SourceCode = _rstr(R"d(
#include <iostream>
#include <string>
#include <regex>
#include"stringize.hpp"
int main()
{
	std::string lines[] = {USE_STRING_CONSTANT("Roses are #ff0000"),
		USE_STRING_CONSTANT("(violets are
							#0000ff)"),
							"all of my base are belong to you"
};
}
)d");


void usageSample()
{
	std::wifstream xmlFs("strings.xml");
	boost::archive::xml_wiarchive archive(xmlFs, boost::archive::no_header);
	archive >> boost::serialization::make_nvp("strings", resource);;
	_rstrw_t::setResource(resource);

	std::cout << "strings before serialization:" << std::endl;
	auto string1 = _rstrw("simple string without arguments");
	auto string2 = _rstrw("string containing some arguments : int {0}, float {1}, string {2}", 2, 2.1f, "foo");
	auto string3 = _rstrw(R"(string containing another resource string : "{0}")", string2);


	std::wcout << string1.str() << std::endl;
	std::wcout << string2.str() << std::endl;
	std::wcout << string3.str() << std::endl;
	
	std::wstringstream ss;
	auto outputArchive = boost::archive::text_woarchive(ss);
	outputArchive << string1;
	outputArchive << string2;
	outputArchive << string3;

	std::wcout << "serialized strings:" << std::endl
		<< ss.str() << std::endl;

	auto inputArcchive = boost::archive::text_wiarchive(ss);
	_rstrw_t string4, string5, string6;
	inputArcchive >> string4;
	inputArcchive >> string5;
	inputArcchive >> string6;

	std::cout << "strings after serialization:" << std::endl;
	std::wcout << string4.str() << std::endl;
	std::wcout << string5.str() << std::endl;
	std::wcout << string6.str() << std::endl;

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
		std::cout << "cannot load old resource: " << ex.what() << std::endl;
	}

	//create a resource from build log
	EditableResource<wchar_t, wchar_t> newResource;
	std::wifstream buildLogFile(BOOST_PP_STRINGIZE(BUILD_LOG_FILE));
	std::wstring buildLog((std::istreambuf_iterator<wchar_t>(buildLogFile)),
		std::istreambuf_iterator<wchar_t>());
	newResource.addStringsFromCompilerOutput(buildLog);

	//merge resources
	oldResource.merge(newResource);

	//save updated resource
	{
		std::wofstream xmlFs("strings.xml");
		boost::archive::xml_woarchive archive(xmlFs, boost::archive::no_header);
		archive << boost::serialization::make_nvp("strings", oldResource);
	}
}

int main()
{
	std::wcout << "User-preferred locale setting is " << std::locale("").name().c_str() << '\n';
	std::locale::global(std::locale(""));
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