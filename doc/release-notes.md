VITAE Core version *4.0.2* is now available from:  <https://github.com/vitae-project/vitae/releases>

This is a new revision version release, including various bug fixes and performance improvements, as well as updated translations.

Please report bugs using the issue tracker at github: <https://github.com/vitae-project/vitae/issues>

Recommended Update
==============

VITAE Core v4.0.2 is NOT a mandatory update, and user can choose to stay with v4.0.0 if they wish. However, v4.0.2 does contain minor bug fixes and performance improvements to address feedback from the v4.0.0/v4.0.1 versions.

How to Upgrade
==============

If you are running an older version, shut it down. Wait until it has completely shut down (which might take a few minutes for older versions), then run the installer (on Windows) or just copy over /Applications/VITAE-Qt (on Mac) or vitaed/vitae-qt (on Linux).


Compatibility
==============

VITAE Core is extensively tested on multiple operating systems using the Linux kernel, macOS 10.10+, and Windows 7 and later.

Microsoft ended support for Windows XP on [April 8th, 2014](https://www.microsoft.com/en-us/WindowsForBusiness/end-of-xp-support),
No attempt is made to prevent installing or running the software on Windows XP, you
can still do so at your own risk but be aware that there are known instabilities and issues.
Please do not report issues about Windows XP to the issue tracker.

VITAE Core is extensively tested on multiple operating systems using the Linux kernel, macOS 10.10+, and Windows 7 and later.

Microsoft ended support for Windows XP on [April 8th, 2014](https://www.microsoft.com/en-us/WindowsForBusiness/end-of-xp-support), No attempt is made to prevent installing or running the software on Windows XP, you can still do so at your own risk but be aware that there are known instabilities and issues. Please do not report issues about Windows XP to the issue tracker.

Apple released it's last Mountain Lion update August 13, 2015, and officially ended support on [December 14, 2015](http://news.fnal.gov/2015/10/mac-os-x-mountain-lion-10-8-end-of-life-december-14/). VITAE Core software starting with v3.2.0 will no longer run on MacOS versions prior to Yosemite (10.10). Please do not report issues about MacOS versions prior to Yosemite to the issue tracker.

VITAE Core should also work on most other Unix-like systems but is not frequently tested on them.


Notable Changes
==============


Fixed a bug where the transaction record for spent cold stake delegations was showing as "No information".

(Developers: add your notes here as part of your pull requests whenever possible)

### Hierarchical Deterministic Wallet (HD Wallet)

Wallets under a tree derivation structure in which keypairs are generated deterministically from a single seed, which can be shared partially or entirely with different systems, each with or without the ability to spend coins, [BIP32](https://github.com/bitcoin/bips/blob/master/bip-0032.mediawiki).

Enabling major improvements over the keystore management, the PIVX wallet doesn't require regular backups as before, keys are following a deterministic creation path that can be verified at any time (before HD Wallet, every keypair was randomly created and added to the keypool, forcing the user to backup the wallet every certain amount of time or could end up loosing coins forever if the latest `wallet.dat` was not being used).
As well as new possibilities like the account extended public key that enables deterministic public keys creation without the private keys requisite inside the wallet (A good use case could be online stores generating fresh addresses).
 
This work includes a customization/extension to the [BIP44](https://github.com/bitcoin/bips/blob/master/bip-0044.mediawiki) standard. We have included an unique staking keys derivation path which introduced the deterministic generation/recovery of staking addresses.

An extended description of this large work can be found in the PR [here](https://github.com/PIVX-Project/PIVX/pull/1327).

#### HD Wallet FAQ

 - How do i upgrade to HD Wallet?
 
    GUI:
    1) A dialog will appear on every wallet startup notifying you that you are running a pre-HD wallet and letting you upgrade it from there.
    2) If you haven't upgraded your wallet, the topbar (bar with icons that appears at the top of your wallet) will have an "HD" icon. Click it and the upgrade dialog will be launched.

    RPC:
    1) If your wallet is unlocked, use the `-upgradewallet` flag at startup and will automatically upgrade your wallet.
    2) If your wallet is encrypted, use the `upgradewallet` rpc command. It will upgrade your wallet to the latest wallet version.
    
 - How do i know if i'm already running an HD Wallet?
 
    1) GUI: Go to settings, press on the Debug option, then Information.
    2) RPC: call `getwalletinfo`, the `walletversion` field must be `169900` (HD Wallet Feature).
       

### Functional Changes

Automatic zPIV backup has been disabled. Thus, the following configuration options have been removed  (either as entries in the pivx.conf file or as startup flags):
- `autozpivbackup`
- `backupzpiv`
- `zpivbackuppath`

### Stake-Split threshold
The stake split threshold is no longer required to be integer. It can be a fractional amount. A threshold value of 0 disables the stake-split functionality.
The default value for the stake-split threshold has been lowered from 2000 PIV, down  to 500 PIV.


Dependencies
------------

The minimum required version of QT has been increased from 5.0 to 5.5.1 (the [depends system](https://github.com/pivx-project/pivx/blob/master/depends/README.md) provides 5.9.7)


RPC Changes
--------------

### Modified input/output for existing commands

- "CoinStake" JSON object in `getblock` output is removed, and replaced with the strings "stakeModifier" and "hashProofOfStake"

- "isPublicSpend" boolean (optional) input parameter is removed from the following commands:
 - `createrawzerocoinspend`
 - `spendzerocoin`
 - `spendzerocoinmints`
 - `spendrawzerocoin`

 These commands are now able to create only *public* spends (private spends were already enabled only on regtest).

- "mintchange" and "minimizechange" boolean input parameters are removed from the following commands:
 - `spendzerocoin`

 Mints are disabled, therefore it is no longer possible to mint the change of a zerocoin spend. The change is minimized by default.

- `setstakesplitthreshold` now accepts decimal amounts. If the provided value is `0`, split staking gets disabled. `getstakesplitthreshold` returns a double.

- `dumpwallet` no longer allows overwriting files. This is a security measure
   as well as prevents dangerous user mistakes.

### Removed commands

The following commands have been removed from the RPC interface:
- `createrawzerocoinstake`
- `getmintsinblocks`
- `reservebalance`


### Newly introduced commands

The following new commands have been added to the RPC interface:
- `...`

Details about each new command can be found below.


*v4.0.2* Change log
==============

Detailed release notes follow. For convenience in locating the code changes and accompanying discussion, both the pull request and git merge commit are mentioned.

### Core
- #1273 `d114eda990` [Core] Update checkpoints for first v7 block (Fuzzbawls)

### GUI
- #1261 `c02cc4acdd` [Bug][GUI] Double counted delegated balance. (furszy)
- #1267 `350184044d` [Qt][Bug] Load the most recent instead of the first transactions (Fuzzbawls)
- #1263 `1d0c1bb81c` [GUI] P2CS transaction divided in two types for visual accuracy. (furszy)
- #1266 `f659cbf1ef` [GUI] Quick minor GUI startup useful changes. (furszy)
- #1269 `0771075668` [GUI] CoinControlDialog, every copy to clipboard action implemented. (furszy)
- #1265 `da7c50eca1` [GUI] Connect P2CSUnlockOwner and P2CSUnlockStaker records to the model (random-zebra)
- #1268 `912cf67847` [GUI] Display latest block number in the top bar (random-zebra)
- #1279 `c09cd0d40f` [GUI] Transaction record cold staking fixes. (furszy)

### Wallet Code
- #1264 `1a12735df5` [Wallet] Don't add P2CS automatically to GetLockedCredit (random-zebra)

### Documentation
- #1272 `d4a9475e40` [Trivial] Update copyright headers for 4.0.2 (Fuzzbawls)

### RPC/REST

### Wallet

The `-reservebalance` configuration/startup option has been removed ([PR #1373](https://github.com/PIVX-Project/PIVX/pull/1373)).

### Miscellaneous

## Credits
Thanks to everyone who directly contributed to this release:

 - Fuzzbawls
 - Mrs-X
 - SHTDJ
 - Sieres
 - Warrows
 - fanquake
 - gpdionisio
 - presstab


As well as everyone that helped translating on [Transifex](https://www.transifex.com/projects/p/vitae-project-translations/), the QA team during Testing and the Node hosts supporting our Testnet.
