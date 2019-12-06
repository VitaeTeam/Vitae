// Copyright (c) 2014-2015 The Dash developers
// Copyright (c) 2015-2017 The PIVX developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

// clang-format off
#include "main.h"
#include "activefundamentalnode.h"
#include "fundamentalnode-sync.h"
#include "fundamentalnode-payments.h"
#include "fundamentalnode-budget.h"
#include "fundamentalnode.h"
#include "fundamentalnodeman.h"
#include "spork.h"
#include "util.h"
#include "addrman.h"
// clang-format on

class CFundamentalnodeSync;
CFundamentalnodeSync fundamentalnodeSync;

CFundamentalnodeSync::CFundamentalnodeSync()
{
    Reset();
}

bool CFundamentalnodeSync::IsSynced()
{
    return RequestedFundamentalnodeAssets == FUNDAMENTALNODE_SYNC_FINISHED;
}

bool CFundamentalnodeSync::IsBlockchainSynced()
{
    static bool fBlockchainSynced = false;
    static int64_t lastProcess = GetTime();

    // if the last call to this function was more than 60 minutes ago (client was in sleep mode) reset the sync process
    if (GetTime() - lastProcess > 60 * 60) {
        Reset();
        fBlockchainSynced = false;
    }
    lastProcess = GetTime();

    if (fBlockchainSynced) return true;

    if (fImporting || fReindex) return false;

    TRY_LOCK(cs_main, lockMain);
    if (!lockMain) return false;

    CBlockIndex* pindex = chainActive.Tip();
    if (pindex == NULL) return false;


    if (pindex->nTime + 60 * 60 < GetTime())
        return false;

    fBlockchainSynced = true;

    return true;
}

void CFundamentalnodeSync::Reset()
{
    lastFundamentalnodeList = 0;
    lastFundamentalnodeWinner = 0;
    lastBudgetItem = 0;
    mapSeenSyncMNB.clear();
    mapSeenSyncMNW.clear();
    mapSeenSyncBudget.clear();
    lastFailure = 0;
    nCountFailures = 0;
    sumFundamentalnodeList = 0;
    sumFundamentalnodeWinner = 0;
    sumBudgetItemProp = 0;
    sumBudgetItemFin = 0;
    countFundamentalnodeList = 0;
    countFundamentalnodeWinner = 0;
    countBudgetItemProp = 0;
    countBudgetItemFin = 0;
    RequestedFundamentalnodeAssets = FUNDAMENTALNODE_SYNC_INITIAL;
    RequestedFundamentalnodeAttempt = 0;
    nAssetSyncStarted = GetTime();
}

void CFundamentalnodeSync::AddedFundamentalnodeList(uint256 hash)
{
    if (mnodeman.mapSeenFundamentalnodeBroadcast.count(hash)) {
        if (mapSeenSyncMNB[hash] < FUNDAMENTALNODE_SYNC_THRESHOLD) {
            lastFundamentalnodeList = GetTime();
            mapSeenSyncMNB[hash]++;
        }
    } else {
        lastFundamentalnodeList = GetTime();
        mapSeenSyncMNB.insert(make_pair(hash, 1));
    }
}

void CFundamentalnodeSync::AddedFundamentalnodeWinner(uint256 hash)
{
    if (fundamentalnodePayments.mapFundamentalnodePayeeVotes.count(hash)) {
        if (mapSeenSyncMNW[hash] < FUNDAMENTALNODE_SYNC_THRESHOLD) {
            lastFundamentalnodeWinner = GetTime();
            mapSeenSyncMNW[hash]++;
        }
    } else {
        lastFundamentalnodeWinner = GetTime();
        mapSeenSyncMNW.insert(make_pair(hash, 1));
    }
}

void CFundamentalnodeSync::AddedBudgetItem(uint256 hash)
{
    if (budget.mapSeenFundamentalnodeBudgetProposals.count(hash) || budget.mapSeenFundamentalnodeBudgetVotes.count(hash) ||
        budget.mapSeenFinalizedBudgets.count(hash) || budget.mapSeenFinalizedBudgetVotes.count(hash)) {
        if (mapSeenSyncBudget[hash] < FUNDAMENTALNODE_SYNC_THRESHOLD) {
            lastBudgetItem = GetTime();
            mapSeenSyncBudget[hash]++;
        }
    } else {
        lastBudgetItem = GetTime();
        mapSeenSyncBudget.insert(make_pair(hash, 1));
    }
}

bool CFundamentalnodeSync::IsBudgetPropEmpty()
{
    return sumBudgetItemProp == 0 && countBudgetItemProp > 0;
}

bool CFundamentalnodeSync::IsBudgetFinEmpty()
{
    return sumBudgetItemFin == 0 && countBudgetItemFin > 0;
}

void CFundamentalnodeSync::GetNextAsset()
{
    switch (RequestedFundamentalnodeAssets) {
    case (FUNDAMENTALNODE_SYNC_INITIAL):
    case (FUNDAMENTALNODE_SYNC_FAILED): // should never be used here actually, use Reset() instead
        ClearFulfilledRequest();
        RequestedFundamentalnodeAssets = FUNDAMENTALNODE_SYNC_SPORKS;
        break;
    case (FUNDAMENTALNODE_SYNC_SPORKS):
        RequestedFundamentalnodeAssets = FUNDAMENTALNODE_SYNC_LIST;
        break;
    case (FUNDAMENTALNODE_SYNC_LIST):
        RequestedFundamentalnodeAssets = FUNDAMENTALNODE_SYNC_MNW;
        break;
    case (FUNDAMENTALNODE_SYNC_MNW):
        RequestedFundamentalnodeAssets = FUNDAMENTALNODE_SYNC_BUDGET;
        break;
    case (FUNDAMENTALNODE_SYNC_BUDGET):
        LogPrintf("CFundamentalnodeSync::GetNextAsset - Sync has finished\n");
        RequestedFundamentalnodeAssets = FUNDAMENTALNODE_SYNC_FINISHED;
        break;
    }
    RequestedFundamentalnodeAttempt = 0;
    nAssetSyncStarted = GetTime();
}

std::string CFundamentalnodeSync::GetSyncStatus()
{
    switch (fundamentalnodeSync.RequestedFundamentalnodeAssets) {
    case FUNDAMENTALNODE_SYNC_INITIAL:
        return _("Synchronization pending...");
    case FUNDAMENTALNODE_SYNC_SPORKS:
        return _("Synchronizing sporks...");
    case FUNDAMENTALNODE_SYNC_LIST:
        return _("Synchronizing fundamentalnodes...");
    case FUNDAMENTALNODE_SYNC_MNW:
        return _("Synchronizing fundamentalnode winners...");
    case FUNDAMENTALNODE_SYNC_BUDGET:
        return _("Synchronizing budgets...");
    case FUNDAMENTALNODE_SYNC_FAILED:
        return _("Synchronization failed");
    case FUNDAMENTALNODE_SYNC_FINISHED:
        return _("Synchronization finished");
    }
    return "";
}

void CFundamentalnodeSync::ProcessMessage(CNode* pfrom, std::string& strCommand, CDataStream& vRecv)
{
    if (strCommand == "ssc") { //Sync status count
        int nItemID;
        int nCount;
        vRecv >> nItemID >> nCount;

        if (RequestedFundamentalnodeAssets >= FUNDAMENTALNODE_SYNC_FINISHED) return;

        //this means we will receive no further communication
        switch (nItemID) {
        case (FUNDAMENTALNODE_SYNC_LIST):
            if (nItemID != RequestedFundamentalnodeAssets) return;
            sumFundamentalnodeList += nCount;
            countFundamentalnodeList++;
            break;
        case (FUNDAMENTALNODE_SYNC_MNW):
            if (nItemID != RequestedFundamentalnodeAssets) return;
            sumFundamentalnodeWinner += nCount;
            countFundamentalnodeWinner++;
            break;
        case (FUNDAMENTALNODE_SYNC_BUDGET_PROP):
            if (RequestedFundamentalnodeAssets != FUNDAMENTALNODE_SYNC_BUDGET) return;
            sumBudgetItemProp += nCount;
            countBudgetItemProp++;
            break;
        case (FUNDAMENTALNODE_SYNC_BUDGET_FIN):
            if (RequestedFundamentalnodeAssets != FUNDAMENTALNODE_SYNC_BUDGET) return;
            sumBudgetItemFin += nCount;
            countBudgetItemFin++;
            break;
        }

        LogPrint("fundamentalnode", "CFundamentalnodeSync:ProcessMessage - ssc - got inventory count %d %d\n", nItemID, nCount);
    }
}

void CFundamentalnodeSync::ClearFulfilledRequest()
{
    TRY_LOCK(cs_vNodes, lockRecv);
    if (!lockRecv) return;

    BOOST_FOREACH (CNode* pnode, vNodes) {
        pnode->ClearFulfilledRequest("getspork");
        pnode->ClearFulfilledRequest("fnsync");
        pnode->ClearFulfilledRequest("fnwsync");
        pnode->ClearFulfilledRequest("busync");
    }
}

void CFundamentalnodeSync::Process()
{
    static int tick = 0;

    if (tick++ % FUNDAMENTALNODE_SYNC_TIMEOUT != 0) return;

    if (IsSynced()) {
        /*
            Resync if we lose all fundamentalnodes from sleep/wake or failure to sync originally
        */
        if (mnodeman.CountEnabled() == 0) {
            Reset();
        } else
            return;
    }

    //try syncing again
    if (RequestedFundamentalnodeAssets == FUNDAMENTALNODE_SYNC_FAILED && lastFailure + (1 * 60) < GetTime()) {
        Reset();
    } else if (RequestedFundamentalnodeAssets == FUNDAMENTALNODE_SYNC_FAILED) {
        return;
    }

    LogPrint("fundamentalnode", "CFundamentalnodeSync::Process() - tick %d RequestedFundamentalnodeAssets %d\n", tick, RequestedFundamentalnodeAssets);

    if (RequestedFundamentalnodeAssets == FUNDAMENTALNODE_SYNC_INITIAL) GetNextAsset();

    // sporks synced but blockchain is not, wait until we're almost at a recent block to continue
    if (Params().NetworkID() != CBaseChainParams::REGTEST &&
        !IsBlockchainSynced() && RequestedFundamentalnodeAssets > FUNDAMENTALNODE_SYNC_SPORKS) return;

    TRY_LOCK(cs_vNodes, lockRecv);
    if (!lockRecv) return;

    BOOST_FOREACH (CNode* pnode, vNodes) {
        if (Params().NetworkID() == CBaseChainParams::REGTEST) {
            if (RequestedFundamentalnodeAttempt <= 2) {
                pnode->PushMessage("getsporks"); //get current network sporks
            } else if (RequestedFundamentalnodeAttempt < 4) {
                mnodeman.DsegUpdate(pnode);
            } else if (RequestedFundamentalnodeAttempt < 6) {
                int nMnCount = mnodeman.CountEnabled();
                pnode->PushMessage("fnget", nMnCount); //sync payees
                uint256 n = 0;
                pnode->PushMessage("fnvs", n); //sync fundamentalnode votes
            } else {
                RequestedFundamentalnodeAssets = FUNDAMENTALNODE_SYNC_FINISHED;
            }
            RequestedFundamentalnodeAttempt++;
            return;
        }

        //set to synced
        if (RequestedFundamentalnodeAssets == FUNDAMENTALNODE_SYNC_SPORKS) {
            if (pnode->HasFulfilledRequest("getspork")) continue;
            pnode->FulfilledRequest("getspork");

            pnode->PushMessage("getsporks"); //get current network sporks
            if (RequestedFundamentalnodeAttempt >= 2) GetNextAsset();
            RequestedFundamentalnodeAttempt++;

            return;
        }

        if (pnode->nVersion >= fundamentalnodePayments.GetMinFundamentalnodePaymentsProto()) {
            if (RequestedFundamentalnodeAssets == FUNDAMENTALNODE_SYNC_LIST) {
                LogPrint("fundamentalnode", "CFundamentalnodeSync::Process() - lastFundamentalnodeList %lld (GetTime() - FUNDAMENTALNODE_SYNC_TIMEOUT) %lld\n", lastFundamentalnodeList, GetTime() - FUNDAMENTALNODE_SYNC_TIMEOUT);
                if (lastFundamentalnodeList > 0 && lastFundamentalnodeList < GetTime() - FUNDAMENTALNODE_SYNC_TIMEOUT * 2 && RequestedFundamentalnodeAttempt >= FUNDAMENTALNODE_SYNC_THRESHOLD) { //hasn't received a new item in the last five seconds, so we'll move to the
                    GetNextAsset();
                    return;
                }

                if (pnode->HasFulfilledRequest("fnsync")) continue;
                pnode->FulfilledRequest("fnsync");

                // timeout
                if (lastFundamentalnodeList == 0 &&
                    (RequestedFundamentalnodeAttempt >= FUNDAMENTALNODE_SYNC_THRESHOLD * 3 || GetTime() - nAssetSyncStarted > FUNDAMENTALNODE_SYNC_TIMEOUT * 5)) {
                    if (IsSporkActive(SPORK_8_FUNDAMENTALNODE_PAYMENT_ENFORCEMENT)) {
                        LogPrintf("CFundamentalnodeSync::Process - ERROR - Sync has failed, will retry later\n");
                        RequestedFundamentalnodeAssets = FUNDAMENTALNODE_SYNC_FAILED;
                        RequestedFundamentalnodeAttempt = 0;
                        lastFailure = GetTime();
                        nCountFailures++;
                    } else {
                        GetNextAsset();
                    }
                    return;
                }

                if (RequestedFundamentalnodeAttempt >= FUNDAMENTALNODE_SYNC_THRESHOLD * 3) return;

                mnodeman.DsegUpdate(pnode);
                RequestedFundamentalnodeAttempt++;
                return;
            }

            if (RequestedFundamentalnodeAssets == FUNDAMENTALNODE_SYNC_MNW) {
                if (lastFundamentalnodeWinner > 0 && lastFundamentalnodeWinner < GetTime() - FUNDAMENTALNODE_SYNC_TIMEOUT * 2 && RequestedFundamentalnodeAttempt >= FUNDAMENTALNODE_SYNC_THRESHOLD) { //hasn't received a new item in the last five seconds, so we'll move to the
                    GetNextAsset();
                    return;
                }

                if (pnode->HasFulfilledRequest("fnwsync")) continue;
                pnode->FulfilledRequest("fnwsync");

                // timeout
                if (lastFundamentalnodeWinner == 0 &&
                    (RequestedFundamentalnodeAttempt >= FUNDAMENTALNODE_SYNC_THRESHOLD * 3 || GetTime() - nAssetSyncStarted > FUNDAMENTALNODE_SYNC_TIMEOUT * 5)) {
                    if (IsSporkActive(SPORK_8_FUNDAMENTALNODE_PAYMENT_ENFORCEMENT)) {
                        LogPrintf("CFundamentalnodeSync::Process - ERROR - Sync has failed, will retry later\n");
                        RequestedFundamentalnodeAssets = FUNDAMENTALNODE_SYNC_FAILED;
                        RequestedFundamentalnodeAttempt = 0;
                        lastFailure = GetTime();
                        nCountFailures++;
                    } else {
                        GetNextAsset();
                    }
                    return;
                }

                if (RequestedFundamentalnodeAttempt >= FUNDAMENTALNODE_SYNC_THRESHOLD * 3) return;

                CBlockIndex* pindexPrev = chainActive.Tip();
                if (pindexPrev == NULL) return;

                int nMnCount = mnodeman.CountEnabled();
                pnode->PushMessage("fnget", nMnCount); //sync payees
                RequestedFundamentalnodeAttempt++;

                return;
            }
        }

        if (pnode->nVersion >= ActiveProtocol()) {
            if (RequestedFundamentalnodeAssets == FUNDAMENTALNODE_SYNC_BUDGET) {

                // We'll start rejecting votes if we accidentally get set as synced too soon
                if (lastBudgetItem > 0 && lastBudgetItem < GetTime() - FUNDAMENTALNODE_SYNC_TIMEOUT * 2 && RequestedFundamentalnodeAttempt >= FUNDAMENTALNODE_SYNC_THRESHOLD) {

                    // Hasn't received a new item in the last five seconds, so we'll move to the
                    GetNextAsset();

                    // Try to activate our fundamentalnode if possible
                    activeFundamentalnode.ManageStatus();

                    return;
                }

                // timeout
                if (lastBudgetItem == 0 &&
                    (RequestedFundamentalnodeAttempt >= FUNDAMENTALNODE_SYNC_THRESHOLD * 3 || GetTime() - nAssetSyncStarted > FUNDAMENTALNODE_SYNC_TIMEOUT * 5)) {
                    // maybe there is no budgets at all, so just finish syncing
                    GetNextAsset();
                    activeFundamentalnode.ManageStatus();
                    return;
                }

                if (pnode->HasFulfilledRequest("busync")) continue;
                pnode->FulfilledRequest("busync");

                if (RequestedFundamentalnodeAttempt >= FUNDAMENTALNODE_SYNC_THRESHOLD * 3) return;

                uint256 n = 0;
                pnode->PushMessage("fnvs", n); //sync fundamentalnode votes
                RequestedFundamentalnodeAttempt++;

                return;
            }
        }
    }
}
