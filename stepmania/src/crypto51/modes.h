#ifndef CRYPTOPP_MODES_H
#define CRYPTOPP_MODES_H

/*! \file
*/

#include "cryptlib.h"
#include "secblock.h"
#include "misc.h"
#include "strciphr.h"
#include "argnames.h"
#include "algparam.h"

NAMESPACE_BEGIN(CryptoPP)

//! Cipher mode documentation. See NIST SP 800-38A for definitions of these modes.

/*! Each class derived from this one defines two types, Encryption and Decryption, 
	both of which implement the SymmetricCipher interface.
	For each mode there are two classes, one of which is a template class,
	and the other one has a name that ends in "_ExternalCipher".
	The "external cipher" mode objects hold a reference to the underlying block cipher,
	instead of holding an instance of it. The reference must be passed in to the constructor.
	For the "cipher holder" classes, the CIPHER template parameter should be a class
	derived from BlockCipherDocumentation, for example DES or AES.
*/
struct CipherModeDocumentation : public SymmetricCipherDocumentation
{
};

class CipherModeBase : public SymmetricCipher
{
public:
	unsigned int MinKeyLength() const {return m_cipher->MinKeyLength();}
	unsigned int MaxKeyLength() const {return m_cipher->MaxKeyLength();}
	unsigned int DefaultKeyLength() const {return m_cipher->DefaultKeyLength();}
	unsigned int GetValidKeyLength(unsigned int n) const {return m_cipher->GetValidKeyLength(n);}
	bool IsValidKeyLength(unsigned int n) const {return m_cipher->IsValidKeyLength(n);}

	void SetKey(const byte *key, unsigned int length, const NameValuePairs &params = g_nullNameValuePairs);

	unsigned int OptimalDataAlignment() const {return BlockSize();}

	unsigned int IVSize() const {return BlockSize();}
	void GetNextIV(byte *IV);
	virtual IV_Requirement IVRequirement() const =0;

protected:
	inline unsigned int BlockSize() const {assert(m_register.size() > 0); return m_register.size();}
	void SetIV(const byte *iv);
	virtual void SetFeedbackSize(unsigned int feedbackSize)
	{
		if (!(feedbackSize == 0 || feedbackSize == BlockSize()))
			throw InvalidArgument("CipherModeBase: feedback size cannot be specified for this cipher mode");
	}
	virtual void ResizeBuffers()
	{
		m_register.New(m_cipher->BlockSize());
	}
	virtual void UncheckedSetKey(const NameValuePairs &params, const byte *key, unsigned int length) =0;

	BlockCipher *m_cipher;
	SecByteBlock m_register;
};

template <class POLICY_INTERFACE>
class ModePolicyCommonTemplate : public CipherModeBase, public POLICY_INTERFACE
{
	unsigned int GetAlignment() const {return m_cipher->BlockAlignment();}
	void CipherSetKey(const NameValuePairs &params, const byte *key, unsigned int length)
	{
		m_cipher->SetKey(key, length, params);
		ResizeBuffers();
		int feedbackSize = params.GetIntValueWithDefault(Name::FeedbackSize(), 0);
		SetFeedbackSize(feedbackSize);
		const byte *iv = params.GetValueWithDefault(Name::IV(), (const byte *)NULL);
		SetIV(iv);
	}
};

class CFB_ModePolicy : public ModePolicyCommonTemplate<CFB_CipherAbstractPolicy>
{
public:
	IV_Requirement IVRequirement() const {return RANDOM_IV;}

protected:
	unsigned int GetBytesPerIteration() const {return m_feedbackSize;}
	byte * GetRegisterBegin() {return m_register + BlockSize() - m_feedbackSize;}
	void TransformRegister()
	{
		m_cipher->ProcessBlock(m_register, m_temp);
		memmove(m_register, m_register+m_feedbackSize, BlockSize()-m_feedbackSize);
		memcpy(m_register+BlockSize()-m_feedbackSize, m_temp, m_feedbackSize);
	}
	void CipherResynchronize(const byte *iv)
	{
		memcpy(m_register, iv, BlockSize());
		TransformRegister();
	}
	void SetFeedbackSize(unsigned int feedbackSize)
	{
		if (feedbackSize > BlockSize())
			throw InvalidArgument("CFB_Mode: invalid feedback size");
		m_feedbackSize = feedbackSize ? feedbackSize : BlockSize();
	}
	void ResizeBuffers()
	{
		CipherModeBase::ResizeBuffers();
		m_temp.New(BlockSize());
	}

	SecByteBlock m_temp;
	unsigned int m_feedbackSize;
};

class BlockOrientedCipherModeBase : public CipherModeBase
{
public:
	void UncheckedSetKey(const NameValuePairs &params, const byte *key, unsigned int length);
	unsigned int MandatoryBlockSize() const {return BlockSize();}
	bool IsRandomAccess() const {return false;}
	bool IsSelfInverting() const {return false;}
	bool IsForwardTransformation() const {return m_cipher->IsForwardTransformation();}
	void Resynchronize(const byte *iv) {memcpy(m_register, iv, BlockSize());}
	void ProcessData(byte *outString, const byte *inString, unsigned int length);

protected:
	bool RequireAlignedInput() const {return true;}
	virtual void ProcessBlocks(byte *outString, const byte *inString, unsigned int numberOfBlocks) =0;
	void ResizeBuffers()
	{
		CipherModeBase::ResizeBuffers();
		m_buffer.New(BlockSize());
	}

	SecByteBlock m_buffer;
};

class CBC_ModeBase : public BlockOrientedCipherModeBase
{
public:
	IV_Requirement IVRequirement() const {return UNPREDICTABLE_RANDOM_IV;}
	bool RequireAlignedInput() const {return false;}
	unsigned int MinLastBlockSize() const {return 0;}
};

class CBC_Encryption : public CBC_ModeBase
{
public:
	void ProcessBlocks(byte *outString, const byte *inString, unsigned int numberOfBlocks);
};

class CBC_CTS_Encryption : public CBC_Encryption
{
public:
	void SetStolenIV(byte *iv) {m_stolenIV = iv;}
	unsigned int MinLastBlockSize() const {return BlockSize()+1;}
	void ProcessLastBlock(byte *outString, const byte *inString, unsigned int length);

protected:
	void UncheckedSetKey(const NameValuePairs &params, const byte *key, unsigned int length)
	{
		CBC_Encryption::UncheckedSetKey(params, key, length);
		m_stolenIV = params.GetValueWithDefault(Name::StolenIV(), (byte *)NULL);
	}

	byte *m_stolenIV;
};

class CBC_Decryption : public CBC_ModeBase
{
public:
	void ProcessBlocks(byte *outString, const byte *inString, unsigned int numberOfBlocks);
	
protected:
	void ResizeBuffers()
	{
		BlockOrientedCipherModeBase::ResizeBuffers();
		m_temp.New(BlockSize());
	}
	SecByteBlock m_temp;
};

//! .
template <class CIPHER, class BASE>
class CipherModeFinalTemplate_CipherHolder : public ObjectHolder<CIPHER>, public BASE
{
public:
	CipherModeFinalTemplate_CipherHolder()
	{
		this->m_cipher = &this->m_object;
		this->ResizeBuffers();
	}
	CipherModeFinalTemplate_CipherHolder(const byte *key, unsigned int length)
	{
		this->m_cipher = &this->m_object;
		this->SetKey(key, length);
	}
	CipherModeFinalTemplate_CipherHolder(const byte *key, unsigned int length, const byte *iv, int feedbackSize = 0)
	{
		this->m_cipher = &this->m_object;
		this->SetKey(key, length, MakeParameters("IV", iv)("FeedbackSize", feedbackSize));
	}
};

//! CFB mode
template <class CIPHER>
struct CFB_Mode : public CipherModeDocumentation
{
	typedef CipherModeFinalTemplate_CipherHolder<CPP_TYPENAME CIPHER::Encryption, ConcretePolicyHolder<Empty, CFB_EncryptionTemplate<AbstractPolicyHolder<CFB_CipherAbstractPolicy, CFB_ModePolicy> > > > Encryption;
	typedef CipherModeFinalTemplate_CipherHolder<CPP_TYPENAME CIPHER::Encryption, ConcretePolicyHolder<Empty, CFB_DecryptionTemplate<AbstractPolicyHolder<CFB_CipherAbstractPolicy, CFB_ModePolicy> > > > Decryption;
};

//! CBC mode
template <class CIPHER>
struct CBC_Mode : public CipherModeDocumentation
{
	typedef CipherModeFinalTemplate_CipherHolder<CPP_TYPENAME CIPHER::Encryption, CBC_Encryption> Encryption;
	typedef CipherModeFinalTemplate_CipherHolder<CPP_TYPENAME CIPHER::Decryption, CBC_Decryption> Decryption;
};

NAMESPACE_END

#endif
