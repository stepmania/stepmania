// modes.cpp - written and placed in the public domain by Wei Dai

#include "pch.h"
#include "modes.h"

#include "strciphr.cpp"

NAMESPACE_BEGIN(CryptoPP)

// explicit instantiations for Darwin gcc-932.1
template class CFB_CipherTemplate<AbstractPolicyHolder<CFB_CipherAbstractPolicy, SymmetricCipher> >;
template class CFB_EncryptionTemplate<>;
template class CFB_DecryptionTemplate<>;
template class AdditiveCipherTemplate<>;
template class CFB_CipherTemplate<AbstractPolicyHolder<CFB_CipherAbstractPolicy, CFB_ModePolicy> >;
template class CFB_EncryptionTemplate<AbstractPolicyHolder<CFB_CipherAbstractPolicy, CFB_ModePolicy> >;
template class CFB_DecryptionTemplate<AbstractPolicyHolder<CFB_CipherAbstractPolicy, CFB_ModePolicy> >;

void CipherModeBase::SetKey(const byte *key, unsigned int length, const NameValuePairs &params)
{
	UncheckedSetKey(params, key, length);	// the underlying cipher will check the key length
}

void CipherModeBase::GetNextIV(byte *IV)
{
	if (!IsForwardTransformation())
		throw NotImplemented("CipherModeBase: GetNextIV() must be called on an encryption object");

	m_cipher->ProcessBlock(m_register);
	memcpy(IV, m_register, BlockSize());
}

void CipherModeBase::SetIV(const byte *iv)
{
	if (iv)
		Resynchronize(iv);
	else if (IsResynchronizable())
	{
		if (!CanUseStructuredIVs())
			throw InvalidArgument("CipherModeBase: this cipher mode cannot use a null IV");

		// use all zeros as default IV
		SecByteBlock iv(BlockSize());
		memset(iv, 0, iv.size());
		Resynchronize(iv);
	}
}

static inline void IncrementCounterByOne(byte *inout, unsigned int s)
{
	for (int i=s-1, carry=1; i>=0 && carry; i--)
		carry = !++inout[i];
}

static inline void IncrementCounterByOne(byte *output, const byte *input, unsigned int s)
{
	for (int i=s-1, carry=1; i>=0; i--)
		carry = !(output[i] = input[i]+carry) && carry;
}

void BlockOrientedCipherModeBase::UncheckedSetKey(const NameValuePairs &params, const byte *key, unsigned int length)
{
	m_cipher->SetKey(key, length, params);
	ResizeBuffers();
	const byte *iv = params.GetValueWithDefault(Name::IV(), (const byte *)NULL);
	SetIV(iv);
}

void BlockOrientedCipherModeBase::ProcessData(byte *outString, const byte *inString, unsigned int length)
{
	unsigned int s = BlockSize();
	assert(length % s == 0);
	unsigned int alignment = m_cipher->BlockAlignment();
	bool inputAlignmentOk = !RequireAlignedInput() || IsAlignedOn(inString, alignment);

	if (IsAlignedOn(outString, alignment))
	{
		if (inputAlignmentOk)
			ProcessBlocks(outString, inString, length / s);
		else
		{
			memcpy(outString, inString, length);
			ProcessBlocks(outString, outString, length / s);
		}
	}
	else
	{
		while (length)
		{
			if (inputAlignmentOk)
				ProcessBlocks(m_buffer, inString, 1);
			else
			{
				memcpy(m_buffer, inString, s);
				ProcessBlocks(m_buffer, m_buffer, 1);
			}
			memcpy(outString, m_buffer, s);
			inString += s;
			outString += s;
			length -= s;
		}
	}
}

void CBC_Encryption::ProcessBlocks(byte *outString, const byte *inString, unsigned int numberOfBlocks)
{
	unsigned int blockSize = BlockSize();
	while (numberOfBlocks--)
	{
		xorbuf(m_register, inString, blockSize);
		m_cipher->ProcessBlock(m_register);
		memcpy(outString, m_register, blockSize);
		inString += blockSize;
		outString += blockSize;
	}
}

void CBC_CTS_Encryption::ProcessLastBlock(byte *outString, const byte *inString, unsigned int length)
{
	if (length <= BlockSize())
	{
		if (!m_stolenIV)
			throw InvalidArgument("CBC_Encryption: message is too short for ciphertext stealing");

		// steal from IV
		memcpy(outString, m_register, length);
		outString = m_stolenIV;
	}
	else
	{
		// steal from next to last block
		xorbuf(m_register, inString, BlockSize());
		m_cipher->ProcessBlock(m_register);
		inString += BlockSize();
		length -= BlockSize();
		memcpy(outString+BlockSize(), m_register, length);
	}

	// output last full ciphertext block
	xorbuf(m_register, inString, length);
	m_cipher->ProcessBlock(m_register);
	memcpy(outString, m_register, BlockSize());
}

void CBC_Decryption::ProcessBlocks(byte *outString, const byte *inString, unsigned int numberOfBlocks)
{
	unsigned int blockSize = BlockSize();
	while (numberOfBlocks--)
	{
		memcpy(m_temp, inString, blockSize);
		m_cipher->ProcessBlock(m_temp, outString);
		xorbuf(outString, m_register, blockSize);
		m_register.swap(m_temp);
		inString += blockSize;
		outString += blockSize;
	}
}

NAMESPACE_END
