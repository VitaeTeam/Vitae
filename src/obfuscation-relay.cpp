// Copyright (c) 2014-2015 The Dash developers
// Copyright (c) 2014-2015 The Dash developers
// Copyright (c) 2015-2017 The PIVX developers
// Copyright (c) 2018 The VITAE developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "messagesigner.h"
#include "obfuscation-relay.h"

CObfuScationRelay::CObfuScationRelay()
{
    vinFundamentalnode = CTxIn();
    nBlockHeight = 0;
    nRelayType = 0;
    in = CTxIn();
    out = CTxOut();
}

CObfuScationRelay::CObfuScationRelay(CTxIn& vinFundamentalnodeIn, std::vector<unsigned char>& vchSigIn, int nBlockHeightIn, int nRelayTypeIn, CTxIn& in2, CTxOut& out2)
{
    vinFundamentalnode = vinFundamentalnodeIn;
    vchSig = vchSigIn;
    nBlockHeight = nBlockHeightIn;
    nRelayType = nRelayTypeIn;
    in = in2;
    out = out2;
}

std::string CObfuScationRelay::ToString()
{
    std::ostringstream info;

    info << "vin: " << vinFundamentalnode.ToString() << " nBlockHeight: " << (int)nBlockHeight << " nRelayType: " << (int)nRelayType << " in " << in.ToString() << " out " << out.ToString();

    return info.str();
}

uint256 CObfuScationRelay::GetSignatureHash() const
{
    CHashWriter ss(SER_GETHASH, PROTOCOL_VERSION);
    ss << in << out;
    return ss.GetHash();
}

std::string CObfuScationRelay::GetStrMessage() const
{
    return in.ToString() + out.ToString();
}

bool CObfuScationRelay::Sign(std::string strSharedKey)
{
    int nHeight;
    {
        LOCK(cs_main);
        nHeight = chainActive.Height();
    }

    std::string strError = "";
    CKey key2;
    CPubKey pubkey2;

    if (!CMessageSigner::GetKeysFromSecret(strSharedKey, key2, pubkey2)) {
        return error("%s : Invalid shared key %s", __func__, strSharedKey);
    }

    if (Params().NewSigsActive(nHeight)) {
        uint256 hash = GetSignatureHash();

        if(!CHashSigner::SignHash(hash, key2, vchSig2)) {
            return error("%s : SignHash() failed", __func__);
        }

        if (!CHashSigner::VerifyHash(hash, pubkey2, vchSig2, strError)) {
            return error("%s : VerifyHash() failed, error: %s", __func__, strError);
        }

    } else {
        // use old signature format
        std::string strMessage = GetStrMessage();

        if (!CMessageSigner::SignMessage(strMessage, vchSig2, key2)) {
            return error("%s : SignMessage() failed", __func__);
        }

        if (!CMessageSigner::VerifyMessage(pubkey2, vchSig2, strMessage, strError)) {
            return error("%s : VerifyMessage() failed, error: %s\n", __func__, strError);
        }
    }

    return true;
}

bool CObfuScationRelay::CheckSignature(std::string strSharedKey) const
{
    std::string strError = "";
    CKey key2;
    CPubKey pubkey2;

    if (!CMessageSigner::GetKeysFromSecret(strSharedKey, key2, pubkey2)) {
        return error("%s : Invalid shared key %s", __func__, strSharedKey);
    }

    uint256 hash = GetSignatureHash();

    if (CHashSigner::VerifyHash(hash, pubkey2, vchSig2, strError))
        return true;

    // if new signature fails, try old format
    std::string strMessage = GetStrMessage();

    if (!CMessageSigner::VerifyMessage(pubkey2, vchSig2, strMessage, strError)) {
        return error("%s : Got bad masternode signature: %s\n", __func__, strError);
    }

    return true;
}

void CObfuScationRelay::Relay()
{
    int nCount = std::min(mnodeman.CountEnabled(ActiveProtocol()), 20);
    int nRank1 = (rand() % nCount) + 1;
    int nRank2 = (rand() % nCount) + 1;

    //keep picking another second number till we get one that doesn't match
    while (nRank1 == nRank2)
        nRank2 = (rand() % nCount) + 1;

    //printf("rank 1 - rank2 %d %d \n", nRank1, nRank2);

    //relay this message through 2 separate nodes for redundancy
    RelayThroughNode(nRank1);
    RelayThroughNode(nRank2);
}

void CObfuScationRelay::RelayThroughNode(int nRank)
{
    CFundamentalnode* pmn = mnodeman.GetFundamentalnodeByRank(nRank, nBlockHeight, ActiveProtocol());

    if (pmn != NULL) {
        //printf("RelayThroughNode %s\n", pmn->addr.ToString().c_str());
        CNode* pnode = ConnectNode((CAddress)pmn->addr, NULL, false);
        if (pnode) {
            //printf("Connected\n");
            pnode->PushMessage("dsr", (*this));
            pnode->Release();
            return;
        }
    } else {
        //printf("RelayThroughNode NULL\n");
    }
}
