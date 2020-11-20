// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2018 The Bitcoin developers
// Copyright (c) 2015-2020 The PIVX developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "chainparamsbase.h"
#include "logging.h"
#include "util.h"
#include "utilstrencodings.h"

#include <list>
#include <mutex>

#include <boost/filesystem/fstream.hpp>

const char * const DEFAULT_DEBUGLOGFILE = "debug.log";
namespace fs = boost::filesystem;

bool fPrintToConsole = false;
bool fPrintToDebugLog = true;
bool fLogTimestamps = false;
bool fLogIPs = false;
std::atomic<bool> fReopenDebugLog(false);

/** Log categories bitfield. Leveldb/libevent need special handling if their flags are changed at runtime. */
std::atomic<uint32_t> logCategories(0);

/**
 * LogPrintf() has been broken a couple of times now
 * by well-meaning people adding mutexes in the most straightforward way.
 * It breaks because it may be called by global destructors during shutdown.
 * Since the order of destruction of static/global objects is undefined,
 * defining a mutex as a global object doesn't work (the mutex gets
 * destroyed, and then some later destructor calls OutputDebugStringF,
 * maybe indirectly, and you get a core dump at shutdown trying to lock
 * the mutex).
 */

static boost::once_flag debugPrintInitFlag = BOOST_ONCE_INIT;

/**
 * We use boost::call_once() to make sure these are initialized
 * in a thread-safe manner the first time called:
 */
static FILE* fileout = nullptr;
static boost::mutex* mutexDebugLog = nullptr;
static std::list<std::string> *vMsgsBeforeOpenLog;

static int FileWriteStr(const std::string &str, FILE *fp)
{
    return fwrite(str.data(), 1, str.size(), fp);
}

static void DebugPrintInit()
{
    assert(mutexDebugLog == nullptr);
    mutexDebugLog = new boost::mutex();
    vMsgsBeforeOpenLog = new std::list<std::string>;
}

fs::path GetDebugLogPath()
{
    fs::path logfile(GetArg("-debuglogfile", DEFAULT_DEBUGLOGFILE));
    if (logfile.is_absolute()) {
        return logfile;
    } else {
        return GetDataDir() / logfile;
    }
}

bool OpenDebugLog()
{
    boost::call_once(&DebugPrintInit, debugPrintInitFlag);
    boost::mutex::scoped_lock scoped_lock(*mutexDebugLog);
    assert(fileout == nullptr);
    assert(vMsgsBeforeOpenLog);

    fs::path pathDebug = GetDebugLogPath();

    fileout = fopen(pathDebug.string().c_str(), "a");
    if (!fileout) return false;

    setbuf(fileout, nullptr); // unbuffered
    // dump buffered messages from before we opened the log
    while (!vMsgsBeforeOpenLog->empty()) {
        FileWriteStr(vMsgsBeforeOpenLog->front(), fileout);
        vMsgsBeforeOpenLog->pop_front();
    }

    delete vMsgsBeforeOpenLog;
    vMsgsBeforeOpenLog = nullptr;
    return true;
}

struct CLogCategoryDesc
{
    uint32_t flag;
    std::string category;
};

const CLogCategoryDesc LogCategories[] = {
        {BCLog::NONE,           "0"},
        {BCLog::NET,            "net"},
        {BCLog::TOR,            "tor"},
        {BCLog::MEMPOOL,        "mempool"},
        {BCLog::HTTP,           "http"},
        {BCLog::BENCH,          "bench"},
        {BCLog::ZMQ,            "zmq"},
        {BCLog::DB,             "db"},
        {BCLog::RPC,            "rpc"},
        {BCLog::ESTIMATEFEE,    "estimatefee"},
        {BCLog::ADDRMAN,        "addrman"},
        {BCLog::SELECTCOINS,    "selectcoins"},
        {BCLog::REINDEX,        "reindex"},
        {BCLog::CMPCTBLOCK,     "cmpctblock"},
        {BCLog::RAND,           "rand"},
        {BCLog::PRUNE,          "prune"},
        {BCLog::PROXY,          "proxy"},
        {BCLog::MEMPOOLREJ,     "mempoolrej"},
        {BCLog::LIBEVENT,       "libevent"},
        {BCLog::COINDB,         "coindb"},
        {BCLog::QT,             "qt"},
        {BCLog::LEVELDB,        "leveldb"},
        {BCLog::STAKING,        "staking"},
        {BCLog::MASTERNODE,     "masternode"},
        {BCLog::MNBUDGET,       "mnbudget"},
        {BCLog::LEGACYZC,       "zero"},
        {BCLog::ALL,            "1"},
        {BCLog::ALL,            "all"},
};

bool GetLogCategory(uint32_t *f, const std::string *str)
{
    if (f && str) {
        if (*str == "") {
            *f = BCLog::ALL;
            return true;
        }
        for (unsigned int i = 0; i < ARRAYLEN(LogCategories); i++) {
            if (LogCategories[i].category == *str) {
                *f = LogCategories[i].flag;
                return true;
            }
        }
    }
    return false;
}

std::string ListLogCategories()
{
    std::string ret;
    int outcount = 0;
    for (unsigned int i = 0; i < ARRAYLEN(LogCategories); i++) {
        // Omit the special cases.
        if (LogCategories[i].flag != BCLog::NONE && LogCategories[i].flag != BCLog::ALL) {
            if (outcount != 0) ret += ", ";
            ret += LogCategories[i].category;
            outcount++;
        }
    }
    return ret;
}

std::vector<CLogCategoryActive> ListActiveLogCategories()
{
    std::vector<CLogCategoryActive> ret;
    for (unsigned int i = 0; i < ARRAYLEN(LogCategories); i++) {
        // Omit the special cases.
        if (LogCategories[i].flag != BCLog::NONE && LogCategories[i].flag != BCLog::ALL) {
            CLogCategoryActive catActive;
            catActive.category = LogCategories[i].category;
            catActive.active = LogAcceptCategory(LogCategories[i].flag);
            ret.push_back(catActive);
        }
    }
    return ret;
}

/**
 * fStartedNewLine is a state variable held by the calling context that will
 * suppress printing of the timestamp when multiple calls are made that don't
 * end in a newline. Initialize it to true, and hold it, in the calling context.
 */
static std::string LogTimestampStr(const std::string &str, bool *fStartedNewLine)
{
    std::string strStamped;

    if (!fLogTimestamps)
        return str;

    if (*fStartedNewLine)
        strStamped =  DateTimeStrFormat("%Y-%m-%d %H:%M:%S", GetTime()) + ' ' + str;
    else
        strStamped = str;

    if (!str.empty() && str[str.size()-1] == '\n')
        *fStartedNewLine = true;
    else
        *fStartedNewLine = false;

    return strStamped;
}

int LogPrintStr(const std::string& str)
{
    int ret = 0; // Returns total number of characters written
    static bool fStartedNewLine = true;
    if (fPrintToConsole) {
        // print to console
        ret = fwrite(str.data(), 1, str.size(), stdout);
        fflush(stdout);
    } else if (fPrintToDebugLog) {
        boost::call_once(&DebugPrintInit, debugPrintInitFlag);
        boost::mutex::scoped_lock scoped_lock(*mutexDebugLog);

        std::string strTimestamped = LogTimestampStr(str, &fStartedNewLine);

        // buffer if we haven't opened the log yet
        if (fileout == NULL) {
            assert(vMsgsBeforeOpenLog);
            ret = strTimestamped.length();
            vMsgsBeforeOpenLog->push_back(strTimestamped);

        } else {
            // reopen the log file, if requested
            if (fReopenDebugLog) {
                fReopenDebugLog = false;
                fs::path pathDebug = GetDebugLogPath();
                if (freopen(pathDebug.string().c_str(),"a",fileout) != NULL)
                    setbuf(fileout, NULL); // unbuffered
            }

            ret = FileWriteStr(strTimestamped, fileout);
        }
    }

    return ret;
}

void ShrinkDebugFile()
{
    // Scroll debug.log if it's getting too big
    fs::path pathLog = GetDebugLogPath();
    FILE* file = fopen(pathLog.string().c_str(), "r");
    if (file && fs::file_size(pathLog) > 10 * 1000000) {
        // Restart the file with some of the end
        std::vector<char> vch(200000, 0);
        fseek(file, -((long)vch.size()), SEEK_END);
        int nBytes = fread(vch.data(), 1, vch.size(), file);
        fclose(file);

        file = fopen(pathLog.string().c_str(), "w");
        if (file) {
            fwrite(vch.data(), 1, nBytes, file);
            fclose(file);
        }
    } else if (file != NULL)
        fclose(file);
}
