// algparam.cpp - written and placed in the public domain by Wei Dai

#include "pch.h"
#include "algparam.h"

namespace CryptoPP {

bool (*AssignIntToInteger)(const std::type_info &valueType, void *pInteger, const void *pInt) = NULL;

}
