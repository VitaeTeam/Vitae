// Copyright (c) 2014-2015 The Dash developers
// Copyright (c) 2015-2017 The PIVX developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "fundamentalnodeman.h"
#include "activefundamentalnode.h"
#include "addrman.h"
#include "fundamentalnode.h"
#include "obfuscation.h"
#include "spork.h"
#include "util.h"
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>

#define MN_WINNER_MINIMUM_AGE 8000    // Age in seconds. This should be > FUNDAMENTALNODE_REMOVAL_SECONDS to avoid misconfigured new nodes in the list.

/** fundamentalnode manager */
CFundamentalnodeMan mnodeman;

struct CompareLastPaid {
    bool operator()(const pair<int64_t, CTxIn>& t1,
        const pair<int64_t, CTxIn>& t2) const
    {
        return t1.first < t2.first;
    }
};

struct CompareScoreTxIn {
    bool operator()(const pair<int64_t, CTxIn>& t1,
        const pair<int64_t, CTxIn>& t2) const
    {
        return t1.first < t2.first;
    }
};

struct CompareScoreMN {
    bool operator()(const pair<int64_t, CFundamentalnode>& t1,
        const pair<int64_t, CFundamentalnode>& t2) const
    {
        return t1.first < t2.first;
    }
};

//
// CFundamentalnodeDB
//

CFundamentalnodeDB::CFundamentalnodeDB()
{
    pathMN = GetDataDir() / "fncache.dat";
    strMagicMessage = "FundamentalnodeCache";
}

bool CFundamentalnodeDB::Write(const CFundamentalnodeMan& mnodemanToSave)
{
    int64_t nStart = GetTimeMillis();

    // serialize, checksum data up to that point, then append checksum
    CDataStream ssFundamentalnodes(SER_DISK, CLIENT_VERSION);
    ssFundamentalnodes << strMagicMessage;                   // fundamentalnode cache file specific magic message
    ssFundamentalnodes << FLATDATA(Params().MessageStart()); // network specific magic number
    ssFundamentalnodes << mnodemanToSave;
    uint256 hash = Hash(ssFundamentalnodes.begin(), ssFundamentalnodes.end());
    ssFundamentalnodes << hash;

    // open output file, and associate with CAutoFile
    FILE* file = fopen(pathMN.string().c_str(), "wb");
    CAutoFile fileout(file, SER_DISK, CLIENT_VERSION);
    if (fileout.IsNull())
        return error("%s : Failed to open file %s", __func__, pathMN.string());

    // Write and commit header, data
    try {
        fileout << ssFundamentalnodes;
    } catch (const std::exception& e) {
        return error("%s : Serialize or I/O error - %s", __func__, e.what());
    }
    //    FileCommit(fileout);
    fileout.fclose();

    LogPrint("fundamentalnode","Written info to mncache.dat  %dms\n", GetTimeMillis() - nStart);
    LogPrint("fundamentalnode","  %s\n", mnodemanToSave.ToString());

    return true;
}

CFundamentalnodeDB::ReadResult CFundamentalnodeDB::Read(CFundamentalnodeMan& mnodemanToLoad, bool fDryRun)
{
    int64_t nStart = GetTimeMillis();
    // open input file, and associate with CAutoFile
    FILE* file = fopen(pathMN.string().c_str(), "rb");
    CAutoFile filein(file, SER_DISK, CLIENT_VERSION);
    if (filein.IsNull()) {
        error("%s : Failed to open file %s", __func__, pathMN.string());
        return FileError;
    }

    // use file size to size memory buffer
    int fileSize = boost::filesystem::file_size(pathMN);
    int dataSize = fileSize - sizeof(uint256);
    // Don't try to resize to a negative number if file is small
    if (dataSize < 0)
        dataSize = 0;
    vector<unsigned char> vchData;
    vchData.resize(dataSize);
    uint256 hashIn;

    // read data and checksum from file
    try {
        filein.read((char*)&vchData[0], dataSize);
        filein >> hashIn;
    } catch (const std::exception& e) {
        error("%s : Deserialize or I/O error - %s", __func__, e.what());
        return HashReadError;
    }
    filein.fclose();

    CDataStream ssFundamentalnodes(vchData, SER_DISK, CLIENT_VERSION);

    // verify stored checksum matches input data
    uint256 hashTmp = Hash(ssFundamentalnodes.begin(), ssFundamentalnodes.end());
    if (hashIn != hashTmp) {
        error("%s : Checksum mismatch, data corrupted", __func__);
        return IncorrectHash;
    }

    unsigned char pchMsgTmp[4];
    std::string strMagicMessageTmp;
    try {
        // de-serialize file header (fundamentalnode cache file specific magic message) and ..

        ssFundamentalnodes >> strMagicMessageTmp;

        // ... verify the message matches predefined one
        if (strMagicMessage != strMagicMessageTmp) {
            error("%s : Invalid fundamentalnode cache magic message", __func__);
            return IncorrectMagicMessage;
        }

        // de-serialize file header (network specific magic number) and ..
        ssFundamentalnodes >> FLATDATA(pchMsgTmp);

        // ... verify the network matches ours
        if (memcmp(pchMsgTmp, Params().MessageStart(), sizeof(pchMsgTmp))) {
            error("%s : Invalid network magic number", __func__);
            return IncorrectMagicNumber;
        }
        // de-serialize data into CFundamentalnodeMan object
        ssFundamentalnodes >> mnodemanToLoad;
    } catch (const std::exception& e) {
        mnodemanToLoad.Clear();
        error("%s : Deserialize or I/O error - %s", __func__, e.what());
        return IncorrectFormat;
    }

    LogPrint("fundamentalnode","Loaded info from mncache.dat  %dms\n", GetTimeMillis() - nStart);
    LogPrint("fundamentalnode","  %s\n", mnodemanToLoad.ToString());
    if (!fDryRun) {
        LogPrint("fundamentalnode","Fundamentalnode manager - cleaning....\n");
        mnodemanToLoad.CheckAndRemove(true);
        LogPrint("fundamentalnode","Fundamentalnode manager - result:\n");
        LogPrint("fundamentalnode","  %s\n", mnodemanToLoad.ToString());
    }

    return Ok;
}

void DumpFundamentalnodes()
{
    int64_t nStart = GetTimeMillis();

    CFundamentalnodeDB mndb;
    CFundamentalnodeMan tempMnodeman;

    LogPrint("fundamentalnode","Verifying mncache.dat format...\n");
    CFundamentalnodeDB::ReadResult readResult = mndb.Read(tempMnodeman, true);
    // there was an error and it was not an error on file opening => do not proceed
    if (readResult == CFundamentalnodeDB::FileError)
        LogPrint("fundamentalnode","Missing fundamentalnode cache file - mncache.dat, will try to recreate\n");
    else if (readResult != CFundamentalnodeDB::Ok) {
        LogPrint("fundamentalnode","Error reading mncache.dat: ");
        if (readResult == CFundamentalnodeDB::IncorrectFormat)
            LogPrint("fundamentalnode","magic is ok but data has invalid format, will try to recreate\n");
        else {
            LogPrint("fundamentalnode","file format is unknown or invalid, please fix it manually\n");
            return;
        }
    }
    LogPrint("fundamentalnode","Writting info to mncache.dat...\n");
    mndb.Write(mnodeman);

    LogPrint("fundamentalnode","Fundamentalnode dump finished  %dms\n", GetTimeMillis() - nStart);
}

CFundamentalnodeMan::CFundamentalnodeMan()
{
    nDsqCount = 0;
}

bool CFundamentalnodeMan::Add(CFundamentalnode& mn)
{
    LOCK(cs);

    if (!mn.IsEnabled())
        return false;

    CFundamentalnode* pmn = Find(mn.vin);
    if (pmn == NULL) {
        LogPrint("fundamentalnode", "CFundamentalnodeMan: Adding new Fundamentalnode %s - %i now\n", mn.vin.prevout.hash.ToString(), size() + 1);
        vFundamentalnodes.push_back(mn);
        return true;
    }

    return false;
}

void CFundamentalnodeMan::AskForMN(CNode* pnode, CTxIn& vin)
{
    std::map<COutPoint, int64_t>::iterator i = mWeAskedForFundamentalnodeListEntry.find(vin.prevout);
    if (i != mWeAskedForFundamentalnodeListEntry.end()) {
        int64_t t = (*i).second;
        if (GetTime() < t) return; // we've asked recently
    }

    // ask for the fnb info once from the node that sent fnp

    LogPrint("fundamentalnode", "CFundamentalnodeMan::AskForMN - Asking node for missing entry, vin: %s\n", vin.prevout.hash.ToString());
    pnode->PushMessage("obseg", vin);
    int64_t askAgain = GetTime() + FUNDAMENTALNODE_MIN_MNP_SECONDS;
    mWeAskedForFundamentalnodeListEntry[vin.prevout] = askAgain;
}

void CFundamentalnodeMan::Check()
{
    LOCK(cs);

    BOOST_FOREACH (CFundamentalnode& mn, vFundamentalnodes) {
        mn.Check();
    }
}

void CFundamentalnodeMan::CheckAndRemove(bool forceExpiredRemoval)
{
    Check();

    LOCK(cs);

    //remove inactive and outdated
    vector<CFundamentalnode>::iterator it = vFundamentalnodes.begin();
    while (it != vFundamentalnodes.end()) {
        if ((*it).activeState == CFundamentalnode::FUNDAMENTALNODE_REMOVE ||
            (*it).activeState == CFundamentalnode::FUNDAMENTALNODE_VIN_SPENT ||
            (forceExpiredRemoval && (*it).activeState == CFundamentalnode::FUNDAMENTALNODE_EXPIRED) ||
            (*it).protocolVersion < fundamentalnodePayments.GetMinFundamentalnodePaymentsProto()) {
            LogPrint("fundamentalnode", "CFundamentalnodeMan: Removing inactive Fundamentalnode %s - %i now\n", (*it).vin.prevout.hash.ToString(), size() - 1);

            //erase all of the broadcasts we've seen from this vin
            // -- if we missed a few pings and the node was removed, this will allow is to get it back without them
            //    sending a brand new fnb
            map<uint256, CFundamentalnodeBroadcast>::iterator it3 = mapSeenFundamentalnodeBroadcast.begin();
            while (it3 != mapSeenFundamentalnodeBroadcast.end()) {
                if ((*it3).second.vin == (*it).vin) {
                    fundamentalnodeSync.mapSeenSyncMNB.erase((*it3).first);
                    mapSeenFundamentalnodeBroadcast.erase(it3++);
                } else {
                    ++it3;
                }
            }

            // allow us to ask for this fundamentalnode again if we see another ping
            map<COutPoint, int64_t>::iterator it2 = mWeAskedForFundamentalnodeListEntry.begin();
            while (it2 != mWeAskedForFundamentalnodeListEntry.end()) {
                if ((*it2).first == (*it).vin.prevout) {
                    mWeAskedForFundamentalnodeListEntry.erase(it2++);
                } else {
                    ++it2;
                }
            }

            it = vFundamentalnodes.erase(it);
        } else {
            ++it;
        }
    }

    // check who's asked for the Fundamentalnode list
    map<CNetAddr, int64_t>::iterator it1 = mAskedUsForFundamentalnodeList.begin();
    while (it1 != mAskedUsForFundamentalnodeList.end()) {
        if ((*it1).second < GetTime()) {
            mAskedUsForFundamentalnodeList.erase(it1++);
        } else {
            ++it1;
        }
    }

    // check who we asked for the Fundamentalnode list
    it1 = mWeAskedForFundamentalnodeList.begin();
    while (it1 != mWeAskedForFundamentalnodeList.end()) {
        if ((*it1).second < GetTime()) {
            mWeAskedForFundamentalnodeList.erase(it1++);
        } else {
            ++it1;
        }
    }

    // check which Fundamentalnodes we've asked for
    map<COutPoint, int64_t>::iterator it2 = mWeAskedForFundamentalnodeListEntry.begin();
    while (it2 != mWeAskedForFundamentalnodeListEntry.end()) {
        if ((*it2).second < GetTime()) {
            mWeAskedForFundamentalnodeListEntry.erase(it2++);
        } else {
            ++it2;
        }
    }

    // remove expired mapSeenFundamentalnodeBroadcast
    map<uint256, CFundamentalnodeBroadcast>::iterator it3 = mapSeenFundamentalnodeBroadcast.begin();
    while (it3 != mapSeenFundamentalnodeBroadcast.end()) {
        if ((*it3).second.lastPing.sigTime < GetTime() - (FUNDAMENTALNODE_REMOVAL_SECONDS * 2)) {
            mapSeenFundamentalnodeBroadcast.erase(it3++);
            fundamentalnodeSync.mapSeenSyncMNB.erase((*it3).second.GetHash());
        } else {
            ++it3;
        }
    }

    // remove expired mapSeenFundamentalnodePing
    map<uint256, CFundamentalnodePing>::iterator it4 = mapSeenFundamentalnodePing.begin();
    while (it4 != mapSeenFundamentalnodePing.end()) {
        if ((*it4).second.sigTime < GetTime() - (FUNDAMENTALNODE_REMOVAL_SECONDS * 2)) {
            mapSeenFundamentalnodePing.erase(it4++);
        } else {
            ++it4;
        }
    }
}

void CFundamentalnodeMan::Clear()
{
    LOCK(cs);
    vFundamentalnodes.clear();
    mAskedUsForFundamentalnodeList.clear();
    mWeAskedForFundamentalnodeList.clear();
    mWeAskedForFundamentalnodeListEntry.clear();
    mapSeenFundamentalnodeBroadcast.clear();
    mapSeenFundamentalnodePing.clear();
    nDsqCount = 0;
}

int CFundamentalnodeMan::stable_size ()
{
    int nStable_size = 0;
    int nMinProtocol = ActiveProtocol();
    int64_t nFundamentalnode_Min_Age = MN_WINNER_MINIMUM_AGE;
    int64_t nFundamentalnode_Age = 0;

    BOOST_FOREACH (CFundamentalnode& mn, vFundamentalnodes) {
        if (mn.protocolVersion < nMinProtocol) {
            continue; // Skip obsolete versions
        }
        if (IsSporkActive (SPORK_8_FUNDAMENTALNODE_PAYMENT_ENFORCEMENT)) {
            nFundamentalnode_Age = GetAdjustedTime() - mn.sigTime;
            if ((nFundamentalnode_Age) < nFundamentalnode_Min_Age) {
                continue; // Skip fundamentalnodes younger than (default) 8000 sec (MUST be > FUNDAMENTALNODE_REMOVAL_SECONDS)
            }
        }
        mn.Check ();
        if (!mn.IsEnabled ())
            continue; // Skip not-enabled fundamentalnodes

        nStable_size++;
    }

    return nStable_size;
}

int CFundamentalnodeMan::CountEnabled(int protocolVersion)
{
    int i = 0;
    protocolVersion = protocolVersion == -1 ? fundamentalnodePayments.GetMinFundamentalnodePaymentsProto() : protocolVersion;

    BOOST_FOREACH (CFundamentalnode& mn, vFundamentalnodes) {
        mn.Check();
        if (mn.protocolVersion < protocolVersion || !mn.IsEnabled()) continue;
        i++;
    }

    return i;
}

void CFundamentalnodeMan::CountNetworks(int protocolVersion, int& ipv4, int& ipv6, int& onion)
{
    protocolVersion = protocolVersion == -1 ? fundamentalnodePayments.GetMinFundamentalnodePaymentsProto() : protocolVersion;

    BOOST_FOREACH (CFundamentalnode& mn, vFundamentalnodes) {
        mn.Check();
        std::string strHost;
        int port;
        SplitHostPort(mn.addr.ToString(), port, strHost);
        CNetAddr node = CNetAddr(strHost, false);
        int nNetwork = node.GetNetwork();
        switch (nNetwork) {
            case 1 :
                ipv4++;
                break;
            case 2 :
                ipv6++;
                break;
            case 3 :
                onion++;
                break;
        }
    }
}

void CFundamentalnodeMan::DsegUpdate(CNode* pnode)
{
    LOCK(cs);

    if (Params().NetworkID() == CBaseChainParams::MAIN) {
        if (!(pnode->addr.IsRFC1918() || pnode->addr.IsLocal())) {
            std::map<CNetAddr, int64_t>::iterator it = mWeAskedForFundamentalnodeList.find(pnode->addr);
            if (it != mWeAskedForFundamentalnodeList.end()) {
                if (GetTime() < (*it).second) {
                    LogPrint("fundamentalnode", "obseg - we already asked peer %i for the list; skipping...\n", pnode->GetId());
                    return;
                }
            }
        }
    }

    pnode->PushMessage("obseg", CTxIn());
    int64_t askAgain = GetTime() + FUNDAMENTALNODES_DSEG_SECONDS;
    mWeAskedForFundamentalnodeList[pnode->addr] = askAgain;
}

CFundamentalnode* CFundamentalnodeMan::Find(const CScript& payee)
{
    LOCK(cs);
    CScript payee2;

    BOOST_FOREACH (CFundamentalnode& mn, vFundamentalnodes) {
        payee2 = GetScriptForDestination(mn.pubKeyCollateralAddress.GetID());
        if (payee2 == payee)
            return &mn;
    }
    return NULL;
}

CFundamentalnode* CFundamentalnodeMan::Find(const CTxIn& vin)
{
    LOCK(cs);

    BOOST_FOREACH (CFundamentalnode& mn, vFundamentalnodes) {
        if (mn.vin.prevout == vin.prevout)
            return &mn;
    }
    return NULL;
}


CFundamentalnode* CFundamentalnodeMan::Find(const CPubKey& pubKeyFundamentalnode)
{
    LOCK(cs);

    BOOST_FOREACH (CFundamentalnode& mn, vFundamentalnodes) {
        if (mn.pubKeyFundamentalnode == pubKeyFundamentalnode)
            return &mn;
    }
    return NULL;
}

//
// Deterministically select the oldest/best fundamentalnode to pay on the network
//
CFundamentalnode* CFundamentalnodeMan::GetNextFundamentalnodeInQueueForPayment(int nBlockHeight, bool fFilterSigTime, int& nCount)
{
    LOCK(cs);

    CFundamentalnode* pBestFundamentalnode = NULL;
    std::vector<pair<int64_t, CTxIn> > vecFundamentalnodeLastPaid;

    /*
        Make a vector with all of the last paid times
    */

    int nMnCount = CountEnabled();
    BOOST_FOREACH (CFundamentalnode& mn, vFundamentalnodes) {
        mn.Check();
        if (!mn.IsEnabled()) continue;

        // //check protocol version
        if (mn.protocolVersion < fundamentalnodePayments.GetMinFundamentalnodePaymentsProto()) continue;

        //it's in the list (up to 8 entries ahead of current block to allow propagation) -- so let's skip it
        if (fundamentalnodePayments.IsScheduled(mn, nBlockHeight)) continue;

        //it's too new, wait for a cycle
        if (fFilterSigTime && mn.sigTime + (nMnCount * 2.6 * 60) > GetAdjustedTime()) continue;

        //make sure it has as many confirmations as there are fundamentalnodes
        if (mn.GetFundamentalnodeInputAge() < nMnCount) continue;

        vecFundamentalnodeLastPaid.push_back(make_pair(mn.SecondsSincePayment(), mn.vin));
    }

    nCount = (int)vecFundamentalnodeLastPaid.size();

    //when the network is in the process of upgrading, don't penalize nodes that recently restarted
    if (fFilterSigTime && nCount < nMnCount / 3) return GetNextFundamentalnodeInQueueForPayment(nBlockHeight, false, nCount);

    // Sort them high to low
    sort(vecFundamentalnodeLastPaid.rbegin(), vecFundamentalnodeLastPaid.rend(), CompareLastPaid());

    // Look at 1/10 of the oldest nodes (by last payment), calculate their scores and pay the best one
    //  -- This doesn't look at who is being paid in the +8-10 blocks, allowing for double payments very rarely
    //  -- 1/100 payments should be a double payment on mainnet - (1/(3000/10))*2
    //  -- (chance per block * chances before IsScheduled will fire)
    int nTenthNetwork = CountEnabled() / 10;
    int nCountTenth = 0;
    uint256 nHigh = 0;
    BOOST_FOREACH (PAIRTYPE(int64_t, CTxIn) & s, vecFundamentalnodeLastPaid) {
        CFundamentalnode* pmn = Find(s.second);
        if (!pmn) break;

        uint256 n = pmn->CalculateScore(1, nBlockHeight - 100);
        if (n > nHigh) {
            nHigh = n;
            pBestFundamentalnode = pmn;
        }
        nCountTenth++;
        if (nCountTenth >= nTenthNetwork) break;
    }
    return pBestFundamentalnode;
}

CFundamentalnode* CFundamentalnodeMan::FindRandomNotInVec(std::vector<CTxIn>& vecToExclude, int protocolVersion)
{
    LOCK(cs);

    protocolVersion = protocolVersion == -1 ? fundamentalnodePayments.GetMinFundamentalnodePaymentsProto() : protocolVersion;

    int nCountEnabled = CountEnabled(protocolVersion);
    LogPrint("fundamentalnode", "CFundamentalnodeMan::FindRandomNotInVec - nCountEnabled - vecToExclude.size() %d\n", nCountEnabled - vecToExclude.size());
    if (nCountEnabled - vecToExclude.size() < 1) return NULL;

    int rand = GetRandInt(nCountEnabled - vecToExclude.size());
    LogPrint("fundamentalnode", "CFundamentalnodeMan::FindRandomNotInVec - rand %d\n", rand);
    bool found;

    BOOST_FOREACH (CFundamentalnode& mn, vFundamentalnodes) {
        if (mn.protocolVersion < protocolVersion || !mn.IsEnabled()) continue;
        found = false;
        BOOST_FOREACH (CTxIn& usedVin, vecToExclude) {
            if (mn.vin.prevout == usedVin.prevout) {
                found = true;
                break;
            }
        }
        if (found) continue;
        if (--rand < 1) {
            return &mn;
        }
    }

    return NULL;
}

CFundamentalnode* CFundamentalnodeMan::GetCurrentFundamentalNode(int mod, int64_t nBlockHeight, int minProtocol)
{
    int64_t score = 0;
    CFundamentalnode* winner = NULL;

    // scan for winner
    BOOST_FOREACH (CFundamentalnode& mn, vFundamentalnodes) {
        mn.Check();
        if (mn.protocolVersion < minProtocol || !mn.IsEnabled()) continue;

        // calculate the score for each Fundamentalnode
        uint256 n = mn.CalculateScore(mod, nBlockHeight);
        int64_t n2 = n.GetCompact(false);

        // determine the winner
        if (n2 > score) {
            score = n2;
            winner = &mn;
        }
    }

    return winner;
}

int CFundamentalnodeMan::GetFundamentalnodeRank(const CTxIn& vin, int64_t nBlockHeight, int minProtocol, bool fOnlyActive)
{
    std::vector<pair<int64_t, CTxIn> > vecFundamentalnodeScores;
    int64_t nFundamentalnode_Min_Age = MN_WINNER_MINIMUM_AGE;
    int64_t nFundamentalnode_Age = 0;

    //make sure we know about this block
    uint256 hash = 0;
    if (!GetBlockHash(hash, nBlockHeight)) return -1;

    // scan for winner
    BOOST_FOREACH (CFundamentalnode& mn, vFundamentalnodes) {
        if (mn.protocolVersion < minProtocol) {
            LogPrint("fundamentalnode","Skipping Fundamentalnode with obsolete version %d\n", mn.protocolVersion);
            continue;                                                       // Skip obsolete versions
        }

        if (IsSporkActive(SPORK_8_FUNDAMENTALNODE_PAYMENT_ENFORCEMENT)) {
            nFundamentalnode_Age = GetAdjustedTime() - mn.sigTime;
            if ((nFundamentalnode_Age) < nFundamentalnode_Min_Age) {
                if (fDebug) LogPrint("fundamentalnode","Skipping just activated Fundamentalnode. Age: %ld\n", nFundamentalnode_Age);
                continue;                                                   // Skip fundamentalnodes younger than (default) 1 hour
            }
        }
        if (fOnlyActive) {
            mn.Check();
            if (!mn.IsEnabled()) continue;
        }
        uint256 n = mn.CalculateScore(1, nBlockHeight);
        int64_t n2 = n.GetCompact(false);

        vecFundamentalnodeScores.push_back(make_pair(n2, mn.vin));
    }

    sort(vecFundamentalnodeScores.rbegin(), vecFundamentalnodeScores.rend(), CompareScoreTxIn());

    int rank = 0;
    BOOST_FOREACH (PAIRTYPE(int64_t, CTxIn) & s, vecFundamentalnodeScores) {
        rank++;
        if (s.second.prevout == vin.prevout) {
            return rank;
        }
    }

    return -1;
}

std::vector<pair<int, CFundamentalnode> > CFundamentalnodeMan::GetFundamentalnodeRanks(int64_t nBlockHeight, int minProtocol)
{
    std::vector<pair<int64_t, CFundamentalnode> > vecFundamentalnodeScores;
    std::vector<pair<int, CFundamentalnode> > vecFundamentalnodeRanks;

    //make sure we know about this block
    uint256 hash = 0;
    if (!GetBlockHash(hash, nBlockHeight)) return vecFundamentalnodeRanks;

    // scan for winner
    BOOST_FOREACH (CFundamentalnode& mn, vFundamentalnodes) {
        mn.Check();

        if (mn.protocolVersion < minProtocol) continue;

        if (!mn.IsEnabled()) {
            vecFundamentalnodeScores.push_back(make_pair(9999, mn));
            continue;
        }

        uint256 n = mn.CalculateScore(1, nBlockHeight);
        int64_t n2 = n.GetCompact(false);

        vecFundamentalnodeScores.push_back(make_pair(n2, mn));
    }

    sort(vecFundamentalnodeScores.rbegin(), vecFundamentalnodeScores.rend(), CompareScoreMN());

    int rank = 0;
    BOOST_FOREACH (PAIRTYPE(int64_t, CFundamentalnode) & s, vecFundamentalnodeScores) {
        rank++;
        vecFundamentalnodeRanks.push_back(make_pair(rank, s.second));
    }

    return vecFundamentalnodeRanks;
}

CFundamentalnode* CFundamentalnodeMan::GetFundamentalnodeByRank(int nRank, int64_t nBlockHeight, int minProtocol, bool fOnlyActive)
{
    std::vector<pair<int64_t, CTxIn> > vecFundamentalnodeScores;

    // scan for winner
    BOOST_FOREACH (CFundamentalnode& mn, vFundamentalnodes) {
        if (mn.protocolVersion < minProtocol) continue;
        if (fOnlyActive) {
            mn.Check();
            if (!mn.IsEnabled()) continue;
        }

        uint256 n = mn.CalculateScore(1, nBlockHeight);
        int64_t n2 = n.GetCompact(false);

        vecFundamentalnodeScores.push_back(make_pair(n2, mn.vin));
    }

    sort(vecFundamentalnodeScores.rbegin(), vecFundamentalnodeScores.rend(), CompareScoreTxIn());

    int rank = 0;
    BOOST_FOREACH (PAIRTYPE(int64_t, CTxIn) & s, vecFundamentalnodeScores) {
        rank++;
        if (rank == nRank) {
            return Find(s.second);
        }
    }

    return NULL;
}

void CFundamentalnodeMan::ProcessFundamentalnodeConnections()
{
    //we don't care about this for regtest
    if (Params().NetworkID() == CBaseChainParams::REGTEST) return;

    LOCK(cs_vNodes);
    BOOST_FOREACH (CNode* pnode, vNodes) {
        if (pnode->fObfuScationMaster) {
            if (obfuScationPool.pSubmittedToFundamentalnode != NULL && pnode->addr == obfuScationPool.pSubmittedToFundamentalnode->addr) continue;
            LogPrint("fundamentalnode","Closing Fundamentalnode connection peer=%i \n", pnode->GetId());
            pnode->fObfuScationMaster = false;
            pnode->Release();
        }
    }
}

void CFundamentalnodeMan::ProcessMessage(CNode* pfrom, std::string& strCommand, CDataStream& vRecv)
{
    if (fLiteMode) return; //disable all Obfuscation/Fundamentalnode related functionality
    if (!fundamentalnodeSync.IsBlockchainSynced()) return;

    LOCK(cs_process_message);

    if (strCommand == "fnb") { //Fundamentalnode Broadcast
        CFundamentalnodeBroadcast fnb;
        vRecv >> fnb;

        if (mapSeenFundamentalnodeBroadcast.count(fnb.GetHash())) { //seen
            fundamentalnodeSync.AddedFundamentalnodeList(fnb.GetHash());
            return;
        }
        mapSeenFundamentalnodeBroadcast.insert(make_pair(fnb.GetHash(), fnb));

        int nDoS = 0;
        if (!fnb.CheckAndUpdate(nDoS)) {
            if (nDoS > 0)
                Misbehaving(pfrom->GetId(), nDoS);

            //failed
            return;
        }

        uint256 hashBlock =0 ;
        CTransaction tx;
        // make sure the vout that was signed is related to the transaction that spawned the Fundamentalnode
        //  - this is expensive, so it's only done once per Fundamentalnode
        if (!obfuScationSigner.IsVinAssociatedWithPubkey(fnb.vin, fnb.pubKeyCollateralAddress, tx, hashBlock)) {
            LogPrint("fundamentalnode","fnb - Got mismatched pubkey and vin\n");
            Misbehaving(pfrom->GetId(), 33);
            return;
        }

        // make sure it's still unspent
        //  - this is checked later by .check() in many places and by ThreadCheckObfuScationPool()
        if (fnb.CheckInputsAndAdd(nDoS)) {
            // use this as a peer
            addrman.Add(CAddress(fnb.addr), pfrom->addr, 2 * 60 * 60);
            fundamentalnodeSync.AddedFundamentalnodeList(fnb.GetHash());
        } else {
            LogPrint("fundamentalnode","fnb - Rejected Fundamentalnode entry %s\n", fnb.vin.prevout.hash.ToString());

            if (nDoS > 0)
                Misbehaving(pfrom->GetId(), nDoS);
        }
    }

    else if (strCommand == "fnp") { //Fundamentalnode Ping
        CFundamentalnodePing fnp;
        vRecv >> fnp;

        LogPrint("fundamentalnode", "fnp - Fundamentalnode ping, vin: %s\n", fnp.vin.prevout.hash.ToString());

        if (mapSeenFundamentalnodePing.count(fnp.GetHash())) return; //seen
        mapSeenFundamentalnodePing.insert(make_pair(fnp.GetHash(), fnp));

        int nDoS = 0;
        if (fnp.CheckAndUpdate(nDoS)) return;

        if (nDoS > 0) {
            // if anything significant failed, mark that node
            Misbehaving(pfrom->GetId(), nDoS);
        } else {
            // if nothing significant failed, search existing Fundamentalnode list
            CFundamentalnode* pmn = Find(fnp.vin);
            // if it's known, don't ask for the fnb, just return
            if (pmn != NULL) return;
        }

        // something significant is broken or mn is unknown,
        // we might have to ask for a fundamentalnode entry once
        AskForMN(pfrom, fnp.vin);

    } else if (strCommand == "obseg") { //Get Fundamentalnode list or specific entry

        CTxIn vin;
        vRecv >> vin;

        if (vin == CTxIn()) { //only should ask for this once
            //local network
            bool isLocal = (pfrom->addr.IsRFC1918() || pfrom->addr.IsLocal());

            if (!isLocal && Params().NetworkID() == CBaseChainParams::MAIN) {
                std::map<CNetAddr, int64_t>::iterator i = mAskedUsForFundamentalnodeList.find(pfrom->addr);
                if (i != mAskedUsForFundamentalnodeList.end()) {
                    int64_t t = (*i).second;
                    if (GetTime() < t) {
                        Misbehaving(pfrom->GetId(), 34);
                        LogPrint("fundamentalnode","obseg - peer already asked me for the list\n");
                        return;
                    }
                }
                int64_t askAgain = GetTime() + FUNDAMENTALNODES_DSEG_SECONDS;
                mAskedUsForFundamentalnodeList[pfrom->addr] = askAgain;
            }
        } //else, asking for a specific node which is ok


        int nInvCount = 0;

        BOOST_FOREACH (CFundamentalnode& mn, vFundamentalnodes) {
            if (mn.addr.IsRFC1918()) continue; //local network

            if (mn.IsEnabled()) {
                LogPrint("fundamentalnode", "obseg - Sending Fundamentalnode entry - %s \n", mn.vin.prevout.hash.ToString());
                if (vin == CTxIn() || vin == mn.vin) {
                    CFundamentalnodeBroadcast fnb = CFundamentalnodeBroadcast(mn);
                    uint256 hash = fnb.GetHash();
                    pfrom->PushInventory(CInv(MSG_FUNDAMENTALNODE_ANNOUNCE, hash));
                    nInvCount++;

                    if (!mapSeenFundamentalnodeBroadcast.count(hash)) mapSeenFundamentalnodeBroadcast.insert(make_pair(hash, fnb));

                    if (vin == mn.vin) {
                        LogPrint("fundamentalnode", "obseg - Sent 1 Fundamentalnode entry to peer %i\n", pfrom->GetId());
                        return;
                    }
                }
            }
        }

        if (vin == CTxIn()) {
            pfrom->PushMessage("ssc", FUNDAMENTALNODE_SYNC_LIST, nInvCount);
            LogPrint("fundamentalnode", "obseg - Sent %d Fundamentalnode entries to peer %i\n", nInvCount, pfrom->GetId());
        }
    }
    /*
     * IT'S SAFE TO REMOVE THIS IN FURTHER VERSIONS
     * AFTER MIGRATION TO V12 IS DONE
     */

    // Light version for OLD MASSTERNODES - fake pings, no self-activation
    else if (strCommand == "obsee") { //ObfuScation Election Entry

        if (IsSporkActive(SPORK_19_FUNDAMENTALNODE_PAY_UPDATED_NODES)) return;

        CTxIn vin;
        CService addr;
        CPubKey pubkey;
        CPubKey pubkey2;
        vector<unsigned char> vchSig;
        int64_t sigTime;
        int count;
        int current;
        int64_t lastUpdated;
        int protocolVersion;
        CScript donationAddress;
        int donationPercentage;
        std::string strMessage;

        vRecv >> vin >> addr >> vchSig >> sigTime >> pubkey >> pubkey2 >> count >> current >> lastUpdated >> protocolVersion >> donationAddress >> donationPercentage;

        // make sure signature isn't in the future (past is OK)
        if (sigTime > GetAdjustedTime() + 60 * 60) {
            LogPrint("fundamentalnode","obsee - Signature rejected, too far into the future %s\n", vin.prevout.hash.ToString());
            Misbehaving(pfrom->GetId(), 1);
            return;
        }

        std::string vchPubKey(pubkey.begin(), pubkey.end());
        std::string vchPubKey2(pubkey2.begin(), pubkey2.end());

        strMessage = addr.ToString() + boost::lexical_cast<std::string>(sigTime) + vchPubKey + vchPubKey2 + boost::lexical_cast<std::string>(protocolVersion) + donationAddress.ToString() + boost::lexical_cast<std::string>(donationPercentage);

        if (protocolVersion < fundamentalnodePayments.GetMinFundamentalnodePaymentsProto()) {
            LogPrint("fundamentalnode","obsee - ignoring outdated Fundamentalnode %s protocol version %d < %d\n", vin.prevout.hash.ToString(), protocolVersion, fundamentalnodePayments.GetMinFundamentalnodePaymentsProto());
            Misbehaving(pfrom->GetId(), 1);
            return;
        }

        CScript pubkeyScript;
        pubkeyScript = GetScriptForDestination(pubkey.GetID());

        if (pubkeyScript.size() != 25) {
            LogPrint("fundamentalnode","obsee - pubkey the wrong size\n");
            Misbehaving(pfrom->GetId(), 100);
            return;
        }

        CScript pubkeyScript2;
        pubkeyScript2 = GetScriptForDestination(pubkey2.GetID());

        if (pubkeyScript2.size() != 25) {
            LogPrint("fundamentalnode","obsee - pubkey2 the wrong size\n");
            Misbehaving(pfrom->GetId(), 100);
            return;
        }

        if (!vin.scriptSig.empty()) {
            LogPrint("fundamentalnode","obsee - Ignore Not Empty ScriptSig %s\n", vin.prevout.hash.ToString());
            Misbehaving(pfrom->GetId(), 100);
            return;
        }

        std::string errorMessage = "";
        if (!obfuScationSigner.VerifyMessage(pubkey, vchSig, strMessage, errorMessage)) {
            LogPrint("fundamentalnode","obsee - Got bad Fundamentalnode address signature\n");
            Misbehaving(pfrom->GetId(), 100);
            return;
        }

        if (Params().NetworkID() == CBaseChainParams::MAIN) {
            if (addr.GetPort() != 8765) return;
        } else if (addr.GetPort() == 8765)
            return;

        //search existing Fundamentalnode list, this is where we update existing Fundamentalnodes with new obsee broadcasts
        CFundamentalnode* pmn = this->Find(vin);
        if (pmn != NULL) {
            // count == -1 when it's a new entry
            //   e.g. We don't want the entry relayed/time updated when we're syncing the list
            // mn.pubkey = pubkey, IsVinAssociatedWithPubkey is validated once below,
            //   after that they just need to match
            if (count == -1 && pmn->pubKeyCollateralAddress == pubkey && (GetAdjustedTime() - pmn->nLastDsee > FUNDAMENTALNODE_MIN_MNB_SECONDS)) {
                if (pmn->protocolVersion > GETHEADERS_VERSION && sigTime - pmn->lastPing.sigTime < FUNDAMENTALNODE_MIN_MNB_SECONDS) return;
                if (pmn->nLastDsee < sigTime) { //take the newest entry
                    LogPrint("fundamentalnode", "obsee - Got updated entry for %s\n", vin.prevout.hash.ToString());
                    if (pmn->protocolVersion < GETHEADERS_VERSION) {
                        pmn->pubKeyFundamentalnode = pubkey2;
                        pmn->sigTime = sigTime;
                        pmn->sig = vchSig;
                        pmn->protocolVersion = protocolVersion;
                        pmn->addr = addr;
                        //fake ping
                        pmn->lastPing = CFundamentalnodePing(vin);
                    }
                    pmn->nLastDsee = sigTime;
                    pmn->Check();
                    if (pmn->IsEnabled()) {
                        TRY_LOCK(cs_vNodes, lockNodes);
                        if (!lockNodes) return;
                        BOOST_FOREACH (CNode* pnode, vNodes)
                            if (pnode->nVersion >= fundamentalnodePayments.GetMinFundamentalnodePaymentsProto())
                                pnode->PushMessage("obsee", vin, addr, vchSig, sigTime, pubkey, pubkey2, count, current, lastUpdated, protocolVersion, donationAddress, donationPercentage);
                    }
                }
            }

            return;
        }

        static std::map<COutPoint, CPubKey> mapSeenobsee;
        if (mapSeenobsee.count(vin.prevout) && mapSeenobsee[vin.prevout] == pubkey) {
            LogPrint("fundamentalnode", "obsee - already seen this vin %s\n", vin.prevout.ToString());
            return;
        }
        mapSeenobsee.insert(make_pair(vin.prevout, pubkey));
        // make sure the vout that was signed is related to the transaction that spawned the Fundamentalnode
        //  - this is expensive, so it's only done once per Fundamentalnode

        uint256 hashBlock = 0;
        CTransaction tx;


        if (!obfuScationSigner.IsVinAssociatedWithPubkey(vin, pubkey, tx, hashBlock )) {
            LogPrint("fundamentalnode","obsee - Got mismatched pubkey and vin\n");
            Misbehaving(pfrom->GetId(), 100);
            return;
        }


        LogPrint("fundamentalnode", "obsee - Got NEW OLD Fundamentalnode entry %s\n", vin.prevout.hash.ToString());

        // make sure it's still unspent
        //  - this is checked later by .check() in many places and by ThreadCheckObfuScationPool()

        CValidationState state;
        /*CMutableTransaction tx = CMutableTransaction();
        CTxOut vout = CTxOut(9999.99 * COIN, obfuScationPool.collateralPubKey);
        tx.vin.push_back(vin);
        tx.vout.push_back(vout);*/

        bool fAcceptable = false;
        {
            TRY_LOCK(cs_main, lockMain);
            if (!lockMain) return;
            fAcceptable = AcceptableFundamentalTxn(mempool, state, tx);
        }

        if (fAcceptable) {
            if (GetInputAge(vin) < FUNDAMENTALNODE_MIN_CONFIRMATIONS) {
                LogPrint("fundamentalnode","obsee - Input must have least %d confirmations\n", FUNDAMENTALNODE_MIN_CONFIRMATIONS);
                Misbehaving(pfrom->GetId(), 20);
                return;
            }

            // verify that sig time is legit in past
            // should be at least not earlier than block when 1000 VITAE tx got FUNDAMENTALNODE_MIN_CONFIRMATIONS
            uint256 hashBlock = 0;
            CTransaction tx2;
            GetTransaction(vin.prevout.hash, tx2, hashBlock, true);
            BlockMap::iterator mi = mapBlockIndex.find(hashBlock);
            if (mi != mapBlockIndex.end() && (*mi).second) {
                CBlockIndex* pMNIndex = (*mi).second;                                                        // block for 10000 VITAE tx -> 1 confirmation
                CBlockIndex* pConfIndex = chainActive[pMNIndex->nHeight + FUNDAMENTALNODE_MIN_CONFIRMATIONS - 1]; // block where tx got FUNDAMENTALNODE_MIN_CONFIRMATIONS
                if (pConfIndex->GetBlockTime() > sigTime) {
                    LogPrint("fundamentalnode","fnb - Bad sigTime %d for Fundamentalnode %s (%i conf block is at %d)\n",
                        sigTime, vin.prevout.hash.ToString(), FUNDAMENTALNODE_MIN_CONFIRMATIONS, pConfIndex->GetBlockTime());
                    return;
                }
            }

            // use this as a peer
            addrman.Add(CAddress(addr), pfrom->addr, 2 * 60 * 60);

            // add Fundamentalnode
            CFundamentalnode mn = CFundamentalnode();
            mn.addr = addr;
            mn.vin = vin;
            mn.pubKeyCollateralAddress = pubkey;
            mn.sig = vchSig;
            mn.sigTime = sigTime;
            mn.pubKeyFundamentalnode = pubkey2;
            mn.protocolVersion = protocolVersion;
            // fake ping
            mn.lastPing = CFundamentalnodePing(vin);
            mn.Check(true);
            // add v11 fundamentalnodes, v12 should be added by fnb only
            if (protocolVersion < GETHEADERS_VERSION) {
                LogPrint("fundamentalnode", "obsee - Accepted OLD Fundamentalnode entry %i %i\n", count, current);
                Add(mn);
            }
            if (mn.IsEnabled()) {
                TRY_LOCK(cs_vNodes, lockNodes);
                if (!lockNodes) return;
                BOOST_FOREACH (CNode* pnode, vNodes)
                    if (pnode->nVersion >= fundamentalnodePayments.GetMinFundamentalnodePaymentsProto())
                        pnode->PushMessage("obsee", vin, addr, vchSig, sigTime, pubkey, pubkey2, count, current, lastUpdated, protocolVersion, donationAddress, donationPercentage);
            }
        } else {
            LogPrint("fundamentalnode","obsee - Rejected Fundamentalnode entry %s\n", vin.prevout.hash.ToString());

            int nDoS = 0;
            if (state.IsInvalid(nDoS)) {
                LogPrint("fundamentalnode","obsee - %s from %i %s was not accepted into the memory pool\n", tx.GetHash().ToString().c_str(),
                    pfrom->GetId(), pfrom->cleanSubVer.c_str());
                if (nDoS > 0)
                    Misbehaving(pfrom->GetId(), nDoS);
            }
        }
    }

    else if (strCommand == "obseep") { //ObfuScation Election Entry Ping

        if (IsSporkActive(SPORK_19_FUNDAMENTALNODE_PAY_UPDATED_NODES)) return;

        CTxIn vin;
        vector<unsigned char> vchSig;
        int64_t sigTime;
        bool stop;
        vRecv >> vin >> vchSig >> sigTime >> stop;

        //LogPrint("fundamentalnode","obseep - Received: vin: %s sigTime: %lld stop: %s\n", vin.ToString().c_str(), sigTime, stop ? "true" : "false");

        if (sigTime > GetAdjustedTime() + 60 * 60) {
            LogPrint("fundamentalnode","obseep - Signature rejected, too far into the future %s\n", vin.prevout.hash.ToString());
            Misbehaving(pfrom->GetId(), 1);
            return;
        }

        if (sigTime <= GetAdjustedTime() - 60 * 60) {
            LogPrint("fundamentalnode","obseep - Signature rejected, too far into the past %s - %d %d \n", vin.prevout.hash.ToString(), sigTime, GetAdjustedTime());
            Misbehaving(pfrom->GetId(), 1);
            return;
        }

        std::map<COutPoint, int64_t>::iterator i = mWeAskedForFundamentalnodeListEntry.find(vin.prevout);
        if (i != mWeAskedForFundamentalnodeListEntry.end()) {
            int64_t t = (*i).second;
            if (GetTime() < t) return; // we've asked recently
        }

        // see if we have this Fundamentalnode
        CFundamentalnode* pmn = this->Find(vin);
        if (pmn != NULL && pmn->protocolVersion >= fundamentalnodePayments.GetMinFundamentalnodePaymentsProto()) {
            // LogPrint("fundamentalnode","obseep - Found corresponding mn for vin: %s\n", vin.ToString().c_str());
            // take this only if it's newer
            if (sigTime - pmn->nLastDseep > FUNDAMENTALNODE_MIN_MNP_SECONDS) {
                std::string strMessage = pmn->addr.ToString() + boost::lexical_cast<std::string>(sigTime) + boost::lexical_cast<std::string>(stop);

                std::string errorMessage = "";
                if (!obfuScationSigner.VerifyMessage(pmn->pubKeyFundamentalnode, vchSig, strMessage, errorMessage)) {
                    LogPrint("fundamentalnode","obseep - Got bad Fundamentalnode address signature %s \n", vin.prevout.hash.ToString());
                    //Misbehaving(pfrom->GetId(), 100);
                    return;
                }

                // fake ping for v11 fundamentalnodes, ignore for v12
                if (pmn->protocolVersion < GETHEADERS_VERSION) pmn->lastPing = CFundamentalnodePing(vin);
                pmn->nLastDseep = sigTime;
                pmn->Check();
                if (pmn->IsEnabled()) {
                    TRY_LOCK(cs_vNodes, lockNodes);
                    if (!lockNodes) return;
                    LogPrint("fundamentalnode", "obseep - relaying %s \n", vin.prevout.hash.ToString());
                    BOOST_FOREACH (CNode* pnode, vNodes)
                        if (pnode->nVersion >= fundamentalnodePayments.GetMinFundamentalnodePaymentsProto())
                            pnode->PushMessage("obseep", vin, vchSig, sigTime, stop);
                }
            }
            return;
        }

        LogPrint("fundamentalnode", "obseep - Couldn't find Fundamentalnode entry %s peer=%i\n", vin.prevout.hash.ToString(), pfrom->GetId());

        AskForMN(pfrom, vin);
    }

    /*
     * END OF "REMOVE"
     */
}

void CFundamentalnodeMan::Remove(CTxIn vin)
{
    LOCK(cs);

    vector<CFundamentalnode>::iterator it = vFundamentalnodes.begin();
    while (it != vFundamentalnodes.end()) {
        if ((*it).vin == vin) {
            LogPrint("fundamentalnode", "CFundamentalnodeMan: Removing Fundamentalnode %s - %i now\n", (*it).vin.prevout.hash.ToString(), size() - 1);
            vFundamentalnodes.erase(it);
            break;
        }
        ++it;
    }
}

void CFundamentalnodeMan::UpdateFundamentalnodeList(CFundamentalnodeBroadcast fnb)
{
    mapSeenFundamentalnodePing.insert(std::make_pair(fnb.lastPing.GetHash(), fnb.lastPing));
    mapSeenFundamentalnodeBroadcast.insert(std::make_pair(fnb.GetHash(), fnb));
	fundamentalnodeSync.AddedFundamentalnodeList(fnb.GetHash());

    LogPrint("fundamentalnode","CFundamentalnodeMan::UpdateFundamentalnodeList() -- fundamentalnode=%s\n", fnb.vin.prevout.ToString());

    CFundamentalnode* pmn = Find(fnb.vin);
    if (pmn == NULL) {
        CFundamentalnode mn(fnb);
        Add(mn);
    } else {
        pmn->UpdateFromNewBroadcast(fnb);
    }
}

std::string CFundamentalnodeMan::ToString() const
{
    std::ostringstream info;

    info << "Fundamentalnodes: " << (int)vFundamentalnodes.size() << ", peers who asked us for Fundamentalnode list: " << (int)mAskedUsForFundamentalnodeList.size() << ", peers we asked for Fundamentalnode list: " << (int)mWeAskedForFundamentalnodeList.size() << ", entries in Fundamentalnode list we asked for: " << (int)mWeAskedForFundamentalnodeListEntry.size() << ", nDsqCount: " << (int)nDsqCount;

    return info.str();
}
