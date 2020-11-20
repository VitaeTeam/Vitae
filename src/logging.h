// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2018 The Bitcoin developers
// Copyright (c) 2015-2020 The PIVX developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

/**
 * Server/client environment: argument handling, config file parsing,
 * logging, thread wrappers
 */
#ifndef BITCOIN_LOGGING_H
#define BITCOIN_LOGGING_H

#include "tinyformat.h"

#include <atomic>
#include <vector>

#include <boost/filesystem.hpp>

extern bool fPrintToConsole;
extern bool fPrintToDebugLog;

extern bool fLogTimestamps;
extern bool fLogIPs;
extern std::atomic<bool> fReopenDebugLog;

extern std::atomic<uint32_t> logCategories;

struct CLogCategoryActive
{
    std::string category;
    bool active;
};

namespace BCLog {
    enum LogFlags : uint32_t {
        NONE        = 0,
        NET         = (1 <<  0),
        TOR         = (1 <<  1),
        MEMPOOL     = (1 <<  2),
        HTTP        = (1 <<  3),
        BENCH       = (1 <<  4),
        ZMQ         = (1 <<  5),
        DB          = (1 <<  6),
        RPC         = (1 <<  7),
        ESTIMATEFEE = (1 <<  8),
        ADDRMAN     = (1 <<  9),
        SELECTCOINS = (1 << 10),
        REINDEX     = (1 << 11),
        CMPCTBLOCK  = (1 << 12),
        RAND        = (1 << 13),
        PRUNE       = (1 << 14),
        PROXY       = (1 << 15),
        MEMPOOLREJ  = (1 << 16),
        LIBEVENT    = (1 << 17),
        COINDB      = (1 << 18),
        QT          = (1 << 19),
        LEVELDB     = (1 << 20),
        STAKING     = (1 << 21),
        MASTERNODE  = (1 << 22),
        MNBUDGET    = (1 << 23),
        LEGACYZC    = (1 << 24),
        ALL         = ~(uint32_t)0,
    };
}

/** Return true if log accepts specified category */
static inline bool LogAcceptCategory(uint32_t category)
{
    return (logCategories.load(std::memory_order_relaxed) & category) != 0;
}

/** Returns a string with the supported log categories */
std::string ListLogCategories();

/** Returns a vector of the active log categories. */
std::vector<CLogCategoryActive> ListActiveLogCategories();

/** Return true if str parses as a log category and set the flags in f */
bool GetLogCategory(uint32_t *f, const std::string *str);

/** Send a string to the log output */
int LogPrintStr(const std::string& str);

/** Get format string from VA_ARGS for error reporting */
template<typename... Args> std::string FormatStringFromLogArgs(const char *fmt, const Args&... args) { return fmt; }

// Be conservative when using LogPrintf/error or other things which
// unconditionally log to debug.log! It should not be the case that an inbound
// peer can fill up a user's disk with debug.log entries.

#define LogPrintf(...) do {                                                         \
    std::string _log_msg_; /* Unlikely name to avoid shadowing variables */         \
    try {                                                                           \
        _log_msg_ = tfm::format(__VA_ARGS__);                                       \
    } catch (tinyformat::format_error &e) {                                               \
        /* Original format string will have newline so don't add one here */        \
        _log_msg_ = "Error \"" + std::string(e.what()) + "\" while formatting log message: " + FormatStringFromLogArgs(__VA_ARGS__); \
    }                                                                               \
    LogPrintStr(_log_msg_);                                                         \
} while(0)

#define LogPrint(category, ...) do {                                                \
    if (LogAcceptCategory((category))) {                                            \
        LogPrintf(__VA_ARGS__);                                                     \
    }                                                                               \
} while(0)

boost::filesystem::path GetDebugLogPath();
bool OpenDebugLog();
void ShrinkDebugFile();

#endif // BITCOIN_LOGGING_H
