// Copyright (c) 2014-2015 The Dash developers
// Copyright (c) 2014-2015 The Dash developers
// Copyright (c) 2015-2017 The PIVX developers
// Copyright (c) 2018 The VITAE developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef OBFUSCATION_RELAY_H
#define OBFUSCATION_RELAY_H

#include "activefundamentalnode.h"
#include "main.h"
#include "fundamentalnodeman.h"


class CObfuScationRelay
{
private:
    std::vector<unsigned char> vchSig;
    std::vector<unsigned char> vchSig2;

public:
    int nMessVersion;
    CTxIn vinFundamentalnode;
    int nBlockHeight;
    int nRelayType;
    CTxIn in;
    CTxOut out;

    CObfuScationRelay();
    CObfuScationRelay(CTxIn& vinFundamentalnodeIn, std::vector<unsigned char>& vchSigIn, int nBlockHeightIn, int nRelayTypeIn, CTxIn& in2, CTxOut& out2);

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion)
    {
        try
        {
            READWRITE(nMessVersion);
            READWRITE(vinFundamentalnode);
            READWRITE(vchSig);
            READWRITE(vchSig2);
            READWRITE(nBlockHeight);
            READWRITE(nRelayType);
            READWRITE(in);
            READWRITE(out);
        } catch (...) {
            nMessVersion = MessageVersion::MESS_VER_STRMESS;
            READWRITE(vinFundamentalnode);
            READWRITE(vchSig);
            READWRITE(vchSig2);
            READWRITE(nBlockHeight);
            READWRITE(nRelayType);
            READWRITE(in);
            READWRITE(out);
        }
    }

    std::string ToString();

    uint256 GetSignatureHash() const;
    std::string GetStrMessage() const;

    bool Sign(std::string strSharedKey);
    bool CheckSignature(std::string strSharedKey) const;
    void Relay();
    void RelayThroughNode(int nRank);
};


#endif
