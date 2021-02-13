// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "chain.h"
#include "spork.h"

using namespace std;

/**
 * CChain implementation
 */
void CChain::SetTip(CBlockIndex* pindex)
{
    if (pindex == NULL) {
        vChain.clear();
        return;
    }
    vChain.resize(pindex->nHeight + 1);
    while (pindex && vChain[pindex->nHeight] != pindex) {
        vChain[pindex->nHeight] = pindex;
        pindex = pindex->pprev;
    }
}

CBlockLocator CChain::GetLocator(const CBlockIndex* pindex) const
{
    int nStep = 1;
    std::vector<uint256> vHave;
    vHave.reserve(32);

    if (!pindex)
        pindex = Tip();
    while (pindex) {
        vHave.push_back(pindex->GetBlockHash());
        // Stop when we have added the genesis block.
        if (pindex->nHeight == 0)
            break;
        // Exponentially larger steps back, plus the genesis block.
        int nHeight = std::max(pindex->nHeight - nStep, 0);
        if (Contains(pindex)) {
            // Use O(1) CChain index if possible.
            pindex = (*this)[nHeight];
        } else {
            // Otherwise, use O(log n) skiplist.
            pindex = pindex->GetAncestor(nHeight);
        }
        if (vHave.size() > 10)
            nStep *= 2;
    }

    return CBlockLocator(vHave);
}

const CBlockIndex* CChain::FindFork(const CBlockIndex* pindex) const
{
    if (pindex->nHeight > Height())
        pindex = pindex->GetAncestor(Height());
    while (pindex && !Contains(pindex))
        pindex = pindex->pprev;
    return pindex;
}

uint256 CBlockIndex::GetBlockTrust() const
{
    uint256 bnTarget;
    bnTarget.SetCompact(nBits);
    if (bnTarget <= 0)
        return 0;

    if (IsProofOfStake()) {
        // Return trust score as usual
        return (uint256(1) << 256) / (bnTarget + 1);
    } else {
        // Calculate work amount for block
        uint256 bnPoWTrust = ((~uint256(0) >> 20) / (bnTarget + 1));
        return bnPoWTrust > 1 ? bnPoWTrust : 1;
    }
}

int64_t CBlockIndex::MaxFutureBlockTime() const
{
    return GetAdjustedTime() + Params().FutureBlockTimeDrift(nHeight+1, GetSporkValue(SPORK_23_TIME_PROTOCOL_V2_BLOCK));
}

int64_t CBlockIndex::MinPastBlockTime() const
{
    // Time Protocol v1: pindexPrev->MedianTimePast + 1
    if (!Params().IsTimeProtocolV2(nHeight+1, GetSporkValue(SPORK_23_TIME_PROTOCOL_V2_BLOCK)))
        return GetMedianTimePast();

    // on the transition from Time Protocol v1 to v2
    // pindexPrev->nTime might be in the future (up to the allowed drift)
    // so we allow the nBlockTimeProtocolV2 to be at most (180-14) seconds earlier than previous block
    if (nHeight + 1 == Params().BlockStartTimeProtocolV2(GetSporkValue(SPORK_23_TIME_PROTOCOL_V2_BLOCK)))
        return GetBlockTime() - Params().FutureBlockTimeDrift(nHeight, GetSporkValue(SPORK_23_TIME_PROTOCOL_V2_BLOCK)) + Params().FutureBlockTimeDrift(nHeight + 1, GetSporkValue(SPORK_23_TIME_PROTOCOL_V2_BLOCK));

    // Time Protocol v2: pindexPrev->nTime
    return GetBlockTime();
}

