// Copyright (c) 2014-2015 The Dash developers
// Copyright (c) 2015-2017 The PIVX developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef FUNDAMENTALNODE_H
#define FUNDAMENTALNODE_H

#include "base58.h"
#include "key.h"
#include "main.h"
#include "net.h"
#include "sync.h"
#include "timedata.h"
#include "util.h"

#define FUNDAMENTALNODE_MIN_CONFIRMATIONS 15
#define FUNDAMENTALNODE_MIN_MNP_SECONDS (10 * 60)
#define FUNDAMENTALNODE_MIN_MNB_SECONDS (5 * 60)
#define FUNDAMENTALNODE_PING_SECONDS (5 * 60)
#define FUNDAMENTALNODE_EXPIRATION_SECONDS (120 * 60)
#define FUNDAMENTALNODE_REMOVAL_SECONDS (130 * 60)
#define FUNDAMENTALNODE_CHECK_SECONDS 5

static const CAmount FUNDAMENTALNODE_AMOUNT = 10000* COIN;
static const CAmount FN_MAGIC_AMOUNT = 0.1234 *COIN;

class CFundamentalnode;
class CFundamentalnodeBroadcast;
class CFundamentalnodePing;
extern std::map<int64_t, uint256> mapCacheBlockHashes;

bool GetBlockHash(uint256& hash, int nBlockHeight);


//
// The Fundamentalnode Ping Class : Contains a different serialize method for sending pings from fundamentalnodes throughout the network
//

class CFundamentalnodePing
{
public:
    CTxIn vin;
    uint256 blockHash;
    int64_t sigTime; //mnb message times
    std::vector<unsigned char> vchSig;
    //removed stop

    CFundamentalnodePing();
    CFundamentalnodePing(CTxIn& newVin);

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion)
    {
        READWRITE(vin);
        READWRITE(blockHash);
        READWRITE(sigTime);
        READWRITE(vchSig);
    }

    bool CheckAndUpdate(int& nDos, bool fRequireEnabled = true, bool fCheckSigTimeOnly = false);
    bool Sign(CKey& keyFundamentalnode, CPubKey& pubKeyFundamentalnode);
    bool VerifySignature(CPubKey& pubKeyFundamentalnode, int &nDos);
    void Relay();

    uint256 GetHash()
    {
        CHashWriter ss(SER_GETHASH, PROTOCOL_VERSION);
        ss << vin;
        ss << sigTime;
        return ss.GetHash();
    }

    void swap(CFundamentalnodePing& first, CFundamentalnodePing& second) // nothrow
    {
        // enable ADL (not necessary in our case, but good practice)
        using std::swap;

        // by swapping the members of two classes,
        // the two classes are effectively swapped
        swap(first.vin, second.vin);
        swap(first.blockHash, second.blockHash);
        swap(first.sigTime, second.sigTime);
        swap(first.vchSig, second.vchSig);
    }

    CFundamentalnodePing& operator=(CFundamentalnodePing from)
    {
        swap(*this, from);
        return *this;
    }
    friend bool operator==(const CFundamentalnodePing& a, const CFundamentalnodePing& b)
    {
        return a.vin == b.vin && a.blockHash == b.blockHash;
    }
    friend bool operator!=(const CFundamentalnodePing& a, const CFundamentalnodePing& b)
    {
        return !(a == b);
    }
};

//
// The Fundamentalnode Class. For managing the Obfuscation process. It contains the input of the 10000 VITAE, signature to prove
// it's the one who own that ip address and code for calculating the payment election.
//
class CFundamentalnode
{
private:
    // critical section to protect the inner data structures
    mutable CCriticalSection cs;
    int64_t lastTimeChecked;

public:
    enum state {
        FUNDAMENTALNODE_PRE_ENABLED,
        FUNDAMENTALNODE_ENABLED,
        FUNDAMENTALNODE_EXPIRED,
        FUNDAMENTALNODE_OUTPOINT_SPENT,
        FUNDAMENTALNODE_REMOVE,
        FUNDAMENTALNODE_WATCHDOG_EXPIRED,
        FUNDAMENTALNODE_POSE_BAN,
        FUNDAMENTALNODE_VIN_SPENT,
        FUNDAMENTALNODE_POS_ERROR,
        FUNDAMENTALNODE_MISSING
    };

    CTxIn vin;
    CService addr;
    CPubKey pubKeyCollateralAddress;
    CPubKey pubKeyFundamentalnode;
    CPubKey pubKeyCollateralAddress1;
    CPubKey pubKeyFundamentalnode1;
    std::vector<unsigned char> sig;
    int activeState;
    int64_t sigTime; //mnb message time
    int cacheInputAge;
    int cacheInputAgeBlock;
    bool unitTest;
    bool allowFreeTx;
    int protocolVersion;
    int nActiveState;
    int64_t nLastDsq; //the dsq count from the last dsq broadcast of this node
    int nScanningErrorCount;
    int nLastScanningErrorBlockHeight;
    CFundamentalnodePing lastPing;

    int64_t nLastDsee;  // temporary, do not save. Remove after migration to v12
    int64_t nLastDseep; // temporary, do not save. Remove after migration to v12

    CFundamentalnode();
    CFundamentalnode(const CFundamentalnode& other);
    CFundamentalnode(const CFundamentalnodeBroadcast& mnb);


    void swap(CFundamentalnode& first, CFundamentalnode& second) // nothrow
    {
        // enable ADL (not necessary in our case, but good practice)
        using std::swap;

        // by swapping the members of two classes,
        // the two classes are effectively swapped
        swap(first.vin, second.vin);
        swap(first.addr, second.addr);
        swap(first.pubKeyCollateralAddress, second.pubKeyCollateralAddress);
        swap(first.pubKeyFundamentalnode, second.pubKeyFundamentalnode);
        swap(first.sig, second.sig);
        swap(first.activeState, second.activeState);
        swap(first.sigTime, second.sigTime);
        swap(first.lastPing, second.lastPing);
        swap(first.cacheInputAge, second.cacheInputAge);
        swap(first.cacheInputAgeBlock, second.cacheInputAgeBlock);
        swap(first.unitTest, second.unitTest);
        swap(first.allowFreeTx, second.allowFreeTx);
        swap(first.protocolVersion, second.protocolVersion);
        swap(first.nLastDsq, second.nLastDsq);
        swap(first.nScanningErrorCount, second.nScanningErrorCount);
        swap(first.nLastScanningErrorBlockHeight, second.nLastScanningErrorBlockHeight);
    }

    CFundamentalnode& operator=(CFundamentalnode from)
    {
        swap(*this, from);
        return *this;
    }
    friend bool operator==(const CFundamentalnode& a, const CFundamentalnode& b)
    {
        return a.vin == b.vin;
    }
    friend bool operator!=(const CFundamentalnode& a, const CFundamentalnode& b)
    {
        return !(a.vin == b.vin);
    }

    uint256 CalculateScore(int mod = 1, int64_t nBlockHeight = 0);

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion)
    {
        LOCK(cs);

        READWRITE(vin);
        READWRITE(addr);
        READWRITE(pubKeyCollateralAddress);
        READWRITE(pubKeyFundamentalnode);
        READWRITE(sig);
        READWRITE(sigTime);
        READWRITE(protocolVersion);
        READWRITE(activeState);
        READWRITE(lastPing);
        READWRITE(cacheInputAge);
        READWRITE(cacheInputAgeBlock);
        READWRITE(unitTest);
        READWRITE(allowFreeTx);
        READWRITE(nLastDsq);
        READWRITE(nScanningErrorCount);
        READWRITE(nLastScanningErrorBlockHeight);
    }

    int64_t SecondsSincePayment();

    bool UpdateFromNewBroadcast(CFundamentalnodeBroadcast& mnb);

    inline uint64_t SliceHash(uint256& hash, int slice)
    {
        uint64_t n = 0;
        memcpy(&n, &hash + slice * 64, 64);
        return n;
    }

    void Check(bool forceCheck = false);

    bool IsBroadcastedWithin(int seconds)
    {
        return (GetAdjustedTime() - sigTime) < seconds;
    }

    bool IsPingedWithin(int seconds, int64_t now = -1)
    {
        now == -1 ? now = GetAdjustedTime() : now;

        return (lastPing == CFundamentalnodePing()) ? false : now - lastPing.sigTime < seconds;
    }

    void Disable()
    {
        sigTime = 0;
        lastPing = CFundamentalnodePing();
    }

    bool IsEnabled()
    {
        return activeState == FUNDAMENTALNODE_ENABLED;
    }

    int GetFundamentalnodeInputAge()
    {
        if (chainActive.Tip() == NULL) return 0;

        if (cacheInputAge == 0) {
            cacheInputAge = GetInputAge(vin);
            cacheInputAgeBlock = chainActive.Tip()->nHeight;
        }

        return cacheInputAge + (chainActive.Tip()->nHeight - cacheInputAgeBlock);
    }

    std::string GetStatus();

    std::string Status()
    {
        std::string strStatus = "ACTIVE";

        if (activeState == CFundamentalnode::FUNDAMENTALNODE_ENABLED) strStatus = "ENABLED";
        if (activeState == CFundamentalnode::FUNDAMENTALNODE_EXPIRED) strStatus = "EXPIRED";
        if (activeState == CFundamentalnode::FUNDAMENTALNODE_VIN_SPENT) strStatus = "VIN_SPENT";
        if (activeState == CFundamentalnode::FUNDAMENTALNODE_REMOVE) strStatus = "REMOVE";
        if (activeState == CFundamentalnode::FUNDAMENTALNODE_POS_ERROR) strStatus = "POS_ERROR";
        if (activeState == CFundamentalnode::FUNDAMENTALNODE_POS_ERROR) strStatus = "MISSING";

        return strStatus;
    }

    int64_t GetLastPaid();
    bool IsValidNetAddr();
};


//
// The Fundamentalnode Broadcast Class : Contains a different serialize method for sending fundamentalnodes through the network
//

class CFundamentalnodeBroadcast : public CFundamentalnode
{
public:
    CFundamentalnodeBroadcast();
    CFundamentalnodeBroadcast(CService newAddr, CTxIn newVin, CPubKey newPubkey, CPubKey newPubkey2, int protocolVersionIn);
    CFundamentalnodeBroadcast(const CFundamentalnode& mn);

    bool CheckAndUpdate(int& nDoS);
    bool CheckInputsAndAdd(int& nDos);
    bool Sign(CKey& keyCollateralAddress);
    bool VerifySignature();
    void Relay();
    std::string GetOldStrMessage();
    std::string GetNewStrMessage();

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion)
    {
        READWRITE(vin);
        READWRITE(addr);
        READWRITE(pubKeyCollateralAddress);
        READWRITE(pubKeyFundamentalnode);
        READWRITE(sig);
        READWRITE(sigTime);
        READWRITE(protocolVersion);
        READWRITE(lastPing);
        READWRITE(nLastDsq);
    }

    uint256 GetHash()
    {
        CHashWriter ss(SER_GETHASH, PROTOCOL_VERSION);
        ss << sigTime;
        ss << pubKeyCollateralAddress;
        return ss.GetHash();
    }

    /// Create Fundamentalnode broadcast, needs to be relayed manually after that
    static bool Create(CTxIn vin, CService service, CKey keyCollateralAddressNew, CPubKey pubKeyCollateralAddressNew, CKey keyFundamentalnodeNew, CPubKey pubKeyFundamentalnodeNew, std::string& strErrorRet, CFundamentalnodeBroadcast& mnbRet);
    static bool Create(std::string strService, std::string strKey, std::string strTxHash, std::string strOutputIndex, std::string& strErrorRet, CFundamentalnodeBroadcast& mnbRet, bool fOffline = false);
    static bool CheckDefaultPort(std::string strService, std::string& strErrorRet, std::string strContext);
};

#endif
