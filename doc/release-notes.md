VITAE Core version *3.4.0* is now available from:  <https://github.com/vitae-project/vitae/releases>

This is a new major version release, including various bug fixes and performance improvements.

Please report bugs using the issue tracker at github: <https://github.com/vitae-project/vitae/issues>

Mandatory Update
==============

VITAE Core v3.4.0 is a mandatory update for all users. This release contains new consensus rules and improvements that are not backwards compatible with older versions. Users will need to update their clients before enforcement of this update goes into effect.

Masternodes will need to be restarted once both the masternode daemon and the controller wallet have been upgraded.

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


Apple released it's last Mountain Lion update August 13, 2015, and officially ended support on [December 14, 2015](http://news.fnal.gov/2015/10/mac-os-x-mountain-lion-10-8-end-of-life-december-14/). VITAE Core software starting with v3.2.0 will no longer run on MacOS versions prior to Yosemite (10.10). Please do not report issues about MacOS versions prior to Yosemite to the issue tracker.

VITAE Core should also work on most other Unix-like systems but is not frequently tested on them.


Notable Changes
==============

(Developers: add your notes here as part of your pull requests whenever possible)

*3.4.0* Change log
==============

Detailed release notes follow. This overview includes changes that affect behavior, not code moves, refactors and string updates. For convenience in locating the code changes and accompanying discussion, both the pull request and git merge commit are mentioned.

### Core Features
 - #983 `ac8cb7376d` [PoS] Stake Modifier V2 (random-zebra)
 - #958 `454c487424` [Staking] Modify miner and staking thread for efficiency (Cave Spectre)
 - #915 `9c5a300624` Modify GetNextWorkRequired to set Target Limit correctly (Cave Spectre)
 - #952 `7ab673f6fa` [Staking] Prevent potential negative out values during stake splitting (Cave Spectre)
 - #941 `0ac0116ae4` [Refactor] Move ThreadStakeMinter out of net.cpp (Fuzzbawls)
 - #932 `924ec4f6dd` [Node] Do all block index writes in a batch (Pieter Wuille)

### Build System
 - #934 `92aa6c2daa` [Build] Bump master to 3.3.99 (pre-3.4) (Fuzzbawls)
 - #943 `918852cb90` [Travis] Show functional tests progress (warrows)
 - #957 `2c9f624455` [Build] Add info about '--with-unsupported-ssl' (Warrows)

### P2P Protocol and Network Code
 - #987 `fa1dbab247` [Net] Protocol update enforcement for 70917 and new spork keys (Fuzzbawls)

### GUI
 - #933 `e47fe3d379` [Qt] Add blockhash + datadir to information tab (Mrs-X)

### RPC/REST
 - #950 `3d7e16e753` [RPC] require valid URL scheme on budget commands (Cave Spectre)
 - #964 `a03fa6236d` [Refactor] Combine parameter checking of budget commands (Cave Spectre)
 - #965 `b9ce433bd5` [RPC] Correct issues with budget commands (Cave Spectre)

### Wallet
 - #939 `37ad934ad8` [Wallet] Remove (explicitely) unused tx comparator (warrows)
 - #971 `bbeabc4d63` [Wallet][zVIT] zc public spend parse crash in wallet startup. (furszy)
 - #980 `8b81d8f6f9` [Wallet] Remove Bitcoin Core 0.8 block hardlinking (JSKitty)
 - #982 `a0a1af9f78` [Miner] Don't create new keys when generating PoS blocks (random-zebra)

### Test Suites
 - #961 `2269f10fd9` [Trivial][Tests] Do not fail test when warnings are written to stderr (random-zebra)
 - #974 `f9d4ee0b15` [Tests] Add Spork functional test and update RegTest spork key (random-zebra)
 - #976 `12de5ec1dc` [Refactor] Fix stake age checks for regtest (random-zebra)

### Miscellaneous
 - #947 `6ce55eec2d` [Scripts] Sync github-merge.py with upstream (Fuzzbawls)
 - #948 `4a2b4831a9` [Docs] Clean and re-structure the gitian-keys directory (Fuzzbawls)
 - #949 `9e4c3576af` [Refactor] Remove all "using namespace" statements (warrows)
 - #951 `fa40040f80` [Trivial] typo fixes (Cave Spectre)
 - #986 `fdd0cdb72f` [Doc] Release notes update (Fuzzbawls)



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
