/*
 * This program was placed in the public domain by Avery,
 * with some ideas implemented by AJ. Any C++ improvements
 * are made by others.
 */

#include <ctime>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include "version.hpp"

int main(int argc, char *argv[]) {
  if (argc < 2) {
    return 1;
  }

  std::ifstream versionReadFile("version.bin", std::ios::in);

  unsigned long build = 0;

  // try to read the last version seen.
  if (versionReadFile.is_open()) {
    std::string line;
    std::getline(versionReadFile, line);
    if (!line.empty()) {
      // TODO: Use stoul when C++11 is utilized.
      build = std::atol(line.c_str());
    }

    versionReadFile.close();
  }

  // increment the build number and write it.
  ++build;

  std::ofstream versionWriteFile("version.bin", std::ios::out);

  if (versionWriteFile.is_open()) {
    versionWriteFile << build;
    versionWriteFile.close();
  }

  char strdate[10];
  char strtime[64];
  time_t tm;

  std::time(&tm);

  std::strftime(strdate, 15, "%Y%m%d", std::localtime(&tm));
  std::strftime(strtime, 64, "%H:%M:%S %Z", std::localtime(&tm));

  // zero out the newline.
  strtime[sizeof(strtime) - 1] = 0;

  std::ofstream versionDataFile(argv[1], std::ios::out);
  if (versionDataFile.is_open()) {
    versionDataFile << "extern unsigned long const version_num = " << build << ";" << std::endl;
    versionDataFile << "extern int const sm_version_major = " << smVersionMajor << ";" << std::endl;
    versionDataFile << "extern int const sm_version_minor = " << smVersionMinor << ";" << std::endl;
    versionDataFile << "extern int const sm_version_patch = " << smVersionPatch << ";" << std::endl;
    versionDataFile << "extern char const * const sm_version_git_hash = \"" << smVersionGitHash << "\";" << std::endl;
    versionDataFile << "extern char const * const sm_version_traditional = \"" << smVersionTraditional << "\";" << std::endl;
    versionDataFile << "extern char const * const version_date = \"" << strdate << "\";" << std::endl;
    versionDataFile << "extern char const * const version_time = \"" << strtime << "\";" << std::endl;
    versionDataFile << "extern char const * const product_version = \"" << smVersionFull << "\";" << std::endl;
    versionDataFile.close();
  }

  return 0;
}
