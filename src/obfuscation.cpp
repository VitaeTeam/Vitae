// Copyright (c) 2014-2015 The Dash developers
// Copyright (c) 2014-2015 The Dash developers
// Copyright (c) 2015-2017 The PIVX developers
// Copyright (c) 2018 The VITAE developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "obfuscation.h"
#include "coincontrol.h"
#include "init.h"
#include "main.h"
#include "fundamentalnodeman.h"
#include "messagesigner.h"
#include "script/sign.h"
#include "swifttx.h"
#include "ui_interface.h"
#include "util.h"
#include <boost/algorithm/string/replace.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/foreach.hpp>

#include <algorithm>
#include <boost/assign/list_of.hpp>
#include <openssl/rand.h>


// The main object for accessing Obfuscation
CObfuscationPool obfuScationPool;
// The current Obfuscations in progress on the network
std::vector<CObfuscationQueue> vecObfuscationQueue;
// Keep track of the used Fundamentalnodes
std::vector<CTxIn> vecFundamentalnodesUsed;
// Keep track of the scanning errors I've seen
std::map<uint256, CObfuscationBroadcastTx> mapObfuscationBroadcastTxes;
// Keep track of the active Fundamentalnode
CActiveFundamentalnode activeFundamentalnode;

int randomizeList(int i) { return std::rand() % i; }

void CObfuscationPool::Reset()
{
    cachedLastSuccess = 0;
    lastNewBlock = 0;
    vecFundamentalnodesUsed.clear();
    UnlockCoins();
    SetNull();
}

void CObfuscationPool::SetNull()
{
    // MN side
    sessionUsers = 0;
    vecSessionCollateral.clear();

    // Client side
    entriesCount = 0;
    lastEntryAccepted = 0;
    countEntriesAccepted = 0;

    // Both sides
    state = POOL_STATUS_IDLE;
    sessionID = 0;
    sessionDenom = 0;
    entries.clear();
    finalTransaction.vin.clear();
    finalTransaction.vout.clear();
    lastTimeChanged = GetTimeMillis();

    // -- seed random number generator (used for ordering output lists)
    unsigned int seed = 0;
    RAND_bytes((unsigned char*)&seed, sizeof(seed));
    std::srand(seed);
}

bool CObfuscationPool::SetCollateralAddress(std::string strAddress)
{
    CBitcoinAddress address;
    if (!address.SetString(strAddress)) {
        LogPrintf("CObfuscationPool::SetCollateralAddress - Invalid Obfuscation collateral address\n");
        return false;
    }
    collateralPubKey = GetScriptForDestination(address.Get());
    return true;
}

//
// Unlock coins after Obfuscation fails or succeeds
//
void CObfuscationPool::UnlockCoins()
{
    if (!pwalletMain)
        return;

    while (true) {
        TRY_LOCK(pwalletMain->cs_wallet, lockWallet);
        if (!lockWallet) {
            MilliSleep(50);
            continue;
        }
        for (CTxIn v : lockedCoins)
            pwalletMain->UnlockCoin(v.prevout);
        break;
    }

    lockedCoins.clear();
}

//
// Check the Obfuscation progress and send client updates if a Fundamentalnode
//
void CObfuscationPool::Check()
{
    if (fFundamentalNode) LogPrint("obfuscation", "CObfuscationPool::Check() - entries count %lu\n", entries.size());
    //printf("CObfuscationPool::Check() %d - %d - %d\n", state, anonTx.CountEntries(), GetTimeMillis()-lastTimeChanged);

    if (fFundamentalNode) {
        LogPrint("obfuscation", "CObfuscationPool::Check() - entries count %lu\n", entries.size());

        // If entries is full, then move on to the next phase
        if (state == POOL_STATUS_ACCEPTING_ENTRIES && (int)entries.size() >= GetMaxPoolTransactions()) {
            LogPrint("obfuscation", "CObfuscationPool::Check() -- TRYING TRANSACTION \n");
            UpdateState(POOL_STATUS_FINALIZE_TRANSACTION);
        }
    }

    // create the finalized transaction for distribution to the clients
    if (state == POOL_STATUS_FINALIZE_TRANSACTION) {
        LogPrint("obfuscation", "CObfuscationPool::Check() -- FINALIZE TRANSACTIONS\n");
        UpdateState(POOL_STATUS_SIGNING);

        if (fFundamentalNode) {
            CMutableTransaction txNew;

            // make our new transaction
            for (unsigned int i = 0; i < entries.size(); i++) {
                for (const CTxOut& v : entries[i].vout)
                    txNew.vout.push_back(v);

                for (const CTxDSIn& s : entries[i].sev)
                    txNew.vin.push_back(s);
            }

            // shuffle the outputs for improved anonymity
            std::random_shuffle(txNew.vin.begin(), txNew.vin.end(), randomizeList);
            std::random_shuffle(txNew.vout.begin(), txNew.vout.end(), randomizeList);


            LogPrint("obfuscation", "Transaction 1: %s\n", txNew.ToString());
            finalTransaction = txNew;

            // request signatures from clients
            RelayFinalTransaction(sessionID, finalTransaction);
        }
    }

    // If we have all of the signatures, try to compile the transaction
    if (fFundamentalNode && state == POOL_STATUS_SIGNING && SignaturesComplete()) {
        LogPrint("obfuscation", "CObfuscationPool::Check() -- SIGNING\n");
        UpdateState(POOL_STATUS_TRANSMISSION);

        CheckFinalTransaction();
    }

    // reset if we're here for 10 seconds
    if ((state == POOL_STATUS_ERROR || state == POOL_STATUS_SUCCESS) && GetTimeMillis() - lastTimeChanged >= 10000) {
        LogPrint("obfuscation", "CObfuscationPool::Check() -- timeout, RESETTING\n");
        UnlockCoins();
        SetNull();
        if (fFundamentalNode) RelayStatus(sessionID, GetState(), GetEntriesCount(), FUNDAMENTALNODE_RESET);
    }
}

void CObfuscationPool::CheckFinalTransaction()
{
    if (!fFundamentalNode) return; // check and relay final tx only on fundamentalnode

    CWalletTx txNew = CWalletTx(pwalletMain, finalTransaction);

    LOCK2(cs_main, pwalletMain->cs_wallet);
    {
        LogPrint("obfuscation", "Transaction 2: %s\n", txNew.ToString());

        // See if the transaction is valid
        if (!txNew.AcceptToMemoryPool(false, true, true)) {
            LogPrintf("CObfuscationPool::Check() - CommitTransaction : Error: Transaction not valid\n");
            SetNull();

            // not much we can do in this case
            UpdateState(POOL_STATUS_ACCEPTING_ENTRIES);
            RelayCompletedTransaction(sessionID, true, ERR_INVALID_TX);
            return;
        }

        LogPrintf("CObfuscationPool::Check() -- IS MASTER -- TRANSMITTING OBFUSCATION\n");

        // sign a message

        int64_t sigTime = GetAdjustedTime();
        std::string strMessage = txNew.GetHash().ToString() + std::to_string(sigTime);
        std::string strError = "";
        std::vector<unsigned char> vchSig;
        CKey key2;
        CPubKey pubkey2;

        if (!CMessageSigner::GetKeysFromSecret(strFundamentalNodePrivKey, key2, pubkey2)) {
            LogPrintf("%s : Invalid Fundamentalnodeprivkey key %s", __func__, strFundamentalNodePrivKey);
            return;
        }

        if (!CMessageSigner::SignMessage(strMessage, vchSig, key2)) {
            LogPrintf("%s : Sign message failed", __func__);
            return;
        }

        if (!CMessageSigner::VerifyMessage(pubkey2, vchSig, strMessage, strError)) {
            LogPrintf("%s : Verify message failed, error: %s", __func__, strError);
            return;
        }

        if (!mapObfuscationBroadcastTxes.count(txNew.GetHash())) {
            CObfuscationBroadcastTx dstx;
            dstx.tx = txNew;
            dstx.vin = activeFundamentalnode.vin;
            dstx.vchSig = vchSig;
            dstx.sigTime = sigTime;

            mapObfuscationBroadcastTxes.insert(std::make_pair(txNew.GetHash(), dstx));
        }

        CInv inv(MSG_DSTX, txNew.GetHash());
        RelayInv(inv);

        // Tell the clients it was successful
        RelayCompletedTransaction(sessionID, false, MSG_SUCCESS);

        // Randomly charge clients
        ChargeRandomFees();

        // Reset
        LogPrint("obfuscation", "CObfuscationPool::Check() -- COMPLETED -- RESETTING\n");
        SetNull();
        RelayStatus(sessionID, GetState(), GetEntriesCount(), FUNDAMENTALNODE_RESET);
    }
}

//
// Charge clients a fee if they're abusive
//
// Why bother? Obfuscation uses collateral to ensure abuse to the process is kept to a minimum.
// The submission and signing stages in Obfuscation are completely separate. In the cases where
// a client submits a transaction then refused to sign, there must be a cost. Otherwise they
// would be able to do this over and over again and bring the mixing to a hault.
//
// How does this work? Messages to Fundamentalnodes come in via "dsi", these require a valid collateral
// transaction for the client to be able to enter the pool. This transaction is kept by the Fundamentalnode
// until the transaction is either complete or fails.
//
void CObfuscationPool::ChargeFees()
{
    if (!fFundamentalNode) return;

    //we don't need to charge collateral for every offence.
    int offences = 0;
    int r = rand() % 100;
    if (r > 33) return;

    if (state == POOL_STATUS_ACCEPTING_ENTRIES) {
        for (const CTransaction& txCollateral : vecSessionCollateral) {
            bool found = false;
            for (const CObfuScationEntry& v : entries) {
                if (v.collateral == txCollateral) {
                    found = true;
                }
            }

            // This queue entry didn't send us the promised transaction
            if (!found) {
                LogPrintf("CObfuscationPool::ChargeFees -- found uncooperative node (didn't send transaction). Found offence.\n");
                offences++;
            }
        }
    }

    if (state == POOL_STATUS_SIGNING) {
        // who didn't sign?
        for (const CObfuScationEntry &v : entries) {
            for (const CTxDSIn &s : v.sev) {
                if (!s.fHasSig) {
                    LogPrintf("CObfuscationPool::ChargeFees -- found uncooperative node (didn't sign). Found offence\n");
                    offences++;
                }
            }
        }
    }

    r = rand() % 100;
    int target = 0;

    //mostly offending?
    if (offences >= Params().PoolMaxTransactions() - 1 && r > 33) return;

    //everyone is an offender? That's not right
    if (offences >= Params().PoolMaxTransactions()) return;

    //charge one of the offenders randomly
    if (offences > 1) target = 50;

    //pick random client to charge
    r = rand() % 100;

    if (state == POOL_STATUS_ACCEPTING_ENTRIES) {
        for (const CTransaction& txCollateral : vecSessionCollateral) {
            bool found = false;
            for (const CObfuScationEntry& v : entries) {
                if (v.collateral == txCollateral) {
                    found = true;
                }
            }

            // This queue entry didn't send us the promised transaction
            if (!found && r > target) {
                LogPrintf("CObfuscationPool::ChargeFees -- found uncooperative node (didn't send transaction). charging fees.\n");

                CWalletTx wtxCollateral = CWalletTx(pwalletMain, txCollateral);

                // Broadcast
                if (!wtxCollateral.AcceptToMemoryPool(true)) {
                    // This must not fail. The transaction has already been signed and recorded.
                    LogPrintf("CObfuscationPool::ChargeFees() : Error: Transaction not valid");
                }
                wtxCollateral.RelayWalletTransaction();
                return;
            }
        }
    }

    if (state == POOL_STATUS_SIGNING) {
        // who didn't sign?
        for (const CObfuScationEntry &v : entries) {
            for (const CTxDSIn &s : v.sev) {
                if (!s.fHasSig && r > target) {
                    LogPrintf("CObfuscationPool::ChargeFees -- found uncooperative node (didn't sign). charging fees.\n");

                    CWalletTx wtxCollateral = CWalletTx(pwalletMain, v.collateral);

                    // Broadcast
                    if (!wtxCollateral.AcceptToMemoryPool(false)) {
                        // This must not fail. The transaction has already been signed and recorded.
                        LogPrintf("CObfuscationPool::ChargeFees() : Error: Transaction not valid");
                    }
                    wtxCollateral.RelayWalletTransaction();
                    return;
                }
            }
        }
    }
}

// charge the collateral randomly
//  - Obfuscation is completely free, to pay miners we randomly pay the collateral of users.
void CObfuscationPool::ChargeRandomFees()
{
    if (fFundamentalNode) {
        int i = 0;

        for (const CTransaction& txCollateral : vecSessionCollateral) {
            int r = rand() % 100;

            /*
                Collateral Fee Charges:

                Being that Obfuscation has "no fees" we need to have some kind of cost associated
                with using it to stop abuse. Otherwise it could serve as an attack vector and
                allow endless transaction that would bloat VITAE and make it unusable. To
                stop these kinds of attacks 1 in 10 successful transactions are charged. This
                adds up to a cost of 0.001 VITAE per transaction on average.
            */
            if (r <= 10) {
                LogPrintf("CObfuscationPool::ChargeRandomFees -- charging random fees. %u\n", i);

                CWalletTx wtxCollateral = CWalletTx(pwalletMain, txCollateral);

                // Broadcast
                if (!wtxCollateral.AcceptToMemoryPool(true)) {
                    // This must not fail. The transaction has already been signed and recorded.
                    LogPrintf("CObfuscationPool::ChargeRandomFees() : Error: Transaction not valid");
                }
                wtxCollateral.RelayWalletTransaction();
            }
        }
    }
}

//
// Check for various timeouts (queue objects, Obfuscation, etc)
//
void CObfuscationPool::CheckTimeout()
{
    if (!fEnableZeromint && !fFundamentalNode) return;

    // catching hanging sessions
    if (!fFundamentalNode) {
        switch (state) {
        case POOL_STATUS_TRANSMISSION:
            LogPrint("obfuscation", "CObfuscationPool::CheckTimeout() -- Session complete -- Running Check()\n");
            Check();
            break;
        case POOL_STATUS_ERROR:
            LogPrint("obfuscation", "CObfuscationPool::CheckTimeout() -- Pool error -- Running Check()\n");
            Check();
            break;
        case POOL_STATUS_SUCCESS:
            LogPrint("obfuscation", "CObfuscationPool::CheckTimeout() -- Pool success -- Running Check()\n");
            Check();
            break;
        }
    }

    // check Obfuscation queue objects for timeouts
    int c = 0;
    std::vector<CObfuscationQueue>::iterator it = vecObfuscationQueue.begin();
    while (it != vecObfuscationQueue.end()) {
        if ((*it).IsExpired()) {
            LogPrint("obfuscation", "CObfuscationPool::CheckTimeout() : Removing expired queue entry - %d\n", c);
            it = vecObfuscationQueue.erase(it);
        } else
            ++it;
        c++;
    }

    int addLagTime = 0;
    if (!fFundamentalNode) addLagTime = 10000; //if we're the client, give the server a few extra seconds before resetting.

    if (state == POOL_STATUS_ACCEPTING_ENTRIES || state == POOL_STATUS_QUEUE) {
        c = 0;

        // check for a timeout and reset if needed
        std::vector<CObfuScationEntry>::iterator it2 = entries.begin();
        while (it2 != entries.end()) {
            if ((*it2).IsExpired()) {
                LogPrint("obfuscation", "CObfuscationPool::CheckTimeout() : Removing expired entry - %d\n", c);
                it2 = entries.erase(it2);
                if (entries.size() == 0) {
                    UnlockCoins();
                    SetNull();
                }
                if (fFundamentalNode) {
                    RelayStatus(sessionID, GetState(), GetEntriesCount(), FUNDAMENTALNODE_RESET);
                }
            } else
                ++it2;
            c++;
        }

        if (GetTimeMillis() - lastTimeChanged >= (OBFUSCATION_QUEUE_TIMEOUT * 1000) + addLagTime) {
            UnlockCoins();
            SetNull();
        }
    } else if (GetTimeMillis() - lastTimeChanged >= (OBFUSCATION_QUEUE_TIMEOUT * 1000) + addLagTime) {
        LogPrint("obfuscation", "CObfuscationPool::CheckTimeout() -- Session timed out (%ds) -- resetting\n", OBFUSCATION_QUEUE_TIMEOUT);
        UnlockCoins();
        SetNull();

        UpdateState(POOL_STATUS_ERROR);
        lastMessage = _("Session timed out.");
    }

    if (state == POOL_STATUS_SIGNING && GetTimeMillis() - lastTimeChanged >= (OBFUSCATION_SIGNING_TIMEOUT * 1000) + addLagTime) {
        LogPrint("obfuscation", "CObfuscationPool::CheckTimeout() -- Session timed out (%ds) -- restting\n", OBFUSCATION_SIGNING_TIMEOUT);
        ChargeFees();
        UnlockCoins();
        SetNull();

        UpdateState(POOL_STATUS_ERROR);
        lastMessage = _("Signing timed out.");
    }
}

//
// Check for complete queue
//
void CObfuscationPool::CheckForCompleteQueue()
{
    if (!fEnableZeromint && !fFundamentalNode) return;

    /* Check to see if we're ready for submissions from clients */
    //
    // After receiving multiple dsa messages, the queue will switch to "accepting entries"
    // which is the active state right before merging the transaction
    //
    if (state == POOL_STATUS_QUEUE && sessionUsers == GetMaxPoolTransactions()) {
        UpdateState(POOL_STATUS_ACCEPTING_ENTRIES);

        CObfuscationQueue dsq;
        dsq.nDenom = sessionDenom;
        dsq.vin = activeFundamentalnode.vin;
        dsq.time = GetTime();
        dsq.ready = true;
        dsq.Sign();
        dsq.Relay();
    }
}


// Check to make sure everything is signed
bool CObfuscationPool::SignaturesComplete()
{
    for (const CObfuScationEntry& v : entries) {
        for (const CTxDSIn& s : v.sev) {
            if (!s.fHasSig) return false;
        }
    }
    return true;
}

void CObfuscationPool::NewBlock()
{
    LogPrint("obfuscation", "CObfuscationPool::NewBlock \n");

    //we we're processing lots of blocks, we'll just leave
    if (GetTime() - lastNewBlock < 10) return;
    lastNewBlock = GetTime();

    obfuScationPool.CheckTimeout();
}

bool CObfuscationQueue::Sign()
{
    if (!fFundamentalNode) return false;

    std::string strError = "";
    std::string strMessage = vin.ToString() + std::to_string(nDenom) + std::to_string(time) + std::to_string(ready);

    CKey key2;
    CPubKey pubkey2;

    if (!CMessageSigner::GetKeysFromSecret(strFundamentalNodePrivKey, key2, pubkey2)) {
        return error("%s : Invalid masternode key", __func__);
    }

    if (!CMessageSigner::SignMessage(strMessage, vchSig, key2)) {
        return error("%s : Sign message failed", __func__);
    }

    if (!CMessageSigner::VerifyMessage(pubkey2, vchSig, strMessage, strError)) {
        return error("%s : Verify message failed, error: %s", __func__, strError);
    }

    return true;
}

bool CObfuscationQueue::Relay()
{
    LOCK(cs_vNodes);
    for (CNode* pnode : vNodes) {
        // always relay to everyone
        pnode->PushMessage("dsq", (*this));
    }

    return true;
}

void CObfuscationPool::RelayFinalTransaction(const int sessionID, const CTransaction& txNew)
{
    LOCK(cs_vNodes);
    for (CNode* pnode : vNodes) {
        pnode->PushMessage("dsf", sessionID, txNew);
    }
}

void CObfuscationPool::RelayStatus(const int sessionID, const int newState, const int newEntriesCount, const int newAccepted, const int errorID)
{
    LOCK(cs_vNodes);
    for (CNode* pnode : vNodes)
        pnode->PushMessage("dssu", sessionID, newState, newEntriesCount, newAccepted, errorID);
}

void CObfuscationPool::RelayCompletedTransaction(const int sessionID, const bool error, const int errorID)
{
    LOCK(cs_vNodes);
    for (CNode* pnode : vNodes)
        pnode->PushMessage("dsc", sessionID, error, errorID);
}

//TODO: Rename/move to core
void ThreadCheckObfuScationPool()
{
    if (fLiteMode) return; //disable all Obfuscation/Fundamentalnode related functionality

    // Make this thread recognisable as the wallet flushing thread
    RenameThread("vitae-obfuscation");
    LogPrintf("Masternodes thread started\n");

    unsigned int c = 0;

    while (true) {
        MilliSleep(1000);
        //LogPrintf("ThreadCheckObfuScationPool::check timeout\n");

        // try to sync from all available nodes, one step at a time
        fundamentalnodeSync.Process();

        if (fundamentalnodeSync.IsBlockchainSynced()) {
            c++;

            // check if we should activate or ping every few minutes,
            // start right after sync is considered to be done
            if (c % FUNDAMENTALNODE_PING_SECONDS == 1) activeFundamentalnode.ManageStatus();

            if (c % 60 == 0) {
                mnodeman.CheckAndRemove();
                mnodeman.ProcessFundamentalnodeConnections();
                fundamentalnodePayments.CleanPaymentList();
                CleanTransactionLocksList();
            }

            //if(c % FUNDAMENTALNODES_DUMP_SECONDS == 0) DumpFundamentalnodes();

            obfuScationPool.CheckTimeout();
            obfuScationPool.CheckForCompleteQueue();
        }
    }
}
