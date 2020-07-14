// Copyright (c) 2014-2016 The Dash developers
// Copyright (c) 2015-2017 The PIVX developers
// Copyright (c) 2018 The VITAE developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef ACTIVEFUNDAMENTALNODE_H
#define ACTIVEFUNDAMENTALNODE_H

#include "init.h"
#include "key.h"
#include "fundamentalnode.h"
#include "net.h"
#include "obfuscation.h"
#include "sync.h"
#include "wallet.h"

#define ACTIVE_FUNDAMENTALNODE_INITIAL 0 // initial state
#define ACTIVE_FUNDAMENTALNODE_SYNC_IN_PROCESS 1
#define ACTIVE_FUNDAMENTALNODE_INPUT_TOO_NEW 2
#define ACTIVE_FUNDAMENTALNODE_NOT_CAPABLE 3
#define ACTIVE_FUNDAMENTALNODE_STARTED 4
#define ACTIVE_FUNDAMENTALNODE_NEW_NODE_DISABLED 5

// Responsible for activating the Fundamentalnode and pinging the network
class CActiveFundamentalnode
{
private:
    // critical section to protect the inner data structures
    mutable CCriticalSection cs;

    /// Ping Fundamentalnode
    bool SendFundamentalnodePing(std::string& errorMessage);

    /// Create Fundamentalnode broadcast, needs to be relayed manually after that
    bool CreateBroadcast(CTxIn vin, CService service, CKey key, CPubKey pubKey, CKey keyFundamentalnode, CPubKey pubKeyFundamentalnode, std::string& errorMessage, CFundamentalnodeBroadcast &mnb);

    /// Get 10000 VITAE input that can be used for the Fundamentalnode
    bool GetFundamentalNodeVin(CTxIn& vin, CPubKey& pubkey, CKey& secretKey, std::string strTxHash, std::string strOutputIndex);
    bool GetVinFromOutput(COutput out, CTxIn& vin, CPubKey& pubkey, CKey& secretKey);

public:
    // Initialized by init.cpp
    // Keys for the main Fundamentalnode
    CPubKey pubKeyFundamentalnode;

    // Initialized while registering Fundamentalnode
    CTxIn vin;
    CService service;

    int status;
    std::string notCapableReason;

    CActiveFundamentalnode()
    {
        status = ACTIVE_FUNDAMENTALNODE_INITIAL;
    }

    /// Manage status of main Fundamentalnode
    void ManageStatus();
    std::string GetStatus();

    /// Create Fundamentalnode broadcast, needs to be relayed manually after that
    bool CreateBroadcast(std::string strService, std::string strKey, std::string strTxHash, std::string strOutputIndex, std::string& errorMessage, CFundamentalnodeBroadcast &mnb, bool fOffline = false);

    /// Get 10000 VITAE input that can be used for the Fundamentalnode
    bool GetFundamentalNodeVin(CTxIn& vin, CPubKey& pubkey, CKey& secretKey);
    vector<COutput> SelectCoinsFundamentalnode();

    /// Enable cold wallet mode (run a Fundamentalnode with no funds)
    bool EnableHotColdFundamentalNode(CTxIn& vin, CService& addr);
};

#endif
