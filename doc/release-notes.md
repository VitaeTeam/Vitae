VITAE Core version *3.1.1* is now available from:  <https://github.com/vitae-project/vitae/releases>

This is a new minor version release, including various bug fixes and performance improvements, as well as updated translations.

Please report bugs using the issue tracker at github: <https://github.com/vitae-project/vitae/issues>

Non-Mandatory Update
==============

VITAE Core v3.1.1 is a non-mandatory update to address bugs and introduce minor enhancements that do not require a network change.

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

Minimum Supported MacOS Version
------

The minimum supported version of MacOS (OSX) has been moved from 10.8 Mountain Lion to 10.10 Yosemite. Users still running a MacOS version prior to Yosemite will need to upgrade their OS if they wish to continue using the latest version(s) of the PIVX Core wallet.

Attacks, Exploits, and Mitigations
------

### Fake Stake

On Janurary 22 2019, Decentralized Systems Lab out of the University of Illinois published a study entitled “[‘Fake Stake’ attacks on chain-based Proof-of-Stake cryptocurrencies](https://medium.com/@dsl_uiuc/fake-stake-attacks-on-chain-based-proof-of-stake-cryptocurrencies-b8b05723f806)”, which outlined a type of Denial of Service attack that could take place on a number of Proof of Stake based networks by exhausting a client's RAM or Disk resources.

A full report provided by PIVX developers is available on the [PIVX Website](https://pivx.org/fake-stake-official-pivx-report/), which includes additional findings, mitigation details, and resources for testing. This type of attack has no risk to users' privacy and does not affect their holdings.

### Wrapped Serials

On March 6th 2019, an attack was detected on the PIVX network zerocoin protocol, or zPIV. The vulnerability allows an attacker to fake serials accepted by the network and thus to spend zerocoins that have never been minted. As severe as it is, it does not harm users’ privacy and does not affect their holdings directly.

As a result of this, all zPIV functionality was disabled via one of our sporks shortly after verification of this exploit. A full report, detailing how this attack was performed, as well as investigation results and mitigation methods is available [On Medium](https://medium.com/@dev.pivx/report-wrapped-serials-attack-5f4bf7b51701).

zPIV functions will be restored after v3.2.0 is pushed out and the majority of the network has upgraded.

Major New Features
------

### BIP65 (CHECKLOCKTIMEVERIFY) Soft-Fork

PIVX Core v3.2.0 introduces new consensus rules for scripting pathways to support the [BIP65](https://github.com/bitcoin/bips/blob/master/bip-0065.mediawiki) standard. This is being carried out as a soft-fork in order to provide ample time for stakers to update their wallet version.

### Automint Addresses

A new "Automint Addresses" feature has been added to the wallet that allows for the creation of new addresses who's purpose is to automatically convert any PIV funds received by such addresses to zPIV. The feature as a whole can be enabled/disabled either at runtime using the `-enableautoconvertaddress` option, via RPC/Console with the `enableautomintaddress` command, or via the GUI's options dialog, with the default being enabled.

Creation of these automint addresses is currently only available via the RPC/Console `createautomintaddress` command, which takes no additional arguments. The command returns a new PIVX address each time, but addresses created by this command can be re-used if desired.

### In-wallet Proposal Voting

A new UI wallet tab has been introduced that allows users to view the current budget proposals, their vote counts, and vote on proposals if the wallet is acting as a masternode controller. The visual design is to be considered temporary, and will be undergoing further design and display improvements in the future.

### Zerocoin Lite Node Protocol

Support for the ZLN Protocol has been added, which allows for a node to opt-in to providing extended network services for the protocol. By default, this functionality is disabled, but can be enabled by using the `-peerbloomfilterszc` runtime option.

A full technical writeup of the protocol can be found [Here](https://pivx.org/wp-content/uploads/2018/11/Zerocoin_Light_Node_Protocol.pdf).

### Precomputed Zerocoin Proofs

This introduces the ability to do most of the heavy computation required for zPIV spends **before** actually initiating the spend. A new thread, `ThreadPrecomputeSpends`, is added which constantly runs in the background.

`ThreadPrecomputeSpends`' purpose is to monitor the wallet's zPIV mints and perform partial witness accumulations up to `nHeight - 20` blocks from the chain's tip (to ensure that it only ever computes data that is at least 2 accumulator checkpoints deep), retaining the results in memory.

Additionally, a file based cache is introduced, `precomputes.dat`, which serves as a place to store any precomputed data between sessions, or when the in-memory cache size is exhausted. Swapping data between memory and disk file is done as needed, and periodic cache flushes to the disk are routine.

This also introduces 2 new runtime configuration options:

* `-precompute` is a binary boolean option (`1` or `0`) that determines wither or not pre-computation should be activated at runtime (default value is to activate, `1`).
* `-precomputecachelength` is a numeric value between `500` and `2000` that tells the precompute thread how many blocks to include during each pass (default is `1000`).

A new RPC command, `clearspendcache`, has been added that allows for the clearing/resetting of the precompute cache (both memory and disk). This command takes no additional arguments.

Finally, the "security level" option for spending zPIV has been completely removed, and all zPIV spends now spend at what was formerly "security level" `100`. This change has been reflected in any RPC command that previously took a security level argument, as well as in the GUI's Privacy section for spending zPIV.

### Regression Test Suite

The RegTest network mode has been re-worked to once again allow for the generation of on-demand PoW and PoS blocks. Additionally, many of the existing functional test scripts have been adapted for use with PIVX, and we now have a solid testing base for highly customizable tests to be written.

With this, the old `setgenerate` RPC command no longer functions in regtest mode, instead a new `generate` command has been introduced that is more suited for use in regtest mode.

GUI Changes
------

### Console Security Warning

Due to an increase in social engineering attacks/scams that rely on users relaying information from console commands, a new warning message has been added to the Console window's initial welcome message.

### Optional Hiding of Orphan Stakes

The options dialog now contains a checkbox option to hide the display of orphan stakes from both the overview and transaction history sections. Further, a right-click context menu option has been introduced in the transaction history tab to achieve the same effect.

**Note:** This option only affects the visual display of orphan stakes, and will not prevent them nor remove them from the underlying wallet database.

### Transaction Type Recoloring

The color of various transaction types has been reworked to provide better visual feedback. Staking and masternode rewards are now purple, orphan stakes are now light gray, other rejected transactions are in red, and normal receive/send transactions are black.

### Receive Tab Changes

The address to be used when creating a new payment request is now automatically displayed in the form. This field is not user-editable, and will be updated as needed by the wallet.

A new button has been added below the payment request form, "Receiving Addresses", which allows for quicker access to all the known receiving addresses. This one-click button is the same as using the `File->Receiving Addresses...` menu command, and will open up the Receiving Addresses UI dialog.

Historical payment requests now also display the address used for the request in the history table. While this information was already available when clicking the "Show" button, it was an extra step that shouldn't have been necessary.

### Privacy Tab Changes

The entire right half of the privacy tab can now be toggled (shown/hidden) via a new UI button. This was done to reduce "clutter" for users that may not wish to see the detailed information regarding individual denomination counts.

RPC Changes
------

### Backupwallet Sanity

The `backupwallet` RPC command no longer allows for overwriting the currently in use wallet.dat file. This was done to avoid potential file corruption caused by multiple conflicting file access operations.

### Spendzerocoin Security Level Removed

The `securitylevel` argument has been removed from the `spendzerocoin` RPC command.

### Spendzerocoinmints Added

Introduce the `spendzerocoinmints` RPC call to enable spending specific zerocoins, provided as an array of hex strings (serial hashes).

### Getreceivedbyaddress Update

When calling `getreceivedbyaddress` with a non-wallet address, return a proper error code/message instead of just `0`

### Validateaddress More Verbosity

`validateaddress` now has the ability to return more (non-critical or identifying) details about P2SH (multisig) addresses by removing the needless check against ISMINE_NO.

### Listmintedzerocoins Additional Options

Add a `fVerbose` boolean optional argument (default=false) to `listmintedzerocoins` call to have a more detailed output.

If `fVerbose` is specified as first argument, then a second optional boolean argument `fMatureOnly` (default=false) can be used to filter-out immature mints.


VITAE Daemon & Client (RPC Changes)
--------------

### Fix listtransactions RPC function

This addresses an issue where new incoming transactions are not recorded properly, and subsequently, not returned with `listtransactions` in the same session.

This fix was previously included in the `v3.1.0.3` tag, and relayed to affected exchanges/services, which typically use this command for accounting purposes. It is included here for completeness.

Technical Changes
--------------

### Switch to libsecp256k1 signature verification

Here is the long overdue update for VITAE to let go of OpenSSL in its consensus code. The rationale behind it is to avoid depending on an external and changing library where our consensus code is affected. This is security and consensus critical. VITAE users will experience quicker block validations and sync times as block transactions are verified under libsecp256k1.

The recent [CVE-2018-0495](https://www.nccgroup.trust/us/our-research/technical-advisory-return-of-the-hidden-number-problem/) brings into question a potential vulnerability with OpenSSL (and other crypto libraries) that libsecp256k1 is not susceptible to.

### Write to the zerocoinDB in batches

Instead of using a separate write operation for each and every bit of data that needs to be flushed to disk, utilize leveldb's batch writing capability. The primary area of improvement this offers is when reindexing the zerocoinDB (`-reindexzerocoin`), which went from needing multiple hours on some systems to mere minutes.

Secondary improvement area is in ConnectBlock() when multiple zerocoin transactions are involved.

### Resolution of excessive peer banning

It was found that following a forced closure of the VITAE core wallet (ungraceful), a situation could arise that left partial/incomplete data in the disk cache. This caused the client to fail a basic sanity test and ban any peer which was sending the (complete) data. This, in turn, was causing the wallet to become stuck. This issue has been resolved client side by guarding against this partial/incomplete data in the disk cache.

*3.1.1* Change log
--------------

Detailed release notes follow. This overview includes changes that affect behavior, code moves, refactoring and string updates. For convenience in locating the code changes and accompanying discussion, both the pull request and git merge commit are mentioned.

### Core Features
 - #549 `8bf13a5ad` [Crypto] Switch to libsecp256k1 signature verification and update the lib (warrows)
 - #609 `6b73598b9` [MoveOnly] Remove zVIT code from main.cpp (presstab)
 - #610 `6c3bc8c76` [Main] Check whether tx is in chain in ContextualCheckZerocoinMint(). (presstab)
 - #624 `1a82aec96` [Core] Missing seesaw value for block 325000 (warrows)
 - #636 `d359c6136` [Main] Write to the zerocoinDB in batches (Fuzzbawls)

### Build System
 - #605 `b4d82c944` [Build] Remove unnecessary BOOST dependency (Mrs-X)
 - #622 `b8c672c98` [Build] Make sure Boost headers are included for libzerocoin (Fuzzbawls)
 - #639 `98c7a4f65` [Travis] Add separate job to check doc/logprint/subtree (Fuzzbawls)
 - #648 `9950fce59` [Depends] Update Qt download url (fanquake)
 
### P2P Protocol and Network Code
 - #608 `a602d00eb` [Budget] Make sorting of finalized budgets deterministic (Mrs-X)
 - #647 `3aa3e5c97` [Net] Update hard-coded fallback seeds (Fuzzbawls)

### GUI
 - #580 `c296b7572` Fixed Multisend dialog to show settings properly (SHTDJ)
 - #598 `f0d894253` [GUI] Fix wrongly displayed balance on Overview tab (Mrs-X)
 - #600 `217433561` [GUI] Only enable/disable PrivacyDialog zVIT elements if needed. (presstab)
 - #612 `6dd752cb5` [Qt] Show progress percent for zvit reindex operations (Fuzzbawls)
 - #626 `9b6a42ba0` [Qt] Add Tor service icon to status bar (Fuzzbawls)
 - #629 `14e125795` [Qt] Remove useless help button from QT dialogs (windows) (warrows)
 - #646 `c66b7b632` [Qt] Periodic translation update (Fuzzbawls)
 
### Wallet
 - #597 `766d5196c` [Wallet] Write new transactions to wtxOrdered properly (Fuzzbawls)
 - #603 `779d8d597` Fix spending for v1 zVIT created before block 1050020. (presstab)
 - #617 `6b525f0df` [Wallet] Adjust staking properties to lower orphan rates. (presstab)
 - #625 `5f2e61d60` [Wallet] Add some LOCK to avoid crash (warrows)
 
### Miscellaneous
 - #585 `76c01a560` [Doc] Change aarch assert sign output folder (Warrows)
 - #595 `d2ce04cc0` [Tests] Fix chain ordering in budget tests (Fuzzbawls)
 - #611 `c6a57f664` [Output] Properly log reason(s) for increasing a peer's DoS score. (Fuzzbawls)
 - #649 `f6bfb4ade` [Utils] Add copyright header to logprint-scanner.py (Fuzzbawls)
 
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
