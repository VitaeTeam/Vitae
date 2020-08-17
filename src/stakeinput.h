// Copyright (c) 2017-2019 The PIVX developers
// Copyright (c) 2018 The VITAE developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef VITAE_STAKEINPUT_H
#define VITAE_STAKEINPUT_H

#include "chain.h"
#include "streams.h"
#include "uint256.h"

class CKeyStore;
class CWallet;
class CWalletTx;

class CStakeInput
{
protected:
    CBlockIndex* pindexFrom = nullptr;

public:
    virtual ~CStakeInput(){};
    virtual bool InitFromTxIn(const CTxIn& txin) = 0;
    virtual CBlockIndex* GetIndexFrom() = 0;
    virtual bool CreateTxIn(CWallet* pwallet, CTxIn& txIn, uint256 hashTxOut = UINT256_ZERO) = 0;
    virtual bool GetTxFrom(CTransaction& tx) const = 0;
    virtual CAmount GetValue() const = 0;
    virtual bool CreateTxOuts(CWallet* pwallet, std::vector<CTxOut>& vout, CAmount nTotal) = 0;
    virtual bool IsZVIT() const = 0;
    virtual CDataStream GetUniqueness() const = 0;
    virtual bool ContextCheck(const CBlockIndex* pTip) = 0;

};


class CVitStake : public CStakeInput
{
private:
    CTransaction txFrom;
    unsigned int nPosition;

public:
    CVitStake() {}

    bool InitFromTxIn(const CTxIn& txin);
    bool SetPrevout(CTransaction txPrev, unsigned int n);

    CBlockIndex* GetIndexFrom() override;
    bool GetTxFrom(CTransaction& tx) const override;
    CAmount GetValue() const override;
    CDataStream GetUniqueness() const override;
    bool CreateTxIn(CWallet* pwallet, CTxIn& txIn, uint256 hashTxOut = UINT256_ZERO) override;
    bool CreateTxOuts(CWallet* pwallet, std::vector<CTxOut>& vout, CAmount nTotal) override;
    bool IsZVIT() const override { return false; }
    bool ContextCheck(const CBlockIndex* pTip);
};


#endif //VITAE_STAKEINPUT_H
