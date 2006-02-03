#ifndef CRYPTOPP_SHA_H
#define CRYPTOPP_SHA_H

#include "iterhash.h"

namespace CryptoPP {

/// <a href="http://www.weidai.com/scan-mirror/md.html#SHA-1">SHA-1</a>
class SHA : public IteratedHashWithStaticTransform<word32, BigEndian, 64, SHA>
{
public:
	enum {DIGESTSIZE = 20};
	SHA() : IteratedHashWithStaticTransform<word32, BigEndian, 64, SHA>(DIGESTSIZE) {Init();}
	static void Transform(word32 *digest, const word32 *data);
	static const char *StaticAlgorithmName() {return "SHA-1";}

protected:
	void Init();
};

typedef SHA SHA1;

}

#endif
