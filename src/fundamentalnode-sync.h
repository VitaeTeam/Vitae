// Copyright (c) 2014-2015 The Dash developers
// Copyright (c) 2015-2017 The PIVX developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef FUNDAMENTALNODE_SYNC_H
#define FUNDAMENTALNODE_SYNC_H

#define FUNDAMENTALNODE_SYNC_INITIAL 0
#define FUNDAMENTALNODE_SYNC_SPORKS 1
#define FUNDAMENTALNODE_SYNC_LIST 2
#define FUNDAMENTALNODE_SYNC_MNW 3
#define FUNDAMENTALNODE_SYNC_BUDGET 4
#define FUNDAMENTALNODE_SYNC_BUDGET_PROP 10
#define FUNDAMENTALNODE_SYNC_BUDGET_FIN 11
#define FUNDAMENTALNODE_SYNC_FAILED 998
#define FUNDAMENTALNODE_SYNC_FINISHED 999

#define FUNDAMENTALNODE_SYNC_TIMEOUT 5
#define FUNDAMENTALNODE_SYNC_THRESHOLD 2

class CFundamentalnodeSync;
extern CFundamentalnodeSync fundamentalnodeSync;

//
// CFundamentalnodeSync : Sync fundamentalnode assets in stages
//

class CFundamentalnodeSync
{
public:
    std::map<uint256, int> mapSeenSyncMNB;
    std::map<uint256, int> mapSeenSyncMNW;
    std::map<uint256, int> mapSeenSyncBudget;

    int64_t lastFundamentalnodeList;
    int64_t lastFundamentalnodeWinner;
    int64_t lastBudgetItem;
    int64_t lastFailure;
    int nCountFailures;

    // sum of all counts
    int sumFundamentalnodeList;
    int sumFundamentalnodeWinner;
    int sumBudgetItemProp;
    int sumBudgetItemFin;
    // peers that reported counts
    int countFundamentalnodeList;
    int countFundamentalnodeWinner;
    int countBudgetItemProp;
    int countBudgetItemFin;

    // Count peers we've requested the list from
    int RequestedFundamentalnodeAssets;
    int RequestedFundamentalnodeAttempt;

    // Time when current fundamentalnode asset sync started
    int64_t nAssetSyncStarted;

    CFundamentalnodeSync();

    void AddedFundamentalnodeList(uint256 hash);
    void AddedFundamentalnodeWinner(uint256 hash);
    void AddedBudgetItem(uint256 hash);
    void GetNextAsset();
    std::string GetSyncStatus();
    void ProcessMessage(CNode* pfrom, std::string& strCommand, CDataStream& vRecv);
    bool IsBudgetFinEmpty();
    bool IsBudgetPropEmpty();

    void Reset();
    void Process();
    bool IsSynced();
    bool IsBlockchainSynced();
    bool IsFundamentalnodeListSynced() { return RequestedFundamentalnodeAssets > FUNDAMENTALNODE_SYNC_LIST; }
    void ClearFulfilledRequest();
};

#endif
