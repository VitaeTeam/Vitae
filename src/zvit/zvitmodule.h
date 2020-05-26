// Copyright (c) 2019 The PIVX developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
//
#ifndef VITAE_ZVITMODULE_H
#define VITAE_ZVITMODULE_H

#include "libzerocoin/bignum.h"
#include "libzerocoin/Denominations.h"
#include "libzerocoin/CoinSpend.h"
#include "libzerocoin/Coin.h"
#include "libzerocoin/CoinRandomnessSchnorrSignature.h"
#include "libzerocoin/SpendType.h"
#include "primitives/transaction.h"
#include "script/script.h"
#include "serialize.h"
#include "uint256.h"
#include <streams.h>
#include <utilstrencodings.h>
#include "zvit/zerocoin.h"
#include "chainparams.h"

static int const PUBSPEND_SCHNORR = 4;

class PublicCoinSpend : public libzerocoin::CoinSpend {
public:

    PublicCoinSpend(libzerocoin::ZerocoinParams* params): pubCoin(params) {};

    PublicCoinSpend(libzerocoin::ZerocoinParams* params, const uint8_t version, const CBigNum& serial, const CBigNum& randomness, const uint256& ptxHash, CPubKey* pubkey);

    ~PublicCoinSpend(){};

    template <typename Stream>
    PublicCoinSpend(
            libzerocoin::ZerocoinParams* params,
            Stream& strm): pubCoin(params) {
        strm >> *this;
        this->spendType = libzerocoin::SpendType::SPEND;
    }

    const uint256 signatureHash() const override;
    void setVchSig(std::vector<unsigned char> vchSig) { this->vchSig = vchSig; };
    bool Verify(const libzerocoin::Accumulator& a, bool verifyParams = true) const override;
    bool validate() const;

    // Members
    CBigNum randomness;
    libzerocoin::CoinRandomnessSchnorrSignature schnorrSig;
    // prev out values
    uint256 txHash = 0;
    unsigned int outputIndex = -1;
    libzerocoin::PublicCoin pubCoin;

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion) {
        READWRITE(version);
        READWRITE(coinSerialNumber);
        if (version < PUBSPEND_SCHNORR)
            READWRITE(randomness);
        else
            READWRITE(schnorrSig);
        if (libzerocoin::ExtractVersionFromSerial(coinSerialNumber) >= libzerocoin::PrivateCoin::PUBKEY_VERSION) {
            READWRITE(pubkey);
            READWRITE(vchSig);
        }
    }
};


class CValidationState;

namespace ZVITModule {
    bool createInput(CTxIn &in, CZerocoinMint& mint, uint256 hashTxOut, const int spendVersion);
    PublicCoinSpend parseCoinSpend(const CTxIn &in);
    bool parseCoinSpend(const CTxIn &in, const CTransaction& tx, const CTxOut &prevOut, PublicCoinSpend& publicCoinSpend);
    bool validateInput(const CTxIn &in, const CTxOut &prevOut, const CTransaction& tx, PublicCoinSpend& ret);

    // Public zc spend parse
    /**
     *
     * @param in --> public zc spend input
     * @param tx --> input parent
     * @param publicCoinSpend ---> return the publicCoinSpend parsed
     * @return true if everything went ok
     */
    bool ParseZerocoinPublicSpend(const CTxIn &in, const CTransaction& tx, CValidationState& state, PublicCoinSpend& publicCoinSpend);
};


#endif //VITAE_ZVITMODULE_H
