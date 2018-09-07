// Copyright (c) 2014-2015 The Dash developers
// Copyright (c) 2015-2017 The PIVX developers
// Copyright (c) 2018 The VITAE developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.


//#include "bignum.h"
#include "sync.h"
#include "net.h"
#include "key.h"
#include "util.h"
#include "script/script.h"
#include "base58.h"
#include "protocol.h"
#include "mn-spork.h"
#include "main.h"
#include <boost/lexical_cast.hpp>

using namespace std;
using namespace boost;

class CMNSporkMessage;
class CMNSporkManager;

CMNSporkManager mn_sporkManager;

std::map<uint256, CMNSporkMessage> mapMNSporks;
std::map<int, CMNSporkMessage> mapMNSporksActive;


void ProcessMNSpork(CNode* pfrom, std::string& strCommand, CDataStream& vRecv)
{
    if(fMNLiteMode) return; //disable all darksend/masternode related functionality

    if (strCommand == "mn_spork")
    {
        //LogPrintf("ProcessMNSpork::mn_spork\n");
        CDataStream vMsg(vRecv);
        CMNSporkMessage mn_spork;
        vRecv >> mn_spork;

        if(chainActive.Tip() == NULL) return;

        uint256 hash = mn_spork.GetHash();
        if(mapMNSporksActive.count(mn_spork.nMNSporkID)) {
            if(mapMNSporksActive[mn_spork.nMNSporkID].nTimeSigned >= mn_spork.nTimeSigned){
                if(fDebug) LogPrintf("mn_spork - seen %s block %d \n", hash.ToString().c_str(), chainActive.Tip()->nHeight);
                return;
            } else {
                if(fDebug) LogPrintf("mn_spork - got updated mn_spork %s block %d \n", hash.ToString().c_str(), chainActive.Tip()->nHeight);
            }
        }

        LogPrintf("mn_spork - new %s ID %d Time %d bestHeight %d\n", hash.ToString().c_str(), mn_spork.nMNSporkID, mn_spork.nValue, chainActive.Tip()->nHeight);

        if(!mn_sporkManager.CheckSignature(mn_spork)){
            LogPrintf("mn_spork - invalid signature\n");
            Misbehaving(pfrom->GetId(), 100);
            return;
        }

        mapMNSporks[hash] = mn_spork;
        mapMNSporksActive[mn_spork.nMNSporkID] = mn_spork;
        mn_sporkManager.Relay(mn_spork);

        //does a task if needed
        ExecuteMNSpork(mn_spork.nMNSporkID, mn_spork.nValue);
    }
    if (strCommand == "getmn_sporks")
    {
        std::map<int, CMNSporkMessage>::iterator it = mapMNSporksActive.begin();

        while(it != mapMNSporksActive.end()) {
            pfrom->PushMessage("mn_spork", it->second);
            it++;
        }
    }

}

// grab the mn_spork, otherwise say it's off
bool IsMNSporkActive(int nMNSporkID)
{
    int64_t r = 0;

    if(mapMNSporksActive.count(nMNSporkID)){
        r = mapMNSporksActive[nMNSporkID].nValue;
    } else {
        if(nMNSporkID == MN_SPORK_1_MASTERNODE_PAYMENTS_ENFORCEMENT) r = MN_SPORK_1_MASTERNODE_PAYMENTS_ENFORCEMENT_DEFAULT;
        if(nMNSporkID == MN_SPORK_2_INSTANTX) r = MN_SPORK_2_INSTANTX_DEFAULT;
        if(nMNSporkID == MN_SPORK_3_INSTANTX_BLOCK_FILTERING) r = MN_SPORK_3_INSTANTX_BLOCK_FILTERING_DEFAULT;
        if(nMNSporkID == MN_SPORK_5_MAX_VALUE) r = MN_SPORK_5_MAX_VALUE_DEFAULT;
        if(nMNSporkID == MN_SPORK_7_MASTERNODE_SCANNING) r = MN_SPORK_7_MASTERNODE_SCANNING;

        if(r == 0) LogPrintf("GetMNSpork::Unknown MNSpork %d\n", nMNSporkID);
    }
    if(r == 0) r = 4070908800; //return 2099-1-1 by default

    return r < GetTime();
}

// grab the value of the mn_spork on the network, or the default
int GetMNSporkValue(int nMNSporkID)
{
    int r = 0;

    if(mapMNSporksActive.count(nMNSporkID)){
        r = mapMNSporksActive[nMNSporkID].nValue;
    } else {
        if(nMNSporkID == MN_SPORK_1_MASTERNODE_PAYMENTS_ENFORCEMENT) r = MN_SPORK_1_MASTERNODE_PAYMENTS_ENFORCEMENT_DEFAULT;
        if(nMNSporkID == MN_SPORK_2_INSTANTX) r = MN_SPORK_2_INSTANTX_DEFAULT;
        if(nMNSporkID == MN_SPORK_3_INSTANTX_BLOCK_FILTERING) r = MN_SPORK_3_INSTANTX_BLOCK_FILTERING_DEFAULT;
        if(nMNSporkID == MN_SPORK_5_MAX_VALUE) r = MN_SPORK_5_MAX_VALUE_DEFAULT;
        if(nMNSporkID == MN_SPORK_7_MASTERNODE_SCANNING) r = MN_SPORK_7_MASTERNODE_SCANNING;

        if(r == 0) LogPrintf("GetMNSpork::Unknown MNSpork %d\n", nMNSporkID);
    }

    return r;
}

void ExecuteMNSpork(int nMNSporkID, int nValue)
{
}


bool CMNSporkManager::CheckSignature(CMNSporkMessage& mn_spork)
{
    //note: need to investigate why this is failing
    std::string strMessage = boost::lexical_cast<std::string>(mn_spork.nMNSporkID) + boost::lexical_cast<std::string>(mn_spork.nValue) + boost::lexical_cast<std::string>(mn_spork.nTimeSigned);
    std::string strPubKey = (Params().NetworkID() == CBaseChainParams::MAIN) ? strMainPubKey : strTestPubKey;
    CPubKey pubkey(ParseHex(strPubKey));

    std::string errorMessage = "";
    if(!obfuScationSigner.VerifyMessage(pubkey, mn_spork.vchSig, strMessage, errorMessage)){
        return false;
    }

    return true;
}

bool CMNSporkManager::Sign(CMNSporkMessage& mn_spork)
{
    std::string strMessage = boost::lexical_cast<std::string>(mn_spork.nMNSporkID) + boost::lexical_cast<std::string>(mn_spork.nValue) + boost::lexical_cast<std::string>(mn_spork.nTimeSigned);

    CKey key2;
    CPubKey pubkey2;
    std::string errorMessage = "";

    if(!obfuScationSigner.SetKey(strMasterPrivKey, errorMessage, key2, pubkey2))
    {
        LogPrintf("CMasternodePayments::Sign - ERROR: Invalid masternodeprivkey: '%s'\n", errorMessage.c_str());
        return false;
    }

    if(!obfuScationSigner.SignMessage(strMessage, errorMessage, mn_spork.vchSig, key2)) {
        LogPrintf("CMasternodePayments::Sign - Sign message failed");
        return false;
    }

    if(!obfuScationSigner.VerifyMessage(pubkey2, mn_spork.vchSig, strMessage, errorMessage)) {
        LogPrintf("CMasternodePayments::Sign - Verify message failed");
        return false;
    }

    return true;
}

bool CMNSporkManager::UpdateMNSpork(int nMNSporkID, int64_t nValue)
{

    CMNSporkMessage msg;
    msg.nMNSporkID = nMNSporkID;
    msg.nValue = nValue;
    msg.nTimeSigned = GetTime();

    if(Sign(msg)){
        Relay(msg);
        mapMNSporks[msg.GetHash()] = msg;
        mapMNSporksActive[nMNSporkID] = msg;
        return true;
    }

    return false;
}

void CMNSporkManager::Relay(CMNSporkMessage& msg)
{
    CInv inv(MSG_MN_SPORK, msg.GetHash());

    vector<CInv> vInv;
    vInv.push_back(inv);
    LOCK(cs_vNodes);
    BOOST_FOREACH(CNode* pnode, vNodes){
        pnode->PushMessage("inv", vInv);
    }
}

bool CMNSporkManager::SetPrivKey(std::string strPrivKey)
{
    CMNSporkMessage msg;

    // Test signing successful, proceed
    strMasterPrivKey = strPrivKey;

    Sign(msg);

    if(CheckSignature(msg)){
        LogPrintf("CMNSporkManager::SetPrivKey - Successfully initialized as mn_spork signer\n");
        return true;
    } else {
        return false;
    }
}

int CMNSporkManager::GetMNSporkIDByName(std::string strName)
{
    if(strName == "MN_SPORK_1_MASTERNODE_PAYMENTS_ENFORCEMENT") return MN_SPORK_1_MASTERNODE_PAYMENTS_ENFORCEMENT;
    if(strName == "MN_SPORK_2_INSTANTX") return MN_SPORK_2_INSTANTX;
    if(strName == "MN_SPORK_3_INSTANTX_BLOCK_FILTERING") return MN_SPORK_3_INSTANTX_BLOCK_FILTERING;
    if(strName == "MN_SPORK_5_MAX_VALUE") return MN_SPORK_5_MAX_VALUE;
    if(strName == "MN_SPORK_7_MASTERNODE_SCANNING") return MN_SPORK_7_MASTERNODE_SCANNING;

    return -1;
}

std::string CMNSporkManager::GetMNSporkNameByID(int id)
{
    if(id == MN_SPORK_1_MASTERNODE_PAYMENTS_ENFORCEMENT) return "MN_SPORK_1_MASTERNODE_PAYMENTS_ENFORCEMENT";
    if(id == MN_SPORK_2_INSTANTX) return "MN_SPORK_2_INSTANTX";
    if(id == MN_SPORK_3_INSTANTX_BLOCK_FILTERING) return "MN_SPORK_3_INSTANTX_BLOCK_FILTERING";
    if(id == MN_SPORK_5_MAX_VALUE) return "MN_SPORK_5_MAX_VALUE";
    if(id == MN_SPORK_7_MASTERNODE_SCANNING) return "MN_SPORK_7_MASTERNODE_SCANNING";

    return "Unknown";
}
