// Ehhh....
//
// I think I'll just put this one in the public domain
// (with no warranty as usual).
//
// --Avery

// took some ideas from OpenITG... -aj

#include <ctime>

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

int main(int argc, char *argv[])
{
	if (argc < 2) {
		return 1;
	}

	std::ifstream versionReadFile("version.bin", std::ios::in );

	size_t uLongSize = sizeof(unsigned long);
	unsigned long build = 0;

	// try to read the last version seen
	if (versionReadFile.is_open()) {
		std::string line;
		std::getline(versionReadFile, line);
		if (!line.empty()) {
			// TODO: When we switch to c++11, use stoul instead.
			build = std::atol(line.c_str());
		}

		versionReadFile.close();
	}
	
	// increment the build number and write it
	++build;

	std::ofstream versionWriteFile("version.bin", std::ios::out );

	if (versionWriteFile.is_open()) {
		versionWriteFile << build;
		versionWriteFile.close();
	}
	
	char strdate[10];
	char strtime[64];
	time_t tm;

	// get the current time
	std::time(&tm);

	// print the debug serial date/time
	std::strftime( strdate, 15, "%Y%m%d", std::localtime(&tm) );
	std::strftime( strtime, 64, "%H:%M:%S %Z", std::localtime(&tm) );

	// zero out the newline character
	strtime[sizeof(strtime)-1] = 0;

	// write to verstub
	std::ofstream versionDataFile(argv[1], std::ios::out);

	if (versionDataFile.is_open()) {
		versionDataFile << "unsigned long version_num = " << build << ";" << std::endl;
		versionDataFile << "extern const char *const version_date = \"" << strdate << "\";" << std::endl;
		versionDataFile << "extern const char *const version_time = \"" << strtime << "\";" << std::endl;
		versionDataFile.close();
	}

	return 0;
}