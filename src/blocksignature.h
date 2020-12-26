// Copyright (c) 2017-2019 The PIVX developers
// Copyright (c) 2018 The VITAE developers
// Distributed under the MIT software license, see the accompanying

#ifndef VITAE_BLOCKSIGNATURE_H
#define VITAE_BLOCKSIGNATURE_H

#include "key.h"
#include "primitives/block.h"
#include "keystore.h"

bool SignBlockWithKey(CBlock& block, const CKey& key);
bool SignBlock(CBlock& block, const CKeyStore& keystore);
bool CheckBlockSignature(const CBlock& block);

#endif //VITAE_BLOCKSIGNATURE_H
