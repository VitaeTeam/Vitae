// Copyright (c) 2014-2016 The Dash developers
// Copyright (c) 2016-2019 The PIVX developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SPORKID_H
#define SPORKID_H

/*
    Don't ever reuse these IDs for other sporks
    - This would result in old clients getting confused about which spork is for what
*/

enum SporkId : int32_t {
    SPORK_2_SWIFTTX                             = 10001,
    SPORK_3_SWIFTTX_BLOCK_FILTERING             = 10002,
    SPORK_5_MAX_VALUE                           = 10004,
    SPORK_8_MASTERNODE_PAYMENT_ENFORCEMENT      = 10007,
    SPORK_9_MASTERNODE_BUDGET_ENFORCEMENT       = 10008,
    SPORK_10_MASTERNODE_PAY_UPDATED_NODES       = 10009,
    SPORK_13_ENABLE_SUPERBLOCKS                 = 10012,
    SPORK_14_NEW_PROTOCOL_ENFORCEMENT           = 10013,
    SPORK_15_NEW_PROTOCOL_ENFORCEMENT_2         = 10014,
    SPORK_16_ZEROCOIN_MAINTENANCE_MODE          = 10015,

    SPORK_START                                 = SPORK_2_SWIFTTX,
    SPORK_END                                   = SPORK_16_ZEROCOIN_MAINTENANCE_MODE,
    SPORK_INVALID                               = -1
};

#endif
