// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin Core developers
// Copyright (c) 2014-2015 The Soferox developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include "amount.h"
#include "uint256.h"
#include "serialize.h"

namespace XCoin {

class ConstBuf {
public:
	const unsigned char *P;
	size_t Size;

	template <typename T>
	ConstBuf(const T pb, const T pe) {
		if (pb == pe) {
			P = 0;
			Size = 0;
		} else {
			P = (unsigned char*)(&pb[0]);
			Size = (pe-pb) * sizeof(pb[0]);
		}
	}

	ConstBuf(const std::vector<unsigned char>& vch) {
		if (vch.empty()) {
			P = 0;
			Size = 0;
		} else {
			P = &vch[0];
			Size = vch.size();
		}
	}
};


uint256 HashSofero(const ConstBuf& cbuf);

uint256 HashFromTx(const ConstBuf& cbuf);
uint256 HashForSignature(const ConstBuf& cbuf);
inline uint256 HashPow(const ConstBuf& cbuf) { return HashSofero(cbuf); }
inline uint256 HashMessage(const ConstBuf& cbuf) { return HashSofero(cbuf); }
inline uint256 HashForAddress(const ConstBuf& cbuf) { return HashSofero(cbuf); }


class SoferoHasher {
private:
	void *ctx;
public:
	void Finalize(unsigned char hash[32]);
	SoferoHasher& Write(const unsigned char *data, size_t len);
	SoferoHasher();
	SoferoHasher(SoferoHasher&& x);
	~SoferoHasher();
	SoferoHasher& operator=(SoferoHasher&& x);
};

class CSoferoHashWriter
{
private:
	SoferoHasher ctx;

	const int nType;
	const int nVersion;
public:

	CSoferoHashWriter(int nTypeIn, int nVersionIn) : nType(nTypeIn), nVersion(nVersionIn) {}

	int GetType() const { return nType; }
	int GetVersion() const { return nVersion; }

	void write(const char *pch, size_t size) {
		ctx.Write((const unsigned char*)pch, size);
	}

	// invalidates the object
	uint256 GetHash() {
		uint256 result;
		ctx.Finalize((unsigned char*)&result);
		return result;
	}

	template<typename T>
	CSoferoHashWriter& operator<<(const T& obj) {
		// Serialize to this stream
		::Serialize(*this, obj);
		return (*this);
	}
};


} // XCoin::
