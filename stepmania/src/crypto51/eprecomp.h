#ifndef CRYPTOPP_EPRECOMP_H
#define CRYPTOPP_EPRECOMP_H

#include "integer.h"
#include "algebra.h"
#include <vector>

NAMESPACE_BEGIN(CryptoPP)

template <class T>
class DL_GroupPrecomputation
{
public:
	typedef T Element;

	virtual bool NeedConversions() const {return false;}
	virtual Element ConvertIn(const Element &v) const {return v;}
	virtual Element ConvertOut(const Element &v) const {return v;}
	virtual const AbstractGroup<Element> & GetGroup() const =0;
	virtual Element BERDecodeElement(BufferedTransformation &bt) const =0;
	virtual void DEREncodeElement(BufferedTransformation &bt, const Element &P) const =0;
};

template <class T>
class DL_FixedBasePrecomputation
{
public:
	typedef T Element;

	virtual bool IsInitialized() const =0;
	virtual void SetBase(const DL_GroupPrecomputation<Element> &group, const Element &base) =0;
	virtual const Element & GetBase(const DL_GroupPrecomputation<Element> &group) const =0;
	virtual void Precompute(const DL_GroupPrecomputation<Element> &group, unsigned int maxExpBits, unsigned int storage) =0;
	virtual void Load(const DL_GroupPrecomputation<Element> &group, BufferedTransformation &storedPrecomputation) =0;
	virtual void Save(const DL_GroupPrecomputation<Element> &group, BufferedTransformation &storedPrecomputation) const =0;
	virtual Element Exponentiate(const DL_GroupPrecomputation<Element> &group, const Integer &exponent) const =0;
	virtual Element CascadeExponentiate(const DL_GroupPrecomputation<Element> &group, const Integer &exponent, const DL_FixedBasePrecomputation<Element> &pc2, const Integer &exponent2) const =0;
};

NAMESPACE_END

#endif
