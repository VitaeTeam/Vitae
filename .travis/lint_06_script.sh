#!/usr/bin/env bash
#
# Copyright (c) 2018 The Bitcoin Core developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.

export LC_ALL=C


# Remove this comment and the `#` from the following two lines when we merge proper subtree implementations for secp256k1 and leveldbcontrib/devtools/git-subtree-check.sh src/secp256k1
#contrib/devtools/git-subtree-check.sh src/univalue
#contrib/devtools/git-subtree-check.sh src/leveldb
contrib/devtools/check-doc.py
contrib/devtools/logprint-scanner.py

if [ "$TRAVIS_EVENT_TYPE" = "pull_request" ]; then
  contrib/devtools/lint-whitespace.sh
fi
