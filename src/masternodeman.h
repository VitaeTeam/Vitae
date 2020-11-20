// Copyright (c) 2014-2015 The Bitsend developers
// Copyright (c) 2014-2015 The Dash developers
// Copyright (c) 2015-2019 The PIVX developers
// Copyright (c) 2018 The VITAE developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef MASTERNODEMAN_H
#define MASTERNODEMAN_H

#include "activemasternode.h"
//#include "bignum.h"
#include "sync.h"
#include "net.h"
#include "key.h"
//#include "core.h"
#include "util.h"
#include "script/script.h"
#include "base58.h"
#include "main.h"
#include "masternode.h"

#define MASTERNODES_DUMP_SECONDS               (15*60)
#define MASTERNODES_DSEG_SECONDS               (3*60*60)

class CMasternodeMan;
class CActiveMasternode;

extern CMasternodeMan m_nodeman;
extern CActiveMasternode activeMasternode;
extern std::string strMasterNodePrivKey;

void DumpMasternodes();

/** Access to the MN database (mncache.dat)
 */
class CMasternodeDB
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

    CMasternodeDB();
    bool Write(const CMasternodeMan &m_nodemanToSave);
    ReadResult Read(CMasternodeMan& m_nodemanToLoad);
};

class CMasternodeMan
{
private:
    // critical section to protect the inner data structures
    mutable RecursiveMutex cs;

    // critical section to protect the inner data structures specifically on messaging
    mutable RecursiveMutex cs_process_message;

    // map to hold all MNs
    std::vector<CMasternode> vMasternodes;
    // who's asked for the Masternode list and the last time
    std::map<CNetAddr, int64_t> mAskedUsForMasternodeList;
    // who we asked for the Masternode list and the last time
    std::map<CNetAddr, int64_t> mWeAskedForMasternodeList;
    // which Masternodes we've asked for
    std::map<COutPoint, int64_t> mWeAskedForMasternodeListEntry;

public:
    // keep track of dsq count to prevent masternodes from gaming queue
    int64_t nDsqCount;

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion)
    {
        // serialized format:
        // * version byte (currently 0)
        // * masternodes vector
        {
                LOCK(cs);
                unsigned char nVersion = 0;
                READWRITE(nVersion);
                READWRITE(vMasternodes);
                READWRITE(mAskedUsForMasternodeList);
                READWRITE(mWeAskedForMasternodeList);
                READWRITE(mWeAskedForMasternodeListEntry);
                READWRITE(nDsqCount);
        }
    }

    CMasternodeMan();
    CMasternodeMan(CMasternodeMan& other);

    /// Add an entry
    bool Add(CMasternode &mn);

    /// Check all Masternodes
    void Check();

    /// Check all Masternodes and remove inactive
    void CheckAndRemove();

    /// Clear Masternode vector
    void Clear();

    int CountEnabled(int protocolVersion = -1);

    void CountNetworks(int protocolVersion, int& ipv4, int& ipv6, int& onion);

    int CountMasternodesAboveProtocol(int protocolVersion);

    void DsegUpdate(CNode* pnode);

    /// Find an entry
    CMasternode* Find(const CTxIn& vin);
    CMasternode* Find(const CPubKey& pubKeyMasternode);

    /// Find an entry thta do not match every entry provided vector
    CMasternode* FindOldestNotInVec(const std::vector<CTxIn> &vVins, int nMinimumAge, int nMinimumActiveSeconds);

    /// Find a random entry
    CMasternode* FindRandom();

    /// Get the current winner for this block
    CMasternode* GetCurrentMasterNode(int mod=1, int64_t nBlockHeight=0, int minProtocol=0);

    std::vector<CMasternode> GetFullMasternodeVector() { Check(); return vMasternodes; }

    std::vector<std::pair<int, CMasternode> > GetMasternodeRanks(int64_t nBlockHeight, int minProtocol=0);
    int GetMasternodeRank(const CTxIn &vin, int64_t nBlockHeight, int minProtocol=0, bool fOnlyActive=true);
    CMasternode* GetMasternodeByRank(int nRank, int64_t nBlockHeight, int minProtocol=0, bool fOnlyActive=true);

    void ProcessMessage(CNode* pfrom, std::string& strCommand, CDataStream& vRecv);

    /// Return the number of (unique) Masternodes
    int size() { return vMasternodes.size(); }

    /// Return the number of masternodes older than (default) 8000 seconds
    int stable_size ();

    std::string ToString() const;

    //
    // Relay Masternode Messages
    //

    void RelayMasternodeEntry(const CTxIn vin, const CService addr, const std::vector<unsigned char> vchSig, const int64_t nNow, const CPubKey pubkey, const CPubKey pubkey2, const int count, const int current, const int64_t lastUpdated, const int protocolVersion, CScript donationAddress, int donationPercentage);
    void RelayMasternodeEntryPing(const CTxIn vin, const std::vector<unsigned char> vchSig, const int64_t nNow, const bool stop);

    void Remove(CTxIn vin);

	// Get Masternode Protocol Version
	int GetMinMasternodePaymentsProto();
};

void ThreadCheckObfuScationPool();

#endif
