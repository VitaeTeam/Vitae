// Copyright (c) 2014-2015 The Dash developers
// Copyright (c) 2015-2017 The PIVX developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef FUNDAMENTALNODEMAN_H
#define FUNDAMENTALNODEMAN_H

#include "base58.h"
#include "key.h"
#include "main.h"
#include "fundamentalnode.h"
#include "net.h"
#include "sync.h"
#include "util.h"

#define FUNDAMENTALNODES_DUMP_SECONDS (15 * 60)
#define FUNDAMENTALNODES_DSEG_SECONDS (3 * 60 * 60)

using namespace std;

class CFundamentalnodeMan;

extern CFundamentalnodeMan mnodeman;
void DumpFundamentalnodes();

/** Access to the MN database (mncache.dat)
 */
class CFundamentalnodeDB
{
private:
    boost::filesystem::path pathMN;
    std::string strMagicMessage;

public:
    enum ReadResult {
        Ok,
        FileError,
        HashReadError,
        IncorrectHash,
        IncorrectMagicMessage,
        IncorrectMagicNumber,
        IncorrectFormat
    };

    CFundamentalnodeDB();
    bool Write(const CFundamentalnodeMan& mnodemanToSave);
    ReadResult Read(CFundamentalnodeMan& mnodemanToLoad, bool fDryRun = false);
};

class CFundamentalnodeMan
{
private:
    // critical section to protect the inner data structures
    mutable CCriticalSection cs;

    // critical section to protect the inner data structures specifically on messaging
    mutable CCriticalSection cs_process_message;

    // map to hold all MNs
    std::vector<CFundamentalnode> vFundamentalnodes;
    // who's asked for the Fundamentalnode list and the last time
    std::map<CNetAddr, int64_t> mAskedUsForFundamentalnodeList;
    // who we asked for the Fundamentalnode list and the last time
    std::map<CNetAddr, int64_t> mWeAskedForFundamentalnodeList;
    // which Fundamentalnodes we've asked for
    std::map<COutPoint, int64_t> mWeAskedForFundamentalnodeListEntry;

public:
    // Keep track of all broadcasts I've seen
    map<uint256, CFundamentalnodeBroadcast> mapSeenFundamentalnodeBroadcast;
    // Keep track of all pings I've seen
    map<uint256, CFundamentalnodePing> mapSeenFundamentalnodePing;

    // keep track of dsq count to prevent fundamentalnodes from gaming obfuscation queue
    int64_t nDsqCount;

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion)
    {
        LOCK(cs);
        READWRITE(vFundamentalnodes);
        READWRITE(mAskedUsForFundamentalnodeList);
        READWRITE(mWeAskedForFundamentalnodeList);
        READWRITE(mWeAskedForFundamentalnodeListEntry);
        READWRITE(nDsqCount);

        READWRITE(mapSeenFundamentalnodeBroadcast);
        READWRITE(mapSeenFundamentalnodePing);
    }

    CFundamentalnodeMan();
    CFundamentalnodeMan(CFundamentalnodeMan& other);

    /// Add an entry
    bool Add(CFundamentalnode& mn);

    /// Ask (source) node for mnb
    void AskForMN(CNode* pnode, CTxIn& vin);

    /// Check all Fundamentalnodes
    void Check();

    /// Check all Fundamentalnodes and remove inactive
    void CheckAndRemove(bool forceExpiredRemoval = false);

    /// Clear Fundamentalnode vector
    void Clear();

    int CountEnabled(int protocolVersion = -1);

    void CountNetworks(int protocolVersion, int& ipv4, int& ipv6, int& onion);

    void DsegUpdate(CNode* pnode);

    /// Find an entry
    CFundamentalnode* Find(const CScript& payee);
    CFundamentalnode* Find(const CTxIn& vin);
    CFundamentalnode* Find(const CPubKey& pubKeyFundamentalnode);

    /// Find an entry in the fundamentalnode list that is next to be paid
    CFundamentalnode* GetNextFundamentalnodeInQueueForPayment(int nBlockHeight, bool fFilterSigTime, int& nCount);

    /// Find a random entry
    CFundamentalnode* FindRandomNotInVec(std::vector<CTxIn>& vecToExclude, int protocolVersion = -1);

    /// Get the current winner for this block
    CFundamentalnode* GetCurrentFundamentalNode(int mod = 1, int64_t nBlockHeight = 0, int minProtocol = 0);

    std::vector<CFundamentalnode> GetFullFundamentalnodeVector()
    {
        Check();
        return vFundamentalnodes;
    }

    std::vector<pair<int, CFundamentalnode> > GetFundamentalnodeRanks(int64_t nBlockHeight, int minProtocol = 0);
    int GetFundamentalnodeRank(const CTxIn& vin, int64_t nBlockHeight, int minProtocol = 0, bool fOnlyActive = true);
    CFundamentalnode* GetFundamentalnodeByRank(int nRank, int64_t nBlockHeight, int minProtocol = 0, bool fOnlyActive = true);

    void ProcessFundamentalnodeConnections();

    void ProcessMessage(CNode* pfrom, std::string& strCommand, CDataStream& vRecv);

    /// Return the number of (unique) Fundamentalnodes
    int size() { return vFundamentalnodes.size(); }

    /// Return the number of Fundamentalnodes older than (default) 8000 seconds
    int stable_size ();

    std::string ToString() const;

    void Remove(CTxIn vin);

    /// Update fundamentalnode list and maps using provided CFundamentalnodeBroadcast
    void UpdateFundamentalnodeList(CFundamentalnodeBroadcast mnb);
};

#endif
