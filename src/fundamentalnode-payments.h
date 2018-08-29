// Copyright (c) 2014-2015 The Dash developers
// Copyright (c) 2015-2017 The PIVX developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef FUNDAMENTALNODE_PAYMENTS_H
#define FUNDAMENTALNODE_PAYMENTS_H

#include "key.h"
#include "main.h"
#include "fundamentalnode.h"
#include <boost/lexical_cast.hpp>

using namespace std;

extern CCriticalSection cs_vecPayments;
extern CCriticalSection cs_mapFundamentalnodeBlocks;
extern CCriticalSection cs_mapFundamentalnodePayeeVotes;

class CFundamentalnodePayments;
class CFundamentalnodePaymentWinner;
class CFundamentalnodeBlockPayees;

extern CFundamentalnodePayments fundamentalnodePayments;

#define MNPAYMENTS_SIGNATURES_REQUIRED 6
#define MNPAYMENTS_SIGNATURES_TOTAL 10

void ProcessMessageFundamentalnodePayments(CNode* pfrom, std::string& strCommand, CDataStream& vRecv);
bool IsBlockPayeeValid(const CBlock& block, int nBlockHeight);
std::string GetRequiredPaymentsString(int nBlockHeight);
bool IsBlockValueValid(const CBlock& block, CAmount nExpectedValue, CAmount nMinted);
void FillBlockPayee(CMutableTransaction& txNew, CAmount nFees, bool fProofOfStake, bool IsMasternode );

void DumpFundamentalnodePayments();

/** Save Fundamentalnode Payment Data (fnpayments.dat)
 */
class CFundamentalnodePaymentDB
{
private:
    boost::filesystem::path pathDB;
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

    CFundamentalnodePaymentDB();
    bool Write(const CFundamentalnodePayments& objToSave);
    ReadResult Read(CFundamentalnodePayments& objToLoad, bool fDryRun = false);
};

class CFundamentalnodePayee
{
public:
    CScript scriptPubKey;
    int nVotes;

    CFundamentalnodePayee()
    {
        scriptPubKey = CScript();
        nVotes = 0;
    }

    CFundamentalnodePayee(CScript payee, int nVotesIn)
    {
        scriptPubKey = payee;
        nVotes = nVotesIn;
    }

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion)
    {
        READWRITE(scriptPubKey);
        READWRITE(nVotes);
    }
};

// Keep track of votes for payees from fundamentalnodes
class CFundamentalnodeBlockPayees
{
public:
    int nBlockHeight;
    std::vector<CFundamentalnodePayee> vecPayments;

    CFundamentalnodeBlockPayees()
    {
        nBlockHeight = 0;
        vecPayments.clear();
    }
    CFundamentalnodeBlockPayees(int nBlockHeightIn)
    {
        nBlockHeight = nBlockHeightIn;
        vecPayments.clear();
    }

    void AddPayee(CScript payeeIn, int nIncrement)
    {
        LOCK(cs_vecPayments);

        BOOST_FOREACH (CFundamentalnodePayee& payee, vecPayments) {
            if (payee.scriptPubKey == payeeIn) {
                payee.nVotes += nIncrement;
                return;
            }
        }

        CFundamentalnodePayee c(payeeIn, nIncrement);
        vecPayments.push_back(c);
    }

    bool GetPayee(CScript& payee)
    {
        LOCK(cs_vecPayments);

        int nVotes = -1;
        BOOST_FOREACH (CFundamentalnodePayee& p, vecPayments) {
            if (p.nVotes > nVotes) {
                payee = p.scriptPubKey;
                nVotes = p.nVotes;
            }
        }

        return (nVotes > -1);
    }

    bool HasPayeeWithVotes(CScript payee, int nVotesReq)
    {
        LOCK(cs_vecPayments);

        BOOST_FOREACH (CFundamentalnodePayee& p, vecPayments) {
            if (p.nVotes >= nVotesReq && p.scriptPubKey == payee) return true;
        }

        return false;
    }

    bool IsTransactionValid(const CTransaction& txNew);
    std::string GetRequiredPaymentsString();

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion)
    {
        READWRITE(nBlockHeight);
        READWRITE(vecPayments);
    }
};

// for storing the winning payments
class CFundamentalnodePaymentWinner
{
public:
    CTxIn vinFundamentalnode;

    int nBlockHeight;
    CScript payee;
    std::vector<unsigned char> vchSig;

    CFundamentalnodePaymentWinner()
    {
        nBlockHeight = 0;
        vinFundamentalnode = CTxIn();
        payee = CScript();
    }

    CFundamentalnodePaymentWinner(CTxIn vinIn)
    {
        nBlockHeight = 0;
        vinFundamentalnode = vinIn;
        payee = CScript();
    }

    uint256 GetHash()
    {
        CHashWriter ss(SER_GETHASH, PROTOCOL_VERSION);
        ss << payee;
        ss << nBlockHeight;
        ss << vinFundamentalnode.prevout;

        return ss.GetHash();
    }

    bool Sign(CKey& keyFundamentalnode, CPubKey& pubKeyFundamentalnode);
    bool IsValid(CNode* pnode, std::string& strError);
    bool SignatureValid();
    void Relay();

    void AddPayee(CScript payeeIn)
    {
        payee = payeeIn;
    }


    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion)
    {
        READWRITE(vinFundamentalnode);
        READWRITE(nBlockHeight);
        READWRITE(payee);
        READWRITE(vchSig);
    }

    std::string ToString()
    {
        std::string ret = "";
        ret += vinFundamentalnode.ToString();
        ret += ", " + boost::lexical_cast<std::string>(nBlockHeight);
        ret += ", " + payee.ToString();
        ret += ", " + boost::lexical_cast<std::string>((int)vchSig.size());
        return ret;
    }
};

//
// fundamentalnode Payments Class
// Keeps track of who should get paid for which blocks
//

class CFundamentalnodePayments
{
private:
    int nSyncedFromPeer;
    int nLastBlockHeight;

public:
    std::map<uint256, CFundamentalnodePaymentWinner> mapFundamentalnodePayeeVotes;
    std::map<int, CFundamentalnodeBlockPayees> mapFundamentalnodeBlocks;
    std::map<uint256, int> mapFundamentalnodesLastVote; //prevout.hash + prevout.n, nBlockHeight

    CFundamentalnodePayments()
    {
        nSyncedFromPeer = 0;
        nLastBlockHeight = 0;
    }

    void Clear()
    {
        LOCK2(cs_mapFundamentalnodeBlocks, cs_mapFundamentalnodePayeeVotes);
        mapFundamentalnodeBlocks.clear();
        mapFundamentalnodePayeeVotes.clear();
    }

    bool AddWinningFundamentalnode(CFundamentalnodePaymentWinner& winner);
    bool ProcessBlock(int nBlockHeight);

    void Sync(CNode* node, int nCountNeeded);
    void CleanPaymentList();
    int LastPayment(CFundamentalnode& mn);

    bool GetBlockPayee(int nBlockHeight, CScript& payee);
    bool IsTransactionValid(const CTransaction& txNew, int nBlockHeight);
    bool IsScheduled(CFundamentalnode& mn, int nNotBlockHeight);

    bool CanVote(COutPoint outFundamentalnode, int nBlockHeight)
    {
        LOCK(cs_mapFundamentalnodePayeeVotes);

        if (mapFundamentalnodesLastVote.count(outFundamentalnode.hash + outFundamentalnode.n)) {
            if (mapFundamentalnodesLastVote[outFundamentalnode.hash + outFundamentalnode.n] == nBlockHeight) {
                return false;
            }
        }

        //record this fundamentalnode voted
        mapFundamentalnodesLastVote[outFundamentalnode.hash + outFundamentalnode.n] = nBlockHeight;
        return true;
    }

    int GetMinFundamentalnodePaymentsProto();
    void ProcessMessageFundamentalnodePayments(CNode* pfrom, std::string& strCommand, CDataStream& vRecv);
    std::string GetRequiredPaymentsString(int nBlockHeight);
    void FillBlockPayee(CMutableTransaction& txNew, int64_t nFees, bool fProofOfStake, bool IsMasternode);
    std::string ToString() const;
    int GetOldestBlock();
    int GetNewestBlock();

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion)
    {
        READWRITE(mapFundamentalnodePayeeVotes);
        READWRITE(mapFundamentalnodeBlocks);
    }
};


#endif
