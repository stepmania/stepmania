// pkcspad.cpp - written and placed in the public domain by Wei Dai

#include "global.h"
#include "pch.h"
#include "pkcspad.h"
#include <assert.h>

namespace CryptoPP {

template<> const byte PKCS_DigestDecoration<SHA>::decoration[] = {0x30,0x21,0x30,0x09,0x06,0x05,0x2B,0x0E,0x03,0x02,0x1A,0x05,0x00,0x04,0x14};
template<> const unsigned int PKCS_DigestDecoration<SHA>::length = sizeof(PKCS_DigestDecoration<SHA>::decoration);

void PKCS1v15_SignatureMessageEncodingMethod::ComputeMessageRepresentative(RandomNumberGenerator &rng, 
	const byte *recoverableMessage, unsigned int recoverableMessageLength,
	HashTransformation &hash, HashIdentifier hashIdentifier, bool messageEmpty,
	byte *representative, unsigned int representativeBitLength) const
{
	unsigned int digestSize = hash.DigestSize();
	if (digestSize + hashIdentifier.second + 10 > representativeBitLength/8)
		throw PK_Signer::KeyTooShort();

	unsigned int pkcsBlockLen = representativeBitLength;
	// convert from bit length to byte length
	if (pkcsBlockLen % 8 != 0)
	{
		representative[0] = 0;
		representative++;
	}
	pkcsBlockLen /= 8;

	representative[0] = 1;   // block type 1

	byte *pPadding = representative + 1;
	byte *pDigest = representative + pkcsBlockLen - digestSize;
	byte *pHashId = pDigest - hashIdentifier.second;
	byte *pSeparator = pHashId - 1;

	// pad with 0xff
	memset(pPadding, 0xff, pSeparator-pPadding);
	*pSeparator = 0;
	memcpy(pHashId, hashIdentifier.first, hashIdentifier.second);
	hash.Final(pDigest);
}

}
