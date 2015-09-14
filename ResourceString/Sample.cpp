#include <fstream>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>

#define PRINT_RESOURCE_STRINGS
#include "rstring.h"
using namespace rstring;

EditableResource<char, wchar_t> resource;

//_rstr_t SourceCode = _rstr(R"d(
//#include <iostream>
//#include <string>
//#include <regex>
//#include"stringize.hpp" "anotherString"
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
	std::ifstream xmlFs("strings.xml");
	boost::archive::xml_iarchive archive(xmlFs, boost::archive::no_header);
	archive >> boost::serialization::make_nvp("strings", resource);;
	_rstrw_t::setResource(resource);

	auto string1 = _rstrw("simple string without arguments");
	auto string2 = _rstrw("string containing some arguments : int {0}, float {1}, string {2} ", 2, 2.1f, "foo");
	auto string3 = _rstrw("string containing another resource string : {0}", string2);
	std::wcout << string1.str() << std::endl;
	std::wcout << string2.str() << std::endl;
	std::wcout << string3.str() << std::endl;
}

void updateResourceFile() 
{
	//open old resource
	EditableResource<char, wchar_t> oldResource;
	try {
		std::ifstream xmlFs("strings.xml");
		boost::archive::xml_iarchive archive(xmlFs, boost::archive::no_header);
		archive >> boost::serialization::make_nvp("strings", oldResource);
	} catch (std::exception ex) {
		std::cout << "cannot load old resource: " << ex.what() << std::endl;
	}

	//create a resource from build log
	EditableResource<char, wchar_t> newResource;
	std::ifstream buildLogFile(BOOST_PP_STRINGIZE(BUILD_LOG_FILE));
	std::string buildLog((std::istreambuf_iterator<char>(buildLogFile)),
		std::istreambuf_iterator<char>());
	newResource.addStringsFromCompilerOutput(buildLog);

	//merge resources
	oldResource.merge(newResource);

	//save updated resource
	{
		std::ofstream xmlFs("strings.xml");
		boost::archive::xml_oarchive archive(xmlFs, boost::archive::no_header);
		archive << boost::serialization::make_nvp("strings", oldResource);
	}
}

int main()
{
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