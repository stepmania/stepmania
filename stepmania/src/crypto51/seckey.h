// seckey.h - written and placed in the public domain by Wei Dai

// This file contains helper classes/functions for implementing secret key algorithms.

#ifndef CRYPTOPP_SECKEY_H
#define CRYPTOPP_SECKEY_H

#include "cryptlib.h"
#include "misc.h"
#include "simple.h"

namespace CryptoPP {

//! .
template <unsigned int N>
class FixedBlockSize
{
public:
	enum {BLOCKSIZE = N};
};

// ************** key length ***************

//! .
template <unsigned int N, unsigned int IV_REQ = SimpleKeyingInterface::NOT_RESYNCHRONIZABLE>
class FixedKeyLength
{
public:
	enum {KEYLENGTH=N, MIN_KEYLENGTH=N, MAX_KEYLENGTH=N, DEFAULT_KEYLENGTH=N};
	enum {IV_REQUIREMENT = IV_REQ};
	static unsigned int StaticGetValidKeyLength(unsigned int) {return KEYLENGTH;}
};

// ************** implementation helper for SimpledKeyed ***************

template <class T>
static inline void CheckedSetKey(T *obj, Empty empty, const byte *key, unsigned int length, const NameValuePairs &param)
{
	obj->ThrowIfInvalidKeyLength(length);
	obj->UncheckedSetKey(key, length);
}

template <class T>
static inline void CheckedSetKey(T *obj, CipherDir dir, const byte *key, unsigned int length, const NameValuePairs &param)
{
	obj->ThrowIfInvalidKeyLength(length);
	obj->UncheckedSetKey(dir, key, length);
}

//! .
template <class BASE, class INFO = BASE>
class SimpleKeyingInterfaceImpl : public BASE
{
public:
	unsigned int MinKeyLength() const {return INFO::MIN_KEYLENGTH;}
	unsigned int MaxKeyLength() const {return (unsigned int)INFO::MAX_KEYLENGTH;}
	unsigned int DefaultKeyLength() const {return INFO::DEFAULT_KEYLENGTH;}
	unsigned int GetValidKeyLength(unsigned int n) const {return INFO::StaticGetValidKeyLength(n);}
	typename BASE::IV_Requirement IVRequirement() const {return (typename BASE::IV_Requirement)INFO::IV_REQUIREMENT;}

protected:
	void AssertValidKeyLength(unsigned int length) {assert(GetValidKeyLength(length) == length);}
};

template <class INFO, class INTERFACE = BlockCipher>
class BlockCipherBaseTemplate : public AlgorithmImpl<SimpleKeyingInterfaceImpl<TwoBases<INFO, INTERFACE> > >
{
public:
	unsigned int BlockSize() const {return this->BLOCKSIZE;}
};

//! .
template <CipherDir DIR, class BASE>
class BlockCipherTemplate : public BASE
{
public:
 	BlockCipherTemplate() {}
	BlockCipherTemplate(const byte *key)
		{SetKey(key, this->DEFAULT_KEYLENGTH);}
	BlockCipherTemplate(const byte *key, unsigned int length)
		{SetKey(key, length);}
	BlockCipherTemplate(const byte *key, unsigned int length, unsigned int rounds)
		{this->SetKeyWithRounds(key, length, rounds);}

	bool IsForwardTransformation() const {return DIR == ENCRYPTION;}

	void SetKey(const byte *key, unsigned int length, const NameValuePairs &param = g_nullNameValuePairs)
	{
		CheckedSetKey(this, DIR, key, length, param);
	}

	Clonable * Clone() const {return new BlockCipherTemplate<DIR, BASE>(*this);}
};

// ************** documentation ***************

/*! \brief Each class derived from this one defines two types, Encryption and Decryption, 
	both of which implement the SymmetricCipher interface. See CipherModeDocumentation
	for information about using block ciphers. */
struct SymmetricCipherDocumentation
{
	//! implements the SymmetricCipher interface
	typedef SymmetricCipher Encryption;
	//! implements the SymmetricCipher interface
	typedef SymmetricCipher Decryption;
};

}

#endif
