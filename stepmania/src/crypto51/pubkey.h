// pubkey.h - written and placed in the public domain by Wei Dai

#ifndef CRYPTOPP_PUBKEY_H
#define CRYPTOPP_PUBKEY_H

/** \file

	This file contains helper classes/functions for implementing public key algorithms.

	The class hierachies in this .h file tend to look like this:
<pre>
                  x1
                 / \
                y1  z1
                 |  |
            x2<y1>  x2<z1>
                 |  |
                y2  z2
                 |  |
            x3<y2>  x3<z2>
                 |  |
                y3  z3
</pre>
	- x1, y1, z1 are abstract interface classes defined in cryptlib.h
	- x2, y2, z2 are implementations of the interfaces using "abstract policies", which
	  are pure virtual functions that should return interfaces to interchangeable algorithms.
	  These classes have "Base" suffixes.
	- x3, y3, z3 hold actual algorithms and implement those virtual functions.
	  These classes have "Impl" suffixes.

	The "TF_" prefix means an implementation using trapdoor functions on integers.
	The "DL_" prefix means an implementation using group operations (in groups where discrete log is hard).
*/

#include "integer.h"
#include "filters.h"
#include "argnames.h"
#include <memory>

#include "modarith.h"

// VC60 workaround: this macro is defined in shlobj.h and conflicts with a template parameter used in this file
#undef INTERFACE

NAMESPACE_BEGIN(CryptoPP)

Integer NR_EncodeDigest(unsigned int modulusBits, const byte *digest, unsigned int digestLen);
Integer DSA_EncodeDigest(unsigned int modulusBits, const byte *digest, unsigned int digestLen);

// ********************************************************

//! .
class TrapdoorFunctionBounds
{
public:
	virtual ~TrapdoorFunctionBounds() {}

	virtual Integer PreimageBound() const =0;
	virtual Integer ImageBound() const =0;
	virtual Integer MaxPreimage() const {return --PreimageBound();}
	virtual Integer MaxImage() const {return --ImageBound();}
};

//! .
class RandomizedTrapdoorFunction : public TrapdoorFunctionBounds
{
public:
	virtual Integer ApplyRandomizedFunction(RandomNumberGenerator &rng, const Integer &x) const =0;
	virtual bool IsRandomized() const {return true;}
};

//! .
class TrapdoorFunction : public RandomizedTrapdoorFunction
{
public:
	Integer ApplyRandomizedFunction(RandomNumberGenerator &rng, const Integer &x) const
		{return ApplyFunction(x);}
	bool IsRandomized() const {return false;}

	virtual Integer ApplyFunction(const Integer &x) const =0;
};

//! .
class RandomizedTrapdoorFunctionInverse
{
public:
	virtual ~RandomizedTrapdoorFunctionInverse() {}

	virtual Integer CalculateRandomizedInverse(RandomNumberGenerator &rng, const Integer &x) const =0;
	virtual bool IsRandomized() const {return true;}
};

//! .
class TrapdoorFunctionInverse : public RandomizedTrapdoorFunctionInverse
{
public:
	virtual ~TrapdoorFunctionInverse() {}

	Integer CalculateRandomizedInverse(RandomNumberGenerator &rng, const Integer &x) const
		{return CalculateInverse(rng, x);}
	bool IsRandomized() const {return false;}

	virtual Integer CalculateInverse(RandomNumberGenerator &rng, const Integer &x) const =0;
};

// ********************************************************

//! .
template <class TFI, class MEI>
class TF_Base
{
public:
	virtual ~TF_Base() {}
protected:
	virtual const TrapdoorFunctionBounds & GetTrapdoorFunctionBounds() const =0;

	typedef TFI TrapdoorFunctionInterface;
	virtual const TrapdoorFunctionInterface & GetTrapdoorFunctionInterface() const =0;

	typedef MEI MessageEncodingInterface;
	virtual const MessageEncodingInterface & GetMessageEncodingInterface() const =0;
};

// ********************************************************

typedef std::pair<const byte *, unsigned int> HashIdentifier;

//! .
class PK_SignatureMessageEncodingMethod
{
public:
	virtual ~PK_SignatureMessageEncodingMethod() {}

	virtual unsigned int MaxRecoverableLength(unsigned int representativeBitLength, unsigned int hashIdentifierLength, unsigned int digestLength) const
		{return 0;}

	bool IsProbabilistic() const 
		{return true;}
	bool AllowNonrecoverablePart() const
		{throw NotImplemented("PK_MessageEncodingMethod: this signature scheme does not support message recovery");}
	virtual bool RecoverablePartFirst() const
		{throw NotImplemented("PK_MessageEncodingMethod: this signature scheme does not support message recovery");}

	// for verification, DL
	virtual void ProcessSemisignature(HashTransformation &hash, const byte *semisignature, unsigned int semisignatureLength) const {}

	// for signature
	virtual void ProcessRecoverableMessage(HashTransformation &hash, 
		const byte *recoverableMessage, unsigned int recoverableMessageLength, 
		const byte *presignature, unsigned int presignatureLength,
		SecByteBlock &semisignature) const
	{
		if (RecoverablePartFirst())
			assert(!"ProcessRecoverableMessage() not implemented");
	}

	virtual void ComputeMessageRepresentative(RandomNumberGenerator &rng, 
		const byte *recoverableMessage, unsigned int recoverableMessageLength,
		HashTransformation &hash, HashIdentifier hashIdentifier, bool messageEmpty,
		byte *representative, unsigned int representativeBitLength) const =0;

	virtual bool VerifyMessageRepresentative(
		HashTransformation &hash, HashIdentifier hashIdentifier, bool messageEmpty,
		byte *representative, unsigned int representativeBitLength) const =0;

	virtual DecodingResult RecoverMessageFromRepresentative(	// for TF
		HashTransformation &hash, HashIdentifier hashIdentifier, bool messageEmpty,
		byte *representative, unsigned int representativeBitLength,
		byte *recoveredMessage) const
		{throw NotImplemented("PK_MessageEncodingMethod: this signature scheme does not support message recovery");}

	virtual DecodingResult RecoverMessageFromSemisignature(		// for DL
		HashTransformation &hash, HashIdentifier hashIdentifier,
		const byte *presignature, unsigned int presignatureLength,
		const byte *semisignature, unsigned int semisignatureLength,
		byte *recoveredMessage) const
		{throw NotImplemented("PK_MessageEncodingMethod: this signature scheme does not support message recovery");}

	// VC60 workaround
	struct HashIdentifierLookup
	{
		template <class H> struct HashIdentifierLookup2
		{
			static HashIdentifier Lookup()
			{
				return HashIdentifier(NULL, 0);
			}
		};
	};
};

class PK_DeterministicSignatureMessageEncodingMethod : public PK_SignatureMessageEncodingMethod
{
public:
	bool VerifyMessageRepresentative(
		HashTransformation &hash, HashIdentifier hashIdentifier, bool messageEmpty,
		byte *representative, unsigned int representativeBitLength) const;
};

class PK_RecoverableSignatureMessageEncodingMethod : public PK_SignatureMessageEncodingMethod
{
public:
	bool VerifyMessageRepresentative(
		HashTransformation &hash, HashIdentifier hashIdentifier, bool messageEmpty,
		byte *representative, unsigned int representativeBitLength) const;
};

class DL_SignatureMessageEncodingMethod_DSA : public PK_DeterministicSignatureMessageEncodingMethod
{
public:
	void ComputeMessageRepresentative(RandomNumberGenerator &rng, 
		const byte *recoverableMessage, unsigned int recoverableMessageLength,
		HashTransformation &hash, HashIdentifier hashIdentifier, bool messageEmpty,
		byte *representative, unsigned int representativeBitLength) const;
};

class DL_SignatureMessageEncodingMethod_NR : public PK_DeterministicSignatureMessageEncodingMethod
{
public:
	void ComputeMessageRepresentative(RandomNumberGenerator &rng, 
		const byte *recoverableMessage, unsigned int recoverableMessageLength,
		HashTransformation &hash, HashIdentifier hashIdentifier, bool messageEmpty,
		byte *representative, unsigned int representativeBitLength) const;
};

class PK_MessageAccumulatorBase : public PK_MessageAccumulator
{
public:
	PK_MessageAccumulatorBase() : m_empty(true) {}

	virtual HashTransformation & AccessHash() =0;

	void Update(const byte *input, unsigned int length)
	{
		AccessHash().Update(input, length);
		m_empty = m_empty && length == 0;
	}

	SecByteBlock m_recoverableMessage, m_representative, m_presignature, m_semisignature;
	Integer m_k, m_s;
	bool m_empty;
};

template <class HASH_ALGORITHM>
class PK_MessageAccumulatorImpl : public PK_MessageAccumulatorBase, protected ObjectHolder<HASH_ALGORITHM>
{
public:
	HashTransformation & AccessHash() {return this->m_object;}
};

//! .
template <class INTERFACE, class BASE>
class TF_SignatureSchemeBase : public INTERFACE, protected BASE
{
public:
	unsigned int SignatureLength() const 
		{return this->GetTrapdoorFunctionBounds().MaxPreimage().ByteCount();}
	unsigned int MaxRecoverableLength() const 
		{return this->GetMessageEncodingInterface().MaxRecoverableLength(MessageRepresentativeBitLength(), GetHashIdentifier().second, GetDigestSize());}
	unsigned int MaxRecoverableLengthFromSignatureLength(unsigned int signatureLength) const
		{return this->MaxRecoverableLength();}

	bool IsProbabilistic() const 
		{return this->GetTrapdoorFunctionInterface().IsRandomized() || this->GetMessageEncodingInterface().IsProbabilistic();}
	bool AllowNonrecoverablePart() const 
		{return this->GetMessageEncodingInterface().AllowNonrecoverablePart();}
	bool RecoverablePartFirst() const 
		{return this->GetMessageEncodingInterface().RecoverablePartFirst();}

protected:
	unsigned int MessageRepresentativeLength() const {return BitsToBytes(MessageRepresentativeBitLength());}
	unsigned int MessageRepresentativeBitLength() const {return this->GetTrapdoorFunctionBounds().ImageBound().BitCount()-1;}
	virtual HashIdentifier GetHashIdentifier() const =0;
	virtual unsigned int GetDigestSize() const =0;
};

//! .
class TF_SignerBase : public TF_SignatureSchemeBase<PK_Signer, TF_Base<RandomizedTrapdoorFunctionInverse, PK_SignatureMessageEncodingMethod> >
{
public:
	void InputRecoverableMessage(PK_MessageAccumulator &messageAccumulator, const byte *recoverableMessage, unsigned int recoverableMessageLength) const;
	unsigned int SignAndRestart(RandomNumberGenerator &rng, PK_MessageAccumulator &messageAccumulator, byte *signature, bool restart=true) const;
};

//! .
class TF_VerifierBase : public TF_SignatureSchemeBase<PK_Verifier, TF_Base<TrapdoorFunction, PK_SignatureMessageEncodingMethod> >
{
public:
	void InputSignature(PK_MessageAccumulator &messageAccumulator, const byte *signature, unsigned int signatureLength) const;
	bool VerifyAndRestart(PK_MessageAccumulator &messageAccumulator) const;
	DecodingResult RecoverAndRestart(byte *recoveredMessage, PK_MessageAccumulator &recoveryAccumulator) const;
};

// ********************************************************

//! .
template <class T1, class T2, class T3>
struct TF_CryptoSchemeOptions
{
	typedef T1 AlgorithmInfo;
	typedef T2 Keys;
	typedef typename Keys::PrivateKey PrivateKey;
	typedef typename Keys::PublicKey PublicKey;
	typedef T3 MessageEncodingMethod;
};

//! .
template <class T1, class T2, class T3, class T4>
struct TF_SignatureSchemeOptions : public TF_CryptoSchemeOptions<T1, T2, T3>
{
	typedef T4 HashFunction;
};

//! .
template <class KEYS>
class PublicKeyCopier
{
public:
	virtual ~PublicKeyCopier() {}
	virtual void CopyKeyInto(typename KEYS::PublicKey &key) const =0;
};

//! .
template <class KEYS>
class PrivateKeyCopier
{
public:
	virtual ~PrivateKeyCopier() {}
	virtual void CopyKeyInto(typename KEYS::PublicKey &key) const =0;
	virtual void CopyKeyInto(typename KEYS::PrivateKey &key) const =0;
};

//! .
template <class BASE, class SCHEME_OPTIONS, class KEY>
class TF_ObjectImplBase : public AlgorithmImpl<BASE, typename SCHEME_OPTIONS::AlgorithmInfo>
{
public:
	typedef SCHEME_OPTIONS SchemeOptions;
	typedef KEY KeyClass;

	PublicKey & AccessPublicKey() {return AccessKey();}
	const PublicKey & GetPublicKey() const {return GetKey();}

	PrivateKey & AccessPrivateKey() {return AccessKey();}
	const PrivateKey & GetPrivateKey() const {return GetKey();}

	virtual const KeyClass & GetKey() const =0;
	virtual KeyClass & AccessKey() =0;

	const KeyClass & GetTrapdoorFunction() const {return GetKey();}

protected:
	const typename BASE::MessageEncodingInterface & GetMessageEncodingInterface() const 
		{static typename SCHEME_OPTIONS::MessageEncodingMethod messageEncodingMethod; return messageEncodingMethod;}
	const TrapdoorFunctionBounds & GetTrapdoorFunctionBounds() const 
		{return GetKey();}
	const typename BASE::TrapdoorFunctionInterface & GetTrapdoorFunctionInterface() const 
		{return GetKey();}

	// for signature scheme
	HashIdentifier GetHashIdentifier() const
	{
        typedef CPP_TYPENAME SchemeOptions::MessageEncodingMethod::HashIdentifierLookup::template HashIdentifierLookup2<CPP_TYPENAME SchemeOptions::HashFunction> L;
        return L::Lookup();
/*        typedef typename SchemeOptions::MessageEncodingMethod MEnMeth;
        typedef typename MEnMeth::HashIdentifierLookup HLookup;
        typedef typename SchemeOptions::HashFunction HashF;
        return HLookup::template HashIdentifierLookup2<HashF>::Lookup();		*/
	}
	unsigned int GetDigestSize() const
	{
		typedef CPP_TYPENAME SchemeOptions::HashFunction H;
		return H::DIGESTSIZE;
	}
};

//! .
template <class BASE, class SCHEME_OPTIONS, class KEY>
class TF_ObjectImplExtRef : public TF_ObjectImplBase<BASE, SCHEME_OPTIONS, KEY>
{
public:
	TF_ObjectImplExtRef(const KEY *pKey = NULL) : m_pKey(pKey) {}
	void SetKeyPtr(const KEY *pKey) {m_pKey = pKey;}

	const KEY & GetKey() const {return *m_pKey;}
	KEY & AccessKey() {throw NotImplemented("TF_ObjectImplExtRef: cannot modify refererenced key");}

	void CopyKeyInto(typename SCHEME_OPTIONS::PrivateKey &key) const {assert(false);}
	void CopyKeyInto(typename SCHEME_OPTIONS::PublicKey &key) const {assert(false);}

private:
	const KEY * m_pKey;
};

//! .
template <class BASE, class SCHEME_OPTIONS, class KEY>
class TF_ObjectImpl : public TF_ObjectImplBase<BASE, SCHEME_OPTIONS, KEY>
{
public:
	const KEY & GetKey() const {return m_trapdoorFunction;}
	KEY & AccessKey() {return m_trapdoorFunction;}

private:
	KEY m_trapdoorFunction;
};

//! .
template <class BASE, class SCHEME_OPTIONS>
class TF_PublicObjectImpl : public TF_ObjectImpl<BASE, SCHEME_OPTIONS, typename SCHEME_OPTIONS::PublicKey>, public PublicKeyCopier<SCHEME_OPTIONS>
{
public:
	void CopyKeyInto(typename SCHEME_OPTIONS::PublicKey &key) const {key = this->GetKey();}
};

//! .
template <class BASE, class SCHEME_OPTIONS>
class TF_PrivateObjectImpl : public TF_ObjectImpl<BASE, SCHEME_OPTIONS, typename SCHEME_OPTIONS::PrivateKey>, public PrivateKeyCopier<SCHEME_OPTIONS>
{
public:
	void CopyKeyInto(typename SCHEME_OPTIONS::PrivateKey &key) const {key = this->GetKey();}
	void CopyKeyInto(typename SCHEME_OPTIONS::PublicKey &key) const {key = this->GetKey();}
};

//! .
template <class SCHEME_OPTIONS>
class TF_SignerImpl : public TF_PrivateObjectImpl<TF_SignerBase, SCHEME_OPTIONS>
{
	PK_MessageAccumulator * NewSignatureAccumulator(RandomNumberGenerator &rng = NullRNG()) const
	{
		return new PK_MessageAccumulatorImpl<CPP_TYPENAME SCHEME_OPTIONS::HashFunction>;
	}
};

//! .
template <class SCHEME_OPTIONS>
class TF_VerifierImpl : public TF_PublicObjectImpl<TF_VerifierBase, SCHEME_OPTIONS>
{
	PK_MessageAccumulator * NewVerificationAccumulator() const
	{
		return new PK_MessageAccumulatorImpl<CPP_TYPENAME SCHEME_OPTIONS::HashFunction>;
	}
};

// ********************************************************

void P1363_MGF1KDF2_Common(HashTransformation &hash, byte *output, unsigned int outputLength, const byte *input, unsigned int inputLength, bool mask, unsigned int counterStart);

// ********************************************************

//! .
template <class H>
class P1363_KDF2
{
public:
	static void DeriveKey(byte *output, unsigned int outputLength, const byte *input, unsigned int inputLength)
	{
		H h;
		P1363_MGF1KDF2_Common(h, output, outputLength, input, inputLength, false, 1);
	}
};

// ********************************************************

template <class BASE>
class PK_FinalTemplate : public BASE
{
public:
	PK_FinalTemplate() {}

	PK_FinalTemplate(const Integer &v1)
		{this->AccessKey().Initialize(v1);}

	PK_FinalTemplate(const typename BASE::KeyClass &key)  {this->AccessKey().operator=(key);}

	template <class T>
	PK_FinalTemplate(const PublicKeyCopier<T> &key)
		{key.CopyKeyInto(this->AccessKey());}

	template <class T>
	PK_FinalTemplate(const PrivateKeyCopier<T> &key)
		{key.CopyKeyInto(this->AccessKey());}

	PK_FinalTemplate(BufferedTransformation &bt) {this->AccessKey().BERDecode(bt);}

#if (defined(_MSC_VER) && _MSC_VER < 1300)

	template <class T1, class T2>
	PK_FinalTemplate(T1 &v1, T2 &v2)
		{this->AccessKey().Initialize(v1, v2);}

	template <class T1, class T2, class T3>
	PK_FinalTemplate(T1 &v1, T2 &v2, T3 &v3)
		{this->AccessKey().Initialize(v1, v2, v3);}
	
	template <class T1, class T2, class T3, class T4>
	PK_FinalTemplate(T1 &v1, T2 &v2, T3 &v3, T4 &v4)
		{this->AccessKey().Initialize(v1, v2, v3, v4);}

	template <class T1, class T2, class T3, class T4, class T5>
	PK_FinalTemplate(T1 &v1, T2 &v2, T3 &v3, T4 &v4, T5 &v5)
		{this->AccessKey().Initialize(v1, v2, v3, v4, v5);}

	template <class T1, class T2, class T3, class T4, class T5, class T6>
	PK_FinalTemplate(T1 &v1, T2 &v2, T3 &v3, T4 &v4, T5 &v5, T6 &v6)
		{this->AccessKey().Initialize(v1, v2, v3, v4, v5, v6);}

	template <class T1, class T2, class T3, class T4, class T5, class T6, class T7>
	PK_FinalTemplate(T1 &v1, T2 &v2, T3 &v3, T4 &v4, T5 &v5, T6 &v6, T7 &v7)
		{this->AccessKey().Initialize(v1, v2, v3, v4, v5, v6, v7);}

	template <class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8>
	PK_FinalTemplate(T1 &v1, T2 &v2, T3 &v3, T4 &v4, T5 &v5, T6 &v6, T7 &v7, T8 &v8)
		{this->AccessKey().Initialize(v1, v2, v3, v4, v5, v6, v7, v8);}

#else

	template <class T1, class T2>
	PK_FinalTemplate(const T1 &v1, const T2 &v2)
		{this->AccessKey().Initialize(v1, v2);}

	template <class T1, class T2, class T3>
	PK_FinalTemplate(const T1 &v1, const T2 &v2, const T3 &v3)
		{this->AccessKey().Initialize(v1, v2, v3);}
	
	template <class T1, class T2, class T3, class T4>
	PK_FinalTemplate(const T1 &v1, const T2 &v2, const T3 &v3, const T4 &v4)
		{this->AccessKey().Initialize(v1, v2, v3, v4);}

	template <class T1, class T2, class T3, class T4, class T5>
	PK_FinalTemplate(const T1 &v1, const T2 &v2, const T3 &v3, const T4 &v4, const T5 &v5)
		{this->AccessKey().Initialize(v1, v2, v3, v4, v5);}

	template <class T1, class T2, class T3, class T4, class T5, class T6>
	PK_FinalTemplate(const T1 &v1, const T2 &v2, const T3 &v3, const T4 &v4, const T5 &v5, const T6 &v6)
		{this->AccessKey().Initialize(v1, v2, v3, v4, v5, v6);}

	template <class T1, class T2, class T3, class T4, class T5, class T6, class T7>
	PK_FinalTemplate(const T1 &v1, const T2 &v2, const T3 &v3, const T4 &v4, const T5 &v5, const T6 &v6, const T7 &v7)
		{this->AccessKey().Initialize(v1, v2, v3, v4, v5, v6, v7);}

	template <class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8>
	PK_FinalTemplate(const T1 &v1, const T2 &v2, const T3 &v3, const T4 &v4, const T5 &v5, const T6 &v6, const T7 &v7, const T8 &v8)
		{this->AccessKey().Initialize(v1, v2, v3, v4, v5, v6, v7, v8);}

	template <class T1, class T2>
	PK_FinalTemplate(T1 &v1, const T2 &v2)
		{this->AccessKey().Initialize(v1, v2);}

	template <class T1, class T2, class T3>
	PK_FinalTemplate(T1 &v1, const T2 &v2, const T3 &v3)
		{this->AccessKey().Initialize(v1, v2, v3);}
	
	template <class T1, class T2, class T3, class T4>
	PK_FinalTemplate(T1 &v1, const T2 &v2, const T3 &v3, const T4 &v4)
		{this->AccessKey().Initialize(v1, v2, v3, v4);}

	template <class T1, class T2, class T3, class T4, class T5>
	PK_FinalTemplate(T1 &v1, const T2 &v2, const T3 &v3, const T4 &v4, const T5 &v5)
		{this->AccessKey().Initialize(v1, v2, v3, v4, v5);}

	template <class T1, class T2, class T3, class T4, class T5, class T6>
	PK_FinalTemplate(T1 &v1, const T2 &v2, const T3 &v3, const T4 &v4, const T5 &v5, const T6 &v6)
		{this->AccessKey().Initialize(v1, v2, v3, v4, v5, v6);}

	template <class T1, class T2, class T3, class T4, class T5, class T6, class T7>
	PK_FinalTemplate(T1 &v1, const T2 &v2, const T3 &v3, const T4 &v4, const T5 &v5, const T6 &v6, const T7 &v7)
		{this->AccessKey().Initialize(v1, v2, v3, v4, v5, v6, v7);}

	template <class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8>
	PK_FinalTemplate(T1 &v1, const T2 &v2, const T3 &v3, const T4 &v4, const T5 &v5, const T6 &v6, const T7 &v7, const T8 &v8)
		{this->AccessKey().Initialize(v1, v2, v3, v4, v5, v6, v7, v8);}

#endif
};

//! Base class for public key signature standard classes. These classes are used to select from variants of algorithms. Note that not all standards apply to all algorithms.
struct SignatureStandard {};

template <class STANDARD, class H, class KEYS, class ALG_INFO>	// VC60 workaround: doesn't work if KEYS is first parameter
class TF_SS;

//! Trapdoor Function Based Signature Scheme
template <class STANDARD, class H, class KEYS, class ALG_INFO = TF_SS<STANDARD, H, KEYS, int> >	// VC60 workaround: doesn't work if KEYS is first parameter
class TF_SS : public KEYS
{
public:
	//! see SignatureStandard for a list of standards
	typedef STANDARD Standard;
	typedef typename Standard::SignatureMessageEncodingMethod MessageEncodingMethod;
	typedef TF_SignatureSchemeOptions<ALG_INFO, KEYS, MessageEncodingMethod, H> SchemeOptions;

	static std::string StaticAlgorithmName() {return KEYS::StaticAlgorithmName() + "/" + MessageEncodingMethod::StaticAlgorithmName() + "(" + H::StaticAlgorithmName() + ")";}

	//! implements PK_Signer interface
	typedef PK_FinalTemplate<TF_SignerImpl<SchemeOptions> > Signer;
	//! implements PK_Verifier interface
	typedef PK_FinalTemplate<TF_VerifierImpl<SchemeOptions> > Verifier;
};

NAMESPACE_END

#endif
