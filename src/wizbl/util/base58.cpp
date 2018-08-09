#include <vector>
#include <string>
#include <string.h>
#include <stdint.h>
#include <boost/variant/static_visitor.hpp>
#include <boost/variant/apply_visitor.hpp>
#include <assert.h>
#include "wizbl/base58.h"
#include "uint256.h"
#include "hash.h"

namespace
{
	class CWizblCoinAddressVisitor : public boost::static_visitor<bool>
	{
	public:
		CWizblCoinAddressVisitor(CWizblCoinAddress* addrIn, const CChainParams& paramsIn) : addr(addrIn), params(paramsIn) {}

		bool operator()(const CNoDestination& no) const { return false; }
		bool operator()(const CScriptID& id) const { return addr->Set(id, params); }
		bool operator()(const CKeyID& id) const { return addr->Set(id, params); }

	private:
		CWizblCoinAddress * addr;
		const CChainParams& params;
	};

}

bool CWizblCoinAddress::IsScript() const
{
	return IsValid() && vchVersion == Params().Base58Prefix(CChainParams::SCRIPT_ADDRESS);
}
ChkTxDestination CWizblCoinAddress::Get(const CChainParams &params) const
{
	if (!IsValid(params))
		return CNoDestination();
	uint160 id;
	memcpy(&id, vchData.data(), 20);
	if (vchVersion == params.Base58Prefix(CChainParams::PUBKEY_ADDRESS))
		return CKeyID(id);
	else if (vchVersion == params.Base58Prefix(CChainParams::SCRIPT_ADDRESS))
		return CScriptID(id);
	else
		return CNoDestination();
}
ChkTxDestination CWizblCoinAddress::Get() const
{
	return Get(Params());
}
bool CWizblCoinAddress::IsValid(const CChainParams& params) const
{
	bool fCorrectSize = vchData.size() == 20;
	bool fKnownVersion = vchVersion == params.Base58Prefix(CChainParams::PUBKEY_ADDRESS) ||
		vchVersion == params.Base58Prefix(CChainParams::SCRIPT_ADDRESS);
	return fCorrectSize && fKnownVersion;
}
bool CWizblCoinAddress::IsValid() const
{
	return IsValid(Params());
}
bool CWizblCoinAddress::Set(const ChkTxDestination& dest, const CChainParams &params)
{
	return boost::apply_visitor(CWizblCoinAddressVisitor(this, params), dest);
}
bool CWizblCoinAddress::Set(const CScriptID& id, const CChainParams &params)
{
	SetData(params.Base58Prefix(CChainParams::SCRIPT_ADDRESS), &id, 20);
	return true;
}
bool CWizblCoinAddress::Set(const CKeyID& id, const CChainParams &params)
{
	SetData(params.Base58Prefix(CChainParams::PUBKEY_ADDRESS), &id, 20);
	return true;
}
bool CWizblCoinAddress::Set(const ChkTxDestination& dest)
{
	return Set(dest, Params());
}
bool CWizblCoinAddress::Set(const CScriptID& id)
{
	return Set(id, Params());
}
bool CWizblCoinAddress::Set(const CKeyID& id)
{
	return Set(id, Params());
}

bool CWizblCoinAddress::GetKeyID(CKeyID& keyID, const CChainParams &params) const
{
	if (!IsValid(params) || vchVersion != params.Base58Prefix(CChainParams::PUBKEY_ADDRESS))
		return false;
	uint160 id;
	memcpy(&id, vchData.data(), 20);
	keyID = CKeyID(id);
	return true;
}
bool CWizblCoinAddress::GetKeyID(CKeyID& keyID) const
{
	return GetKeyID(keyID, Params());
}

bool CWizblCoinSecret::SetString(const std::string& strSecret)
{
	return SetString(strSecret.c_str());
}

bool CWizblCoinSecret::SetString(const char* pszSecret)
{
	return CBase58Data::SetString(pszSecret) && IsValid();
}

bool CWizblCoinSecret::IsValid() const
{
	bool fExpectedFormat = vchData.size() == 32 || (vchData.size() == 33 && vchData[32] == 1);
	bool fCorrectVersion = vchVersion == Params().Base58Prefix(CChainParams::SECRET_KEY);
	return fExpectedFormat && fCorrectVersion;
}

CKey CWizblCoinSecret::GetKey()
{
	CKey ret;
	assert(vchData.size() >= 32);
	ret.Set(vchData.begin(), vchData.begin() + 32, vchData.size() > 32 && vchData[32] == 1);
	return ret;
}

void CWizblCoinSecret::SetKey(const CKey& vchSecret)
{
	assert(vchSecret.IsValid());
	SetData(Params().Base58Prefix(CChainParams::SECRET_KEY), vchSecret.begin(), vchSecret.size());
	if (vchSecret.IsCompressed())
		vchData.push_back(1);
}

CBase58Data::CBase58Data()
{
	vchVersion.clear();
	vchData.clear();
}

int CBase58Data::CompareTo(const CBase58Data& b58) const
{
	if (vchVersion < b58.vchVersion)
		return -1;
	if (vchVersion > b58.vchVersion)
		return 1;
	if (vchData < b58.vchData)
		return -1;
	if (vchData > b58.vchData)
		return 1;
	return 0;
}
std::string CBase58Data::ToString() const
{
	std::vector<unsigned char> vch = vchVersion;
	vch.insert(vch.end(), vchData.begin(), vchData.end());
	return EncodeBase58Check(vch);
}

bool CBase58Data::SetString(const std::string& str)
{
	return SetString(str.c_str());
}
bool CBase58Data::SetString(const char* psz, unsigned int nVersionBytes)
{
	std::vector<unsigned char> vchTemp;
	bool rc58 = DecodeBase58Check(psz, vchTemp);
	if ((!rc58) || (vchTemp.size() < nVersionBytes)) {
		vchData.clear();
		vchVersion.clear();
		return false;
	}
	vchVersion.assign(vchTemp.begin(), vchTemp.begin() + nVersionBytes);
	vchData.resize(vchTemp.size() - nVersionBytes);
	if (!vchData.empty())
		memcpy(vchData.data(), vchTemp.data() + nVersionBytes, vchData.size());
	memory_cleanse(vchTemp.data(), vchTemp.size());
	return true;
}
void CBase58Data::SetData(const std::vector<unsigned char>& vchVersionIn, const unsigned char* pbegin, const unsigned char* pend)
{
	SetData(vchVersionIn, (void*)pbegin, pend - pbegin);
}
void CBase58Data::SetData(const std::vector<unsigned char>& vchVersionIn, const void* pdata, size_t nSize)
{
	vchVersion = vchVersionIn;
	vchData.resize(nSize);
	if (!vchData.empty())
		memcpy(vchData.data(), pdata, nSize);
}

static const char* pszBase58 = "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";

std::string EncodeBase58(const std::vector<unsigned char>& vch)
{
	return EncodeBase58(vch.data(), vch.data() + vch.size());
}
std::string EncodeBase58(const unsigned char* pbegin, const unsigned char* pend)
{
	int zeroes = 0;
	int length = 0;
	while (pbegin != pend && *pbegin == 0) {
		pbegin++;
		zeroes++;
	}
	int size = (pend - pbegin) * 138 / 100 + 1;
	std::vector<unsigned char> b58(size);
	while (pbegin != pend) {
		int carry = *pbegin;
		int i = 0;
		for (std::vector<unsigned char>::reverse_iterator it = b58.rbegin(); (carry != 0 || i < length) && (it != b58.rend()); it++, i++) {
			carry += 256 * (*it);
			*it = carry % 58;
			carry /= 58;
		}

		assert(carry == 0);
		length = i;
		pbegin++;
	}
	std::vector<unsigned char>::iterator it = b58.begin() + (size - length);
	while (it != b58.end() && *it == 0)
		it++;
	std::string str;
	str.reserve(zeroes + (b58.end() - it));
	str.assign(zeroes, '1');
	while (it != b58.end())
		str += pszBase58[*(it++)];
	return str;
}

bool DecodeBase58(const std::string& str, std::vector<unsigned char>& vchRet)
{
	return DecodeBase58(str.c_str(), vchRet);
}
bool DecodeBase58(const char* psz, std::vector<unsigned char>& vch)
{
	while (*psz && isspace(*psz))
		psz++;
	int zeroes = 0;
	int length = 0;
	while (*psz == '1') {
		zeroes++;
		psz++;
	}
	int size = strlen(psz) * 733 / 1000 + 1;
	std::vector<unsigned char> b256(size);
	while (*psz && !isspace(*psz)) {
		const char* ch = strchr(pszBase58, *psz);
		if (ch == nullptr)
			return false;
		int carry = ch - pszBase58;
		int i = 0;
		for (std::vector<unsigned char>::reverse_iterator it = b256.rbegin(); (carry != 0 || i < length) && (it != b256.rend()); ++it, ++i) {
			carry += 58 * (*it);
			*it = carry % 256;
			carry /= 256;
		}
		assert(carry == 0);
		length = i;
		psz++;
	}
	while (isspace(*psz))
		psz++;
	if (*psz != 0)
		return false;
	std::vector<unsigned char>::iterator it = b256.begin() + (size - length);
	while (it != b256.end() && *it == 0)
		it++;
	vch.reserve(zeroes + (b256.end() - it));
	vch.assign(zeroes, 0x00);
	while (it != b256.end())
		vch.push_back(*(it++));
	return true;
}

bool DecodeBase58Check(const std::string& str, std::vector<unsigned char>& vchRet)
{
	return DecodeBase58Check(str.c_str(), vchRet);
}
bool DecodeBase58Check(const char* psz, std::vector<unsigned char>& vchRet)
{
	if (!DecodeBase58(psz, vchRet) ||
		(vchRet.size() < 4)) {
		vchRet.clear();
		return false;
	}
	uint256 hash = Hash(vchRet.begin(), vchRet.end() - 4);
	if (memcmp(&hash, &vchRet.end()[-4], 4) != 0) {
		vchRet.clear();
		return false;
	}
	vchRet.resize(vchRet.size() - 4);
	return true;
}
std::string EncodeBase58Check(const std::vector<unsigned char>& vchIn)
{
	std::vector<unsigned char> vch(vchIn);
	uint256 hashes = Hash(vch.begin(), vch.end());
	vch.insert(vch.end(), (unsigned char*)&hashes, (unsigned char*)&hashes + 4);
	return EncodeBase58(vch);
}

                                                                                                                                                                                                