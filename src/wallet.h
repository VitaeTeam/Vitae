// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin developers
// Copyright (c) 2014-2015 The Dash developers
// Copyright (c) 2015-2017 The PIVX developers
// Copyright (c) 2018 The VITAE developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_WALLET_H
#define BITCOIN_WALLET_H

#include "amount.h"
#include "base58.h"
#include "crypter.h"
#include "kernel.h"
#include "key.h"
#include "keystore.h"
#include "main.h"
#include "primitives/block.h"
#include "primitives/transaction.h"
#include "zvit/zerocoin.h"
#include "ui_interface.h"
#include "util.h"
#include "validationinterface.h"
#include "wallet_ismine.h"
#include "walletdb.h"
#include "zvit/zvitmodule.h"
#include "zvit/zvitwallet.h"
#include "zvit/zvittracker.h"

#include <algorithm>
#include <map>
#include <set>
#include <stdexcept>
#include <stdint.h>
#include <string>
#include <utility>
#include <vector>

/**
 * Settings
 */
extern CFeeRate payTxFee;
extern CAmount maxTxFee;
extern unsigned int nTxConfirmTarget;
extern bool bSpendZeroConfChange;
extern bool bdisableSystemnotifications;
extern bool fSendFreeTransactions;
extern bool fPayAtLeastCustomFee;
extern bool fGlobalUnlockSpendCache; // Bool used for letting the precomputing thread know that zvitspends need to use the cs_spendcache

//! -paytxfee default
static const CAmount DEFAULT_TRANSACTION_FEE = 0;
//! -paytxfee will warn if called with a higher fee than this amount (in satoshis) per KB
static const CAmount nHighTransactionFeeWarning = 0.1 * COIN;
//! -maxtxfee default
static const CAmount DEFAULT_TRANSACTION_MAXFEE = 1 * COIN;
//! -maxtxfee will warn if called with a higher fee than this amount (in satoshis)
static const CAmount nHighTransactionMaxFeeWarning = 100 * nHighTransactionFeeWarning;
//! Largest (in bytes) free transaction we're willing to create
static const unsigned int MAX_FREE_TRANSACTION_CREATE_SIZE = 1000;
//! -custombackupthreshold default
static const int DEFAULT_CUSTOMBACKUPTHRESHOLD = 1;
//! -enableautoconvertaddress default
static const bool DEFAULT_AUTOCONVERTADDRESS = true;

// Zerocoin denomination which creates exactly one of each denominations:
// 6666 = 1*5000 + 1*1000 + 1*500 + 1*100 + 1*50 + 1*10 + 1*5 + 1
static const int ZQ_6666 = 6666;

class CAccountingEntry;
class CCoinControl;
class COutput;
class CReserveKey;
class CScript;
class CWalletTx;

/** (client) version numbers for particular wallet features */
enum WalletFeature {
    FEATURE_BASE = 10500, // the earliest version new wallets supports (only useful for getinfo's clientversion output)

    FEATURE_WALLETCRYPT = 40000, // wallet encryption
    FEATURE_COMPRPUBKEY = 60000, // compressed public keys

    FEATURE_LATEST = 61000
};

enum AvailableCoinsType {
    ALL_COINS = 1,
    ONLY_DENOMINATED = 2,
    ONLY_NOT10000IFMN = 3,
    ONLY_NONDENOMINATED_NOT10000IFMN = 4, // ONLY_NONDENOMINATED and not 10000 VITAE at the same time
    ONLY_10000 = 5,                        // find fundamentalnode outputs including locked ones (use with caution)
    STAKABLE_COINS = 6                          // UTXO's that are valid for staking
};

// Possible states for zVITAE send
enum ZerocoinSpendStatus {
    ZVIT_SPEND_OKAY = 0,                            // No error
    ZVIT_SPEND_ERROR = 1,                           // Unspecified class of errors, more details are (hopefully) in the returning text
    ZVIT_WALLET_LOCKED = 2,                         // Wallet was locked
    ZVIT_COMMIT_FAILED = 3,                         // Commit failed, reset status
    ZVIT_ERASE_SPENDS_FAILED = 4,                   // Erasing spends during reset failed
    ZVIT_ERASE_NEW_MINTS_FAILED = 5,                // Erasing new mints during reset failed
    ZVIT_TRX_FUNDS_PROBLEMS = 6,                    // Everything related to available funds
    ZVIT_TRX_CREATE = 7,                            // Everything related to create the transaction
    ZVIT_TRX_CHANGE = 8,                            // Everything related to transaction change
    ZVIT_TXMINT_GENERAL = 9,                        // General errors in MintToTxIn
    ZVIT_INVALID_COIN = 10,                         // Selected mint coin is not valid
    ZVIT_FAILED_ACCUMULATOR_INITIALIZATION = 11,    // Failed to initialize witness
    ZVIT_INVALID_WITNESS = 12,                      // Spend coin transaction did not verify
    ZVIT_BAD_SERIALIZATION = 13,                    // Transaction verification failed
    ZVIT_SPENT_USED_ZVIT = 14,                      // Coin has already been spend
    ZVIT_TX_TOO_LARGE = 15,                          // The transaction is larger than the max tx size
    ZVIT_SPEND_V1_SEC_LEVEL
};

struct CompactTallyItem {
    CBitcoinAddress address;
    CAmount nAmount;
    std::vector<CTxIn> vecTxIn;
    CompactTallyItem()
    {
        nAmount = 0;
    }
};

/** A key pool entry */
class CKeyPool
{
public:
    int64_t nTime;
    CPubKey vchPubKey;

    CKeyPool();
    CKeyPool(const CPubKey& vchPubKeyIn);

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion)
    {
        if (!(nType & SER_GETHASH))
            READWRITE(nVersion);
        READWRITE(nTime);
        READWRITE(vchPubKey);
    }
};

/** Address book data */
class CAddressBookData
{
public:
    std::string name;
    std::string purpose;

    CAddressBookData()
    {
        purpose = "unknown";
    }

    typedef std::map<std::string, std::string> StringMap;
    StringMap destdata;
};

/**
 * A CWallet is an extension of a keystore, which also maintains a set of transactions and balances,
 * and provides the ability to create new transactions.
 */
class CWallet : public CCryptoKeyStore, public CValidationInterface
{
private:
    bool SelectCoins(const CAmount& nTargetValue, std::set<std::pair<const CWalletTx*, unsigned int> >& setCoinsRet, CAmount& nValueRet, const CCoinControl* coinControl = NULL, AvailableCoinsType coin_type = ALL_COINS, bool useIX = true) const;
    //it was public bool SelectCoins(int64_t nTargetValue, std::set<std::pair<const CWalletTx*,unsigned int> >& setCoinsRet, int64_t& nValueRet, const CCoinControl *coinControl = NULL, AvailableCoinsType coin_type=ALL_COINS, bool useIX = true) const;

    CWalletDB* pwalletdbEncryption;

    //! the current wallet version: clients below this version are not able to load the wallet
    int nWalletVersion;

    //! the maximum wallet format version: memory-only variable that specifies to what version this wallet may be upgraded
    int nWalletMaxVersion;

    int64_t nNextResend;
    int64_t nLastResend;

    /**
     * Used to keep track of spent outpoints, and
     * detect and report conflicts (double-spends or
     * mutated transactions where the mutant gets mined).
     */
    typedef std::multimap<COutPoint, uint256> TxSpends;
    TxSpends mapTxSpends;
    void AddToSpends(const COutPoint& outpoint, const uint256& wtxid);
    void AddToSpends(const uint256& wtxid);

    void SyncMetaData(std::pair<TxSpends::iterator, TxSpends::iterator>);

public:

    static const int STAKE_SPLIT_THRESHOLD = 2000;

    bool MintableCoins();
    bool SelectStakeCoins(std::list<std::unique_ptr<CStakeInput> >& listInputs, CAmount nTargetAmount, int blockHeight, bool fPrecompute = false);
    bool IsCollateralAmount(CAmount nInputAmount) const;

    // Zerocoin additions
    bool CreateZerocoinMintTransaction(const CAmount nValue, CMutableTransaction& txNew, std::vector<CDeterministicMint>& vDMints, CReserveKey* reservekey, int64_t& nFeeRet, std::string& strFailReason, const CCoinControl* coinControl = NULL, const bool isZCSpendChange = false);

    bool CreateZerocoinSpendTransaction(
            CAmount nValue,
            CWalletTx& wtxNew,
            CReserveKey& reserveKey,
            CZerocoinSpendReceipt& receipt,
            std::vector<CZerocoinMint>& vSelectedMints,
            std::vector<CDeterministicMint>& vNewMints,
            bool fMintChange,
            bool fMinimizeChange,
            std::list<std::pair<CBitcoinAddress*,CAmount>> addressesTo,
            CBitcoinAddress* changeAddress = nullptr,
            bool isPublicSpend = true);

    bool CheckCoinSpend(libzerocoin::CoinSpend& spend, libzerocoin::Accumulator& accumulator, CZerocoinSpendReceipt& receipt);
    bool MintToTxIn(CZerocoinMint zerocoinSelected, const uint256& hashTxOut, CTxIn& newTxIn, CZerocoinSpendReceipt& receipt, libzerocoin::SpendType spendType, CBlockIndex* pindexCheckpoint = nullptr, bool publicCoinSpend = true);
    bool MintsToInputVector(std::map<CBigNum, CZerocoinMint>& mapMintsSelected, const uint256& hashTxOut, std::vector<CTxIn>& vin,
                            CZerocoinSpendReceipt& receipt, libzerocoin::SpendType spendType, CBlockIndex* pindexCheckpoint = nullptr);
    // Public coin spend input creation
    bool MintsToInputVectorPublicSpend(std::map<CBigNum, CZerocoinMint>& mapMintsSelected, const uint256& hashTxOut, std::vector<CTxIn>& vin,
                                       CZerocoinSpendReceipt& receipt, libzerocoin::SpendType spendType, CBlockIndex* pindexCheckpoint = nullptr);

    std::string MintZerocoinFromOutPoint(CAmount nValue, CWalletTx& wtxNew, std::vector<CDeterministicMint>& vDMints, const std::vector<COutPoint> vOutpts);
    std::string MintZerocoin(CAmount nValue, CWalletTx& wtxNew, std::vector<CDeterministicMint>& vDMints, const CCoinControl* coinControl = NULL);

    bool SpendZerocoin(
            CAmount nAmount,
            CWalletTx& wtxNew,
            CZerocoinSpendReceipt& receipt,
            std::vector<CZerocoinMint>& vMintsSelected,
            bool fMintChange,
            bool fMinimizeChange,
            std::list<std::pair<CBitcoinAddress*,CAmount>> addressesTo,
            CBitcoinAddress* changeAddress = nullptr,
            bool isPublicSpend = true
    );

    std::string ResetMintZerocoin();
    std::string ResetSpentZerocoin();
    void ReconsiderZerocoins(std::list<CZerocoinMint>& listMintsRestored, std::list<CDeterministicMint>& listDMintsRestored);
    void ZVitBackupWallet();
    bool GetZerocoinKey(const CBigNum& bnSerial, CKey& key);
    bool CreateZVITOutPut(libzerocoin::CoinDenomination denomination, CTxOut& outMint, CDeterministicMint& dMint);
    bool GetMint(const uint256& hashSerial, CZerocoinMint& mint);
    bool GetMintFromStakeHash(const uint256& hashStake, CZerocoinMint& mint);
    bool DatabaseMint(CDeterministicMint& dMint);
    bool SetMintUnspent(const CBigNum& bnSerial);
    bool UpdateMint(const CBigNum& bnValue, const int& nHeight, const uint256& txid, const libzerocoin::CoinDenomination& denom);
    std::string GetUniqueWalletBackupName(bool fzvitAuto) const;
    void InitAutoConvertAddresses();


    /** Zerocin entry changed.
    * @note called with lock cs_wallet held.
    */
    boost::signals2::signal<void(CWallet* wallet, const std::string& pubCoin, const std::string& isUsed, ChangeType status)> NotifyZerocoinChanged;
    /*
     * Main wallet lock.
     * This lock protects all the fields added by CWallet
     *   except for:
     *      fFileBacked (immutable after instantiation)
     *      strWalletFile (immutable after instantiation)
     */
    mutable CCriticalSection cs_wallet;

    CzVITWallet* zwalletMain;

    std::set<CBitcoinAddress> setAutoConvertAddresses;

    bool fFileBacked;
    bool fWalletUnlockAnonymizeOnly;
    std::string strWalletFile;
    bool fBackupMints;
    std::unique_ptr<CzVITTracker> zvitTracker;

    std::set<int64_t> setKeyPool;
    std::map<CKeyID, CKeyMetadata> mapKeyMetadata;

    typedef std::map<unsigned int, CMasterKey> MasterKeyMap;
    MasterKeyMap mapMasterKeys;
    unsigned int nMasterKeyMaxID;

    // Stake Settings
    unsigned int nHashDrift;
    unsigned int nHashInterval;
    uint64_t nStakeSplitThreshold;
    int nStakeSetUpdateTime;

    //MultiSend
    std::vector<std::pair<std::string, int> > vMultiSend;
    bool fMultiSendStake;
    bool fMultiSendFundamentalnodeReward;
    bool fMultiSendMasternodeReward;
    bool fMultiSendNotify;
    std::string strMultiSendChangeAddress;
    int nLastMultiSendHeight;
    std::vector<std::string> vDisabledAddresses;

    //Auto Combine Inputs
    bool fCombineDust;
    CAmount nAutoCombineThreshold;

    CWallet();
    CWallet(std::string strWalletFileIn);
    ~CWallet();
    void SetNull();
    int getZeromintPercentage();
    void setZWallet(CzVITWallet* zwallet);
    CzVITWallet* getZWallet();
    bool isZeromintEnabled();
    void setZVitAutoBackups(bool fEnabled);
    bool isMultiSendEnabled();
    void setMultiSendDisabled();

    std::map<uint256, CWalletTx> mapWallet;
    std::list<CAccountingEntry> laccentries;

    typedef std::pair<CWalletTx*, CAccountingEntry*> TxPair;
    typedef std::multimap<int64_t, TxPair > TxItems;
    TxItems wtxOrdered;

    int64_t nOrderPosNext;
    std::map<uint256, int> mapRequestCount;

    std::map<CTxDestination, CAddressBookData> mapAddressBook;

    CPubKey vchDefaultKey;

    std::set<COutPoint> setLockedCoins;

    int64_t nTimeFirstKey;

    const CWalletTx* GetWalletTx(const uint256& hash) const;

    std::vector<CWalletTx> getWalletTxs();

    void PrecomputeSpends();

    //! check whether we are allowed to upgrade (or already support) to the named feature
    bool CanSupportFeature(enum WalletFeature wf);

    void AvailableCoins(std::vector<COutput>& vCoins, bool fOnlyConfirmed = true, const CCoinControl* coinControl = NULL, bool fIncludeZeroValue = false, AvailableCoinsType nCoinType = ALL_COINS, bool fUseIX = false, int nWatchonlyConfig = 1) const;
    std::map<CBitcoinAddress, std::vector<COutput> > AvailableCoinsByAddress(bool fConfirmed = true, CAmount maxCoinValue = 0);
    bool SelectCoinsMinConf(const CAmount& nTargetValue, int nConfMine, int nConfTheirs, std::vector<COutput> vCoins, std::set<std::pair<const CWalletTx*, unsigned int> >& setCoinsRet, CAmount& nValueRet) const;

    /// Get 1000DASH output and keys which can be used for the Fundamentalnode
    bool GetFundamentalnodeVinAndKeys(CTxIn& txinRet, CPubKey& pubKeyRet, CKey& keyRet, std::string strTxHash = "", std::string strOutputIndex = "");
    /// Extract txin information and keys from output
    bool GetVinAndKeysFromOutput(COutput out, CTxIn& txinRet, CPubKey& pubKeyRet, CKey& keyRet);

    bool IsSpent(const uint256& hash, unsigned int n) const;

    bool IsLockedCoin(uint256 hash, unsigned int n) const;
    void LockCoin(COutPoint& output);
    void UnlockCoin(COutPoint& output);
    void UnlockAllCoins();
    void ListLockedCoins(std::vector<COutPoint>& vOutpts);

    //  keystore implementation
    // Generate a new key
    CPubKey GenerateNewKey();
    CBitcoinAddress GenerateNewAutoMintKey();
    int64_t GetKeyCreationTime(CPubKey pubkey);
    int64_t GetKeyCreationTime(const CBitcoinAddress& address);

    //! Adds a key to the store, and saves it to disk.
    bool AddKeyPubKey(const CKey& key, const CPubKey& pubkey);
    //! Adds a key to the store, without saving it to disk (used by LoadWallet)
    bool LoadKey(const CKey& key, const CPubKey& pubkey) { return CCryptoKeyStore::AddKeyPubKey(key, pubkey); }
    //! Load metadata (used by LoadWallet)
    bool LoadKeyMetadata(const CPubKey& pubkey, const CKeyMetadata& metadata);

    bool LoadMinVersion(int nVersion);

    //! Adds an encrypted key to the store, and saves it to disk.
    bool AddCryptedKey(const CPubKey& vchPubKey, const std::vector<unsigned char>& vchCryptedSecret);
    //! Adds an encrypted key to the store, without saving it to disk (used by LoadWallet)
    bool LoadCryptedKey(const CPubKey& vchPubKey, const std::vector<unsigned char>& vchCryptedSecret);
    bool AddCScript(const CScript& redeemScript);
    bool LoadCScript(const CScript& redeemScript);

    //! Adds a destination data tuple to the store, and saves it to disk
    bool AddDestData(const CTxDestination& dest, const std::string& key, const std::string& value);
    //! Erases a destination data tuple in the store and on disk
    bool EraseDestData(const CTxDestination& dest, const std::string& key);
    //! Adds a destination data tuple to the store, without saving it to disk
    bool LoadDestData(const CTxDestination& dest, const std::string& key, const std::string& value);

    //! Adds a watch-only address to the store, and saves it to disk.
    bool AddWatchOnly(const CScript& dest);
    bool RemoveWatchOnly(const CScript& dest);
    //! Adds a watch-only address to the store, without saving it to disk (used by LoadWallet)
    bool LoadWatchOnly(const CScript& dest);

    //! Adds a MultiSig address to the store, and saves it to disk.
    bool AddMultiSig(const CScript& dest);
    bool RemoveMultiSig(const CScript& dest);
    //! Adds a MultiSig address to the store, without saving it to disk (used by LoadWallet)
    bool LoadMultiSig(const CScript& dest);

    bool Unlock(const SecureString& strWalletPassphrase, bool anonimizeOnly = false);
    bool ChangeWalletPassphrase(const SecureString& strOldWalletPassphrase, const SecureString& strNewWalletPassphrase);
    bool EncryptWallet(const SecureString& strWalletPassphrase);

    void GetKeyBirthTimes(std::map<CKeyID, int64_t>& mapKeyBirth) const;
    unsigned int ComputeTimeSmart(const CWalletTx& wtx) const;

    /**
     * Increment the next transaction order id
     * @return next transaction order id
     */
    int64_t IncOrderPosNext(CWalletDB* pwalletdb = NULL);

    void MarkDirty();
    bool AddToWallet(const CWalletTx& wtxIn, bool fFromLoadWallet, CWalletDB* pwalletdb);
    void SyncTransaction(const CTransaction& tx, const CBlock* pblock);
    bool AddToWalletIfInvolvingMe(const CTransaction& tx, const CBlock* pblock, bool fUpdate);
    void EraseFromWallet(const uint256& hash);
    int ScanForWalletTransactions(CBlockIndex* pindexStart, bool fUpdate = false, bool fromStartup = false);
    void ReacceptWalletTransactions();
    void ResendWalletTransactions();
    CAmount GetBalance() const;
    CAmount GetZerocoinBalance(bool fMatureOnly) const;
    CAmount GetUnconfirmedZerocoinBalance() const;
    CAmount GetImmatureZerocoinBalance() const;
    CAmount GetLockedCoins() const;
    CAmount GetUnlockedCoins() const;
    std::map<libzerocoin::CoinDenomination, CAmount> GetMyZerocoinDistribution() const;
    CAmount GetUnconfirmedBalance() const;
    CAmount GetImmatureBalance() const;
    CAmount GetWatchOnlyBalance() const;
    CAmount GetUnconfirmedWatchOnlyBalance() const;
    CAmount GetImmatureWatchOnlyBalance() const;
    CAmount GetLockedWatchOnlyBalance() const;
    bool CreateTransaction(CScript scriptPubKey, int64_t nValue, CWalletTx& wtxNew, CReserveKey& reservekey, int64_t& nFeeRet, std::string& strFailReason, const CCoinControl* coinControl, bool IsFundamentalNodePayment = false);
    bool CreateTransaction(const std::vector<std::pair<CScript, CAmount> >& vecSend,
        CWalletTx& wtxNew,
        CReserveKey& reservekey,
        CAmount& nFeeRet,
        std::string& strFailReason,
        const CCoinControl* coinControl = NULL,
        AvailableCoinsType coin_type = ALL_COINS,
        bool useIX = false,
        CAmount nFeePay = 0,
        bool IsFundamentalNodePayment = false);
    bool CreateTransaction(CScript scriptPubKey, const CAmount& nValue, CWalletTx& wtxNew, CReserveKey& reservekey, CAmount& nFeeRet, std::string& strFailReason, const CCoinControl* coinControl = NULL, AvailableCoinsType coin_type = ALL_COINS, bool useIX = false, CAmount nFeePay = 0,
                           bool IsFundamentalNodePayment = false);
    bool CommitTransaction(CWalletTx& wtxNew, CReserveKey& reservekey, std::string strCommand = "tx");
    bool AddAccountingEntry(const CAccountingEntry&, CWalletDB & pwalletdb);
    int GenerateObfuscationOutputs(int nTotalValue, std::vector<CTxOut>& vout);
    bool CreateCoinStake(const CKeyStore& keystore, unsigned int nBits, int64_t nSearchInterval, CMutableTransaction& txNew, unsigned int& nTxNewTime);
    bool MultiSend();
    void AutoCombineDust();
    void AutoZeromint();
    void AutoZeromintForAddress();
    void CreateAutoMintTransaction(const CAmount& nMintAmount, CCoinControl* coinControl = nullptr);

    static CFeeRate minTxFee;
    static CAmount GetMinimumFee(unsigned int nTxBytes, unsigned int nConfirmTarget, const CTxMemPool& pool);

    bool NewKeyPool();
    bool TopUpKeyPool(unsigned int kpSize = 0);
    void ReserveKeyFromKeyPool(int64_t& nIndex, CKeyPool& keypool);
    void KeepKey(int64_t nIndex);
    void ReturnKey(int64_t nIndex);
    bool GetKeyFromPool(CPubKey& key);
    int64_t GetOldestKeyPoolTime();
    void GetAllReserveKeys(std::set<CKeyID>& setAddress) const;

    std::set<std::set<CTxDestination> > GetAddressGroupings();
    std::map<CTxDestination, CAmount> GetAddressBalances();

    std::set<CTxDestination> GetAccountAddresses(std::string strAccount) const;

    bool GetBudgetSystemCollateralTX(CWalletTx& tx, uint256 hash, bool useIX);
    bool GetBudgetFinalizationCollateralTX(CWalletTx& tx, uint256 hash, bool useIX); // Only used for budget finalization 

    bool IsDenominated(const CTxIn& txin) const;

    bool IsDenominatedAmount(CAmount nInputAmount) const;

    bool IsUsed(const CBitcoinAddress address) const;

    isminetype IsMine(const CTxIn& txin) const;
    CAmount GetDebit(const CTxIn& txin, const isminefilter& filter) const;
    isminetype IsMine(const CTxOut& txout) const;
    bool IsMyZerocoinSpend(const CBigNum& bnSerial) const;
    bool IsMyMint(const CBigNum& bnValue) const;
    CAmount GetCredit(const CTxOut& txout, const isminefilter& filter) const;
    bool IsChange(const CTxOut& txout) const;
    CAmount GetChange(const CTxOut& txout) const;
    bool IsMine(const CTransaction& tx) const;
    /** should probably be renamed to IsRelevantToMe */
    bool IsFromMe(const CTransaction& tx) const;
    CAmount GetDebit(const CTransaction& tx, const isminefilter& filter) const;
    CAmount GetCredit(const CTransaction& tx, const isminefilter& filter) const;
    CAmount GetChange(const CTransaction& tx) const;
    void SetBestChain(const CBlockLocator& loc);

    DBErrors LoadWallet(bool& fFirstRunRet);
    DBErrors ZapWalletTx(std::vector<CWalletTx>& vWtx);

    bool SetAddressBook(const CTxDestination& address, const std::string& strName, const std::string& purpose);

    bool DelAddressBook(const CTxDestination& address);

    bool UpdatedTransaction(const uint256& hashTx);

    void Inventory(const uint256& hash);

    unsigned int GetKeyPoolSize();

    bool SetDefaultKey(const CPubKey& vchPubKey);

    //! signify that a particular wallet feature is now used. this may change nWalletVersion and nWalletMaxVersion if those are lower
    bool SetMinVersion(enum WalletFeature, CWalletDB* pwalletdbIn = NULL, bool fExplicit = false);

    //! change which version we're allowed to upgrade to (note that this does not immediately imply upgrading to that format)
    bool SetMaxVersion(int nVersion);

    //! get the current wallet format (the oldest client version guaranteed to understand this wallet)
    int GetVersion();

    //! Get wallet transactions that conflict with given transaction (spend same outputs)
    std::set<uint256> GetConflicts(const uint256& txid) const;

    /**
     * Address book entry changed.
     * @note called with lock cs_wallet held.
     */
    boost::signals2::signal<void(CWallet* wallet, const CTxDestination& address, const std::string& label, bool isMine, const std::string& purpose, ChangeType status)> NotifyAddressBookChanged;

    /**
     * Wallet transaction added, removed or updated.
     * @note called with lock cs_wallet held.
     */
    boost::signals2::signal<void(CWallet* wallet, const uint256& hashTx, ChangeType status)> NotifyTransactionChanged;

    /** Show progress e.g. for rescan */
    boost::signals2::signal<void(const std::string& title, int nProgress)> ShowProgress;

    /** Watch-only address added */
    boost::signals2::signal<void(bool fHaveWatchOnly)> NotifyWatchonlyChanged;

    /** MultiSig address added */
    boost::signals2::signal<void(bool fHaveMultiSig)> NotifyMultiSigChanged;

    /** zVIT reset */
    boost::signals2::signal<void()> NotifyzPIVReset;

    /** notify wallet file backed up */
    boost::signals2::signal<void (const bool& fSuccess, const std::string& filename)> NotifyWalletBacked;
};


/** A key allocated from the key pool. */
class CReserveKey
{
protected:
    CWallet* pwallet;
    int64_t nIndex;
    CPubKey vchPubKey;

public:
    CReserveKey(CWallet* pwalletIn)
    {
        nIndex = -1;
        pwallet = pwalletIn;
    }

    ~CReserveKey()
    {
        ReturnKey();
    }

    void ReturnKey();
    bool GetReservedKey(CPubKey& pubkey);
    void KeepKey();
};


typedef std::map<std::string, std::string> mapValue_t;


static void ReadOrderPos(int64_t& nOrderPos, mapValue_t& mapValue)
{
    if (!mapValue.count("n")) {
        nOrderPos = -1; // TODO: calculate elsewhere
        return;
    }
    nOrderPos = atoi64(mapValue["n"].c_str());
}


static void WriteOrderPos(const int64_t& nOrderPos, mapValue_t& mapValue)
{
    if (nOrderPos == -1)
        return;
    mapValue["n"] = i64tostr(nOrderPos);
}

struct COutputEntry {
    CTxDestination destination;
    CAmount amount;
    int vout;
};

/** A transaction with a merkle branch linking it to the block chain. */
class CMerkleTx : public CTransaction
{
private:
    int GetDepthInMainChainINTERNAL(const CBlockIndex*& pindexRet) const;

public:
    uint256 hashBlock;
    int nIndex;

    CMerkleTx()
    {
        Init();
    }

    CMerkleTx(const CTransaction& txIn) : CTransaction(txIn)
    {
        Init();
    }

    void Init()
    {
        hashBlock = 0;
        nIndex = -1;
    }

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion)
    {
        std::vector<uint256> vMerkleBranch; // For compatibility with older versions.
        READWRITE(*(CTransaction*)this);
        nVersion = this->nVersion;
        READWRITE(hashBlock);
        READWRITE(vMerkleBranch);
        READWRITE(nIndex);
    }

    int SetMerkleBranch(const CBlock& block);


    /**
     * Return depth of transaction in blockchain:
     * -1  : not in blockchain, and not in memory pool (conflicted transaction)
     *  0  : in memory pool, waiting to be included in a block
     * >=1 : this many blocks deep in the main chain
     */
    int GetDepthInMainChain(const CBlockIndex*& pindexRet, bool enableIX = true) const;
    int GetDepthInMainChain(bool enableIX = true) const
    {
        const CBlockIndex* pindexRet;
        return GetDepthInMainChain(pindexRet, enableIX);
    }
    bool IsInMainChain() const
    {
        const CBlockIndex* pindexRet;
        return GetDepthInMainChainINTERNAL(pindexRet) > 0;
    }
    int GetBlocksToMaturity() const;
    bool AcceptToMemoryPool(bool fLimitFree = true, bool fRejectInsaneFee = true, bool ignoreFees = false);
    int GetTransactionLockSignatures() const;
    bool IsTransactionLockTimedOut() const;
};

/**
 * A transaction with a bunch of additional info that only the owner cares about.
 * It includes any unrecorded transactions needed to link it back to the block chain.
 */
class CWalletTx : public CMerkleTx
{
private:
    const CWallet* pwallet;

public:
    mapValue_t mapValue;
    std::vector<std::pair<std::string, std::string> > vOrderForm;
    unsigned int fTimeReceivedIsTxTime;
    unsigned int nTimeReceived; //! time received by this node
    unsigned int nTimeSmart;
    char fFromMe;
    std::string strFromAccount;
    int64_t nOrderPos; //! position in ordered transaction list

    // memory only
    mutable bool fDebitCached;
    mutable bool fCreditCached;
    mutable bool fImmatureCreditCached;
    mutable bool fAvailableCreditCached;
    mutable bool fAnonymizableCreditCached;
    mutable bool fAnonymizedCreditCached;
    mutable bool fDenomUnconfCreditCached;
    mutable bool fDenomConfCreditCached;
    mutable bool fWatchDebitCached;
    mutable bool fWatchCreditCached;
    mutable bool fImmatureWatchCreditCached;
    mutable bool fAvailableWatchCreditCached;
    mutable bool fChangeCached;
    mutable CAmount nDebitCached;
    mutable CAmount nCreditCached;
    mutable CAmount nImmatureCreditCached;
    mutable CAmount nAvailableCreditCached;
    mutable CAmount nAnonymizableCreditCached;
    mutable CAmount nAnonymizedCreditCached;
    mutable CAmount nDenomUnconfCreditCached;
    mutable CAmount nDenomConfCreditCached;
    mutable CAmount nWatchDebitCached;
    mutable CAmount nWatchCreditCached;
    mutable CAmount nImmatureWatchCreditCached;
    mutable CAmount nAvailableWatchCreditCached;
    mutable CAmount nChangeCached;

    CWalletTx();
    CWalletTx(const CWallet* pwalletIn);
    CWalletTx(const CWallet* pwalletIn, const CMerkleTx& txIn);
    CWalletTx(const CWallet* pwalletIn, const CTransaction& txIn);
    void Init(const CWallet* pwalletIn);

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion)
    {
        if (ser_action.ForRead())
            Init(NULL);
        char fSpent = false;

        if (!ser_action.ForRead()) {
            mapValue["fromaccount"] = strFromAccount;

            WriteOrderPos(nOrderPos, mapValue);

            if (nTimeSmart)
                mapValue["timesmart"] = strprintf("%u", nTimeSmart);
        }

        READWRITE(*(CMerkleTx*)this);
        std::vector<CMerkleTx> vUnused; //! Used to be vtxPrev
        READWRITE(vUnused);
        READWRITE(mapValue);
        READWRITE(vOrderForm);
        READWRITE(fTimeReceivedIsTxTime);
        READWRITE(nTimeReceived);
        READWRITE(fFromMe);
        READWRITE(fSpent);

        if (ser_action.ForRead()) {
            strFromAccount = mapValue["fromaccount"];

            ReadOrderPos(nOrderPos, mapValue);

            nTimeSmart = mapValue.count("timesmart") ? (unsigned int)atoi64(mapValue["timesmart"]) : 0;
        }

        mapValue.erase("fromaccount");
        mapValue.erase("version");
        mapValue.erase("spent");
        mapValue.erase("n");
        mapValue.erase("timesmart");
    }

    //! make sure balances are recalculated
    void MarkDirty();

    void BindWallet(CWallet* pwalletIn);

    //! filter decides which addresses will count towards the debit
    CAmount GetDebit(const isminefilter& filter) const;
    CAmount GetCredit(const isminefilter& filter) const;
    CAmount GetImmatureCredit(bool fUseCache = true) const;
    CAmount GetAvailableCredit(bool fUseCache = true) const;
    // Return sum of unlocked coins
    CAmount GetUnlockedCredit() const;
    // Return sum of unlocked coins
    CAmount GetLockedCredit() const;
    CAmount GetImmatureWatchOnlyCredit(const bool& fUseCache = true) const;
    CAmount GetAvailableWatchOnlyCredit(const bool& fUseCache = true) const;
    CAmount GetLockedWatchOnlyCredit() const;
    CAmount GetChange() const;

    void GetAmounts(std::list<COutputEntry>& listReceived,
        std::list<COutputEntry>& listSent,
        CAmount& nFee,
        std::string& strSentAccount,
        const isminefilter& filter) const;

    void GetAccountAmounts(const std::string& strAccount, CAmount& nReceived, CAmount& nSent, CAmount& nFee, const isminefilter& filter) const;

    bool IsFromMe(const isminefilter& filter) const;

    bool InMempool() const;

    bool IsTrusted() const;

    bool WriteToDisk(CWalletDB *pwalletdb);

    int64_t GetTxTime() const;
    int64_t GetComputedTxTime() const;
    int GetRequestCount() const;
    void RelayWalletTransaction(std::string strCommand = "tx");

    std::set<uint256> GetConflicts() const;
};


class COutput
{
public:
    const CWalletTx* tx;
    int i;
    int nDepth;
    bool fSpendable;

    COutput(const CWalletTx* txIn, int iIn, int nDepthIn, bool fSpendableIn)
    {
        tx = txIn;
        i = iIn;
        nDepth = nDepthIn;
        fSpendable = fSpendableIn;
    }

    //Used with Obfuscation. Will return largest nondenom, then denominations, then very small inputs
    int Priority() const
    {
        for (CAmount d : obfuScationDenominations)
            if (tx->vout[i].nValue == d) return 10000;
        if (tx->vout[i].nValue < 1 * COIN) return 20000;

        //nondenom return largest first
        return -(tx->vout[i].nValue / COIN);
    }

    CAmount Value() const
    {
        return tx->vout[i].nValue;
    }

    std::string ToString() const;
};


/** Private key that includes an expiration date in case it never gets used. */
class CWalletKey
{
public:
    CPrivKey vchPrivKey;
    int64_t nTimeCreated;
    int64_t nTimeExpires;
    std::string strComment;
    //! todo: add something to note what created it (user, getnewaddress, change)
    //!   maybe should have a map<string, string> property map

    CWalletKey(int64_t nExpires = 0);

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion)
    {
        if (!(nType & SER_GETHASH))
            READWRITE(nVersion);
        READWRITE(vchPrivKey);
        READWRITE(nTimeCreated);
        READWRITE(nTimeExpires);
        READWRITE(LIMITED_STRING(strComment, 65536));
    }
};


/**
 * Account information.
 * Stored in wallet with key "acc"+string account name.
 */
class CAccount
{
public:
    CPubKey vchPubKey;

    CAccount()
    {
        SetNull();
    }

    void SetNull()
    {
        vchPubKey = CPubKey();
    }

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion)
    {
        if (!(nType & SER_GETHASH))
            READWRITE(nVersion);
        READWRITE(vchPubKey);
    }
};


/**
 * Internal transfers.
 * Database key is acentry<account><counter>.
 */
class CAccountingEntry
{
public:
    std::string strAccount;
    CAmount nCreditDebit;
    int64_t nTime;
    std::string strOtherAccount;
    std::string strComment;
    mapValue_t mapValue;
    int64_t nOrderPos; //! position in ordered transaction list
    uint64_t nEntryNo;

    CAccountingEntry()
    {
        SetNull();
    }

    void SetNull()
    {
        nCreditDebit = 0;
        nTime = 0;
        strAccount.clear();
        strOtherAccount.clear();
        strComment.clear();
        nOrderPos = -1;
        nEntryNo = 0;
    }

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion)
    {
        if (!(nType & SER_GETHASH))
            READWRITE(nVersion);
        //! Note: strAccount is serialized as part of the key, not here.
        READWRITE(nCreditDebit);
        READWRITE(nTime);
        READWRITE(LIMITED_STRING(strOtherAccount, 65536));

        if (!ser_action.ForRead()) {
            WriteOrderPos(nOrderPos, mapValue);

            if (!(mapValue.empty() && _ssExtra.empty())) {
                CDataStream ss(nType, nVersion);
                ss.insert(ss.begin(), '\0');
                ss << mapValue;
                ss.insert(ss.end(), _ssExtra.begin(), _ssExtra.end());
                strComment.append(ss.str());
            }
        }

        READWRITE(LIMITED_STRING(strComment, 65536));

        size_t nSepPos = strComment.find("\0", 0, 1);
        if (ser_action.ForRead()) {
            mapValue.clear();
            if (std::string::npos != nSepPos) {
                CDataStream ss(std::vector<char>(strComment.begin() + nSepPos + 1, strComment.end()), nType, nVersion);
                ss >> mapValue;
                _ssExtra = std::vector<char>(ss.begin(), ss.end());
            }
            ReadOrderPos(nOrderPos, mapValue);
        }
        if (std::string::npos != nSepPos)
            strComment.erase(nSepPos);

        mapValue.erase("n");
    }

private:
    std::vector<char> _ssExtra;
};

void ThreadPrecomputeSpends();

#endif // BITCOIN_WALLET_H
