#include <primitives/deterministicmint.h>
#include "zvittracker.h"
#include "util.h"
#include "sync.h"
#include "main.h"
#include "txdb.h"
#include "walletdb.h"

using namespace std;

CzPIVTracker::CzPIVTracker(std::string strWalletFile)
{
    this->strWalletFile = strWalletFile;
    mapSerialHashes.clear();
    mapPendingSpends.clear();
    fInitialized = false;
}

CzPIVTracker::~CzPIVTracker()
{
    mapSerialHashes.clear();
    mapPendingSpends.clear();
}

void CzPIVTracker::Init()
{
    //Load all CZerocoinMints and CDeterministicMints from the database
    if (!fInitialized) {
        ListMints(false, false, true);
        fInitialized = true;
    }
}

bool CzPIVTracker::Archive(CMintMeta& meta)
{
    if (mapSerialHashes.count(meta.hashSerial))
        mapSerialHashes.at(meta.hashSerial).isArchived = true;

    CWalletDB walletdb(strWalletFile);
    CZerocoinMint mint;
    if (walletdb.ReadZerocoinMint(meta.hashPubcoin, mint)) {
        if (!CWalletDB(strWalletFile).ArchiveMintOrphan(mint))
            return error("%s: failed to archive zerocoinmint", __func__);
    } else {
        //failed to read mint from DB, try reading deterministic
        CDeterministicMint dMint;
        if (!walletdb.ReadDeterministicMint(meta.hashPubcoin, dMint))
            return error("%s: could not find pubcoinhash %s in db", __func__, meta.hashPubcoin.GetHex());
        if (!walletdb.ArchiveDeterministicOrphan(dMint))
            return error("%s: failed to archive deterministic ophaned mint", __func__);
    }

    LogPrintf("%s: archived pubcoinhash %s\n", __func__, meta.hashPubcoin.GetHex());
    return true;
}

bool CzPIVTracker::UnArchive(const uint256& hashPubcoin, bool isDeterministic)
{
    CWalletDB walletdb(strWalletFile);
    if (isDeterministic) {
        CDeterministicMint dMint;
        if (!walletdb.UnarchiveDeterministicMint(hashPubcoin, dMint))
            return error("%s: failed to unarchive deterministic mint", __func__);
        Add(dMint, false);
    } else {
        CZerocoinMint mint;
        if (!walletdb.UnarchiveZerocoinMint(hashPubcoin, mint))
            return error("%s: failed to unarchivezerocoin mint", __func__);
        Add(mint, false);
    }

    LogPrintf("%s: unarchived %s\n", __func__, hashPubcoin.GetHex());
    return true;
}

CMintMeta CzPIVTracker::Get(const uint256 &hashSerial)
{
    if (!mapSerialHashes.count(hashSerial))
        return CMintMeta();

    return mapSerialHashes.at(hashSerial);
}

CMintMeta CzPIVTracker::GetMetaFromPubcoin(const uint256& hashPubcoin)
{
    for (auto it : mapSerialHashes) {
        CMintMeta meta = it.second;
        if (meta.hashPubcoin == hashPubcoin)
            return meta;
    }

    return CMintMeta();
}

bool CzPIVTracker::GetMetaFromStakeHash(const uint256& hashStake, CMintMeta& meta) const
{
    for (auto& it : mapSerialHashes) {
        if (it.second.hashStake == hashStake) {
            meta = it.second;
            return true;
        }
    }

    return false;
}

std::vector<uint256> CzPIVTracker::GetSerialHashes()
{
    vector<uint256> vHashes;
    for (auto it : mapSerialHashes) {
        if (it.second.isArchived)
            continue;

        vHashes.emplace_back(it.first);
    }


    return vHashes;
}

CAmount CzPIVTracker::GetBalance(bool fConfirmedOnly, bool fUnconfirmedOnly) const
{
    CAmount nTotal = 0;
    //! zerocoin specific fields
    std::map<libzerocoin::CoinDenomination, unsigned int> myZerocoinSupply;
    for (auto& denom : libzerocoin::zerocoinDenomList) {
        myZerocoinSupply.insert(make_pair(denom, 0));
    }

    {
        //LOCK(cs_pivtracker);
        // Get Unused coins
        for (auto& it : mapSerialHashes) {
            CMintMeta meta = it.second;
            if (meta.isUsed || meta.isArchived)
                continue;
            bool fConfirmed = ((meta.nHeight < chainActive.Height() - Params().Zerocoin_MintRequiredConfirmations()) && !(meta.nHeight == 0));
            if (fConfirmedOnly && !fConfirmed)
                continue;
            if (fUnconfirmedOnly && fConfirmed)
                continue;

            nTotal += libzerocoin::ZerocoinDenominationToAmount(meta.denom);
            myZerocoinSupply.at(meta.denom)++;
        }
    }
    for (auto& denom : libzerocoin::zerocoinDenomList) {
        LogPrint("zero","%s My coins for denomination %d pubcoin %s\n", __func__,denom, myZerocoinSupply.at(denom));
    }
    LogPrint("zero","Total value of coins %d\n",nTotal);

    if (nTotal < 0 ) nTotal = 0; // Sanity never hurts

    return nTotal;
}

CAmount CzPIVTracker::GetUnconfirmedBalance() const
{
    return GetBalance(false, true);
}

std::vector<CMintMeta> CzPIVTracker::GetMints(bool fConfirmedOnly) const
{
    vector<CMintMeta> vMints;
    for (auto& it : mapSerialHashes) {
        CMintMeta mint = it.second;
        if (mint.isArchived || mint.isUsed)
            continue;
        bool fConfirmed = (mint.nHeight < chainActive.Height() - Params().Zerocoin_MintRequiredConfirmations());
        if (fConfirmedOnly && !fConfirmed)
            continue;
        vMints.emplace_back(mint);
    }
    return vMints;
}

//Does a mint in the tracker have this txid
bool CzPIVTracker::HasMintTx(const uint256& txid)
{
    for (auto it : mapSerialHashes) {
        if (it.second.txid == txid)
            return true;
    }

    return false;
}

bool CzPIVTracker::HasPubcoin(const CBigNum &bnValue) const
{
    // Check if this mint's pubcoin value belongs to our mapSerialHashes (which includes hashpubcoin values)
    uint256 hash = GetPubCoinHash(bnValue);
    return HasPubcoinHash(hash);
}

bool CzPIVTracker::HasPubcoinHash(const uint256& hashPubcoin) const
{
    for (auto it : mapSerialHashes) {
        CMintMeta meta = it.second;
        if (meta.hashPubcoin == hashPubcoin)
            return true;
    }
    return false;
}

bool CzPIVTracker::HasSerial(const CBigNum& bnSerial) const
{
    uint256 hash = GetSerialHash(bnSerial);
    return HasSerialHash(hash);
}

bool CzPIVTracker::HasSerialHash(const uint256& hashSerial) const
{
    auto it = mapSerialHashes.find(hashSerial);
    return it != mapSerialHashes.end();
}

bool CzPIVTracker::UpdateZerocoinMint(const CZerocoinMint& mint)
{
    if (!HasSerial(mint.GetSerialNumber()))
        return error("%s: mint %s is not known", __func__, mint.GetValue().GetHex());

    uint256 hashSerial = GetSerialHash(mint.GetSerialNumber());

    //Update the meta object
    CMintMeta meta = Get(hashSerial);
    meta.isUsed = mint.IsUsed();
    meta.denom = mint.GetDenomination();
    meta.nHeight = mint.GetHeight();
    mapSerialHashes.at(hashSerial) = meta;

    //Write to db
    return CWalletDB(strWalletFile).WriteZerocoinMint(mint);
}

bool CzPIVTracker::UpdateState(const CMintMeta& meta)
{
    CWalletDB walletdb(strWalletFile);

    if (meta.isDeterministic) {
        CDeterministicMint dMint;
        if (!walletdb.ReadDeterministicMint(meta.hashPubcoin, dMint))
            return error("%s: failed to read deterministic mint from database", __func__);

        dMint.SetTxHash(meta.txid);
        dMint.SetHeight(meta.nHeight);
        dMint.SetUsed(meta.isUsed);
        dMint.SetDenomination(meta.denom);
        dMint.SetStakeHash(meta.hashStake);

        if (!walletdb.WriteDeterministicMint(dMint))
            return error("%s: failed to update deterministic mint when writing to db", __func__);
    } else {
        CZerocoinMint mint;
        if (!walletdb.ReadZerocoinMint(meta.hashPubcoin, mint))
            return error("%s: failed to read mint from database", __func__);

        mint.SetTxHash(meta.txid);
        mint.SetHeight(meta.nHeight);
        mint.SetUsed(meta.isUsed);
        mint.SetDenomination(meta.denom);

        if (!walletdb.WriteZerocoinMint(mint))
            return error("%s: failed to write mint to database", __func__);
    }

    mapSerialHashes[meta.hashSerial] = meta;

    return true;
}

void CzPIVTracker::Add(const CDeterministicMint& dMint, bool isNew, bool isArchived)
{
    CMintMeta meta;
    meta.hashPubcoin = dMint.GetPubcoinHash();
    meta.nHeight = dMint.GetHeight();
    meta.nVersion = dMint.GetVersion();
    meta.txid = dMint.GetTxHash();
    meta.isUsed = dMint.IsUsed();
    meta.hashSerial = dMint.GetSerialHash();
    meta.hashStake = dMint.GetStakeHash();
    meta.denom = dMint.GetDenomination();
    meta.isArchived = isArchived;
    meta.isDeterministic = true;
    mapSerialHashes[meta.hashSerial] = meta;

    if (isNew)
        CWalletDB(strWalletFile).WriteDeterministicMint(dMint);
}

void CzPIVTracker::Add(const CZerocoinMint& mint, bool isNew, bool isArchived)
{
    CMintMeta meta;
    meta.hashPubcoin = GetPubCoinHash(mint.GetValue());
    meta.nHeight = mint.GetHeight();
    meta.nVersion = libzerocoin::ExtractVersionFromSerial(mint.GetSerialNumber());
    meta.txid = mint.GetTxHash();
    meta.isUsed = mint.IsUsed();
    meta.hashSerial = GetSerialHash(mint.GetSerialNumber());
    uint256 nSerial = mint.GetSerialNumber().getuint256();
    meta.hashStake = Hash(nSerial.begin(), nSerial.end());
    meta.denom = mint.GetDenomination();
    meta.isArchived = isArchived;
    meta.isDeterministic = false;
    mapSerialHashes[meta.hashSerial] = meta;

    if (isNew)
        CWalletDB(strWalletFile).WriteZerocoinMint(mint);
}

void CzPIVTracker::SetPubcoinUsed(const uint256& hashPubcoin, const uint256& txid)
{
    if (!HasPubcoinHash(hashPubcoin))
        return;
    CMintMeta meta = GetMetaFromPubcoin(hashPubcoin);
    meta.isUsed = true;
    mapPendingSpends.insert(make_pair(meta.hashSerial, txid));
    UpdateState(meta);
}

void CzPIVTracker::SetPubcoinNotUsed(const uint256& hashPubcoin)
{
    if (!HasPubcoinHash(hashPubcoin))
        return;
    CMintMeta meta = GetMetaFromPubcoin(hashPubcoin);
    meta.isUsed = false;

    if (mapPendingSpends.count(meta.hashSerial))
        mapPendingSpends.erase(meta.hashSerial);

    UpdateState(meta);
}

void CzPIVTracker::RemovePending(const uint256& txid)
{
    uint256 hashSerial;
    for (auto it : mapPendingSpends) {
        if (it.second == txid) {
            hashSerial = it.first;
            break;
        }
    }

    if (hashSerial > 0)
        mapPendingSpends.erase(hashSerial);
}

std::set<CMintMeta> CzPIVTracker::ListMints(bool fUnusedOnly, bool fMatureOnly, bool fUpdateStatus)
{
    CWalletDB walletdb(strWalletFile);
    if (fUpdateStatus) {
        std::list<CZerocoinMint> listMintsDB = walletdb.ListMintedCoins();
        for (auto& mint : listMintsDB)
            Add(mint);
        LogPrintf("%s: added %d zerocoinmints from DB\n", __func__, listMintsDB.size());

        std::list<CDeterministicMint> listDeterministicDB = walletdb.ListDeterministicMints();
        for (auto& dMint : listDeterministicDB)
            Add(dMint);
        LogPrintf("%s: added %d dzpiv from DB\n", __func__, listDeterministicDB.size());
    }

    vector<CMintMeta> vOverWrite;
    std::set<CMintMeta> setMints;
    std::set<uint256> setMempool;
    {
        LOCK(mempool.cs);
        mempool.getTransactions(setMempool);
    }

    for (auto& it : mapSerialHashes) {
        CMintMeta mint = it.second;

        //This is only intended for unarchived coins
        if (mint.isArchived)
            continue;

        //if there is not a record of the block height, then look it up and assign it
        uint256 txid;
        bool isMintInChain = zerocoinDB->ReadCoinMint(mint.hashPubcoin, txid);

        //See if there is internal record of spending this mint (note this is memory only, would reset on restart)
        bool isPendingSpend = static_cast<bool>(mapPendingSpends.count(mint.hashSerial));
        if (isPendingSpend) {
            uint256 txidPendingSpend = mapPendingSpends.at(mint.hashSerial);
            if (!setMempool.count(txidPendingSpend)) {
                RemovePending(txidPendingSpend);
                LogPrintf("%s: pending txid %s removed because not in mempool\n", __func__, txidPendingSpend.GetHex());
            }

        }

        //See if there is blockchain record of spending this mint
        uint256 txidSpend;
        bool isConfirmedSpend = zerocoinDB->ReadCoinSpend(mint.hashSerial, txidSpend);

        //If a pending spend got confirmed, then remove it from the pendingspend map
        if (isPendingSpend && isConfirmedSpend)
            mapPendingSpends.erase(mint.hashSerial);

        bool isUsed = isPendingSpend || isConfirmedSpend;

        if (fUnusedOnly && isUsed)
            continue;

        if (fMatureOnly || fUpdateStatus) {
            if (!mint.nHeight || !isMintInChain || isUsed != mint.isUsed) {
                CTransaction tx;
                uint256 hashBlock;

                if (mint.txid == 0) {
                    if (!isMintInChain) {
                        LogPrintf("%s failed to find mint in zerocoinDB %s\n", __func__, mint.hashPubcoin.GetHex().substr(0, 6));
                        Archive(mint);
                        continue;
                    }
                    mint.txid = txid;
                }

                if (!IsInitialBlockDownload() && GetTransaction(mint.txid, tx, hashBlock, true)) {
                    LogPrintf("%s failed to find tx for mint txid=%s\n", __func__, mint.txid.GetHex());
                    Archive(mint);
                    continue;
                }

                //if not in the block index, most likely is unconfirmed tx
                if (mapBlockIndex.count(hashBlock) && !chainActive.Contains(mapBlockIndex[hashBlock])) {
                    if (mint.isUsed != isUsed) {
                        mint.isUsed = isUsed;
                        LogPrintf("%s: set mint %s isUsed to %d\n", __func__, mint.hashPubcoin.GetHex(), isUsed);
                    }
                    mint.nHeight = mapBlockIndex[hashBlock]->nHeight;
                    vOverWrite.emplace_back(mint);
                } else if (mint.isUsed != isUsed){
                    mint.isUsed = isUsed;
                    LogPrintf("%s: set mint %s isUsed to %d\n", __func__, mint.hashPubcoin.GetHex(), isUsed);
                    vOverWrite.emplace_back(mint);
                } else if (fMatureOnly) {
                    continue;
                }
            }

            //not mature
            if (mint.nHeight > chainActive.Height() - Params().Zerocoin_MintRequiredConfirmations()) {
                if (!fMatureOnly)
                    setMints.insert(mint);
                continue;
            }

            //if only requesting an update (fUpdateStatus) then skip the rest and add to list
            if (fMatureOnly) {
                // check to make sure there are at least 3 other mints added to the accumulators after this
                if (chainActive.Height() < mint.nHeight + 1)
                    continue;

                CBlockIndex *pindex = chainActive[mint.nHeight + 1];
                int nMintsAdded = 0;
                while (pindex->nHeight < chainActive.Height() - 30) { // 30 just to make sure that its at least 2 checkpoints from the top block
                    nMintsAdded += count(pindex->vMintDenominationsInBlock.begin(),
                                         pindex->vMintDenominationsInBlock.end(), mint.denom);
                    if (nMintsAdded >= Params().Zerocoin_RequiredAccumulation())
                        break;
                    pindex = chainActive[pindex->nHeight + 1];
                }

                if (nMintsAdded < Params().Zerocoin_RequiredAccumulation())
                    continue;
            }
        }
        setMints.insert(mint);
    }

    //overwrite any updates
    for (CMintMeta& meta : vOverWrite)
        UpdateState(meta);

    return setMints;
}

void CzPIVTracker::Clear()
{
    mapSerialHashes.clear();
}
