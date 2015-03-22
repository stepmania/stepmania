#ifndef STEPMANIA_VER_H
#define STEPMANIA_VER_H

extern const char *const product_version;

// XXX: These names are misnomers. This is actually the BUILD number, time and date.
// NOTE: In GNU toolchain these are defined in ver.cpp. In MSVC these are defined in archutils/Win32/verinc.c I think. Why? I don't know and I don't have MSVC. --root
extern const unsigned long version_num;
extern const char *const version_time;
extern const char *const version_date;

#endif