// Copyright (c) 2009-2012 BITCOIN CORE developers
// Copyright (c) 2014-2015 The Dash developers
// Copyright (c) 2015-2017 The PIVX developers
// Copyright (c) 2018 The VITAE developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SWIFTTX_H
#define SWIFTTX_H

#include "base58.h"
#include "key.h"
#include "main.h"
#include "net.h"
#include "spork.h"
#include "sync.h"
#include "util.h"

/*
    At 15 signatures, 1/2 of the fundamentalnode network can be owned by
    one party without comprimising the security of SwiftX
    (1000/2150.0)**10 = 0.00047382219560689856
    (1000/2900.0)**10 = 2.3769498616783657e-05

    ### getting 5 of 10 signatures w/ 1000 nodes of 2900
    (1000/2900.0)**5 = 0.004875397277841433
*/
#define SWIFTTX_SIGNATURES_REQUIRED 6
#define SWIFTTX_SIGNATURES_TOTAL 10


class CConsensusVote;
class CTransaction;
class CTransactionLock;

static const int MIN_SWIFTTX_PROTO_VERSION = 70103;

extern std::map<uint256, CTransaction> mapTxLockReq;
extern std::map<uint256, CTransaction> mapTxLockReqRejected;
extern std::map<uint256, CConsensusVote> mapTxLockVote;
extern std::map<uint256, CTransactionLock> mapTxLocks;
extern std::map<COutPoint, uint256> mapLockedInputs;
extern int nCompleteTXLocks;


int64_t CreateNewLock(CTransaction tx);

bool IsIXTXValid(const CTransaction& txCollateral);

// if two conflicting locks are approved by the network, they will cancel out
bool CheckForConflictingLocks(CTransaction& tx);

void ProcessMessageSwiftTX(CNode* pfrom, std::string& strCommand, CDataStream& vRecv);

//check if we need to vote on this transaction
void DoConsensusVote(CTransaction& tx, int64_t nBlockHeight);

//process consensus vote message
bool ProcessConsensusVote(CNode* pnode, CConsensusVote& ctx);

// keep transaction locks in memory for an hour
void CleanTransactionLocksList();

// get the accepted transaction lock signatures
int GetTransactionLockSignatures(uint256 txHash);

int64_t GetAverageVoteTime();

class CConsensusVote
{
public:
    CTxIn vinFundamentalnode;
    uint256 txHash;
    int nBlockHeight;
    std::vector<unsigned char> vchSig;

    CConsensusVote() :
        vinFundamentalnode(),
        txHash(),
        nBlockHeight(0),
        vchSig()
    {}

    uint256 GetHash() const;
    uint256 GetSignatureHash() const;
    std::string GetStrMessage() const;

    bool Sign();
    bool SignatureValid() const;

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion)
    {
        READWRITE(txHash);
        READWRITE(vinFundamentalnode);
        READWRITE(vchSig);
        READWRITE(nBlockHeight);
    }
};

class CTransactionLock
{
public:
    int nBlockHeight;
    uint256 txHash;
    std::vector<CConsensusVote> vecConsensusVotes;
    int nExpiration;
    int nTimeout;

    bool SignaturesValid();
    int CountSignatures();
    void AddSignature(CConsensusVote& cv);

    uint256 GetHash()
    {
        return txHash;
    }
};


#endif
