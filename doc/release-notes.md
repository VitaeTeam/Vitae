VITAE Core version *4.0.1* is now available from:  <https://github.com/vitae-project/vitae/releases>

This is a new revision version release, including various bug fixes and performance improvements, as well as updated translations.

Please report bugs using the issue tracker at github: <https://github.com/vitae-project/vitae/issues>

Recommended Update
==============

VITAE Core v4.0.1 is NOT a mandatory update, and user can choose to stay with v4.0.0 if they wish. However, v4.0.1 does contain minor bug fixes and performance improvements to address feedback from the v4.0.0 version.

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

Account System Deprecation
--------------------------

The internal accounting system is planned for removal in a future version. As such, this release contains notations in RPC help texts that state it's deprecation.

Some instances of the term "account" are rather loose, and possibly technically incorrect (confusing "label" with "account"). These instances will be corrected in a subsequent pull request.

Functional Test Suite Overhaul
------------------------------

Our Regression/Functional testing suite has undergone a substantial overhaul, particularly in regards to the cached chain that can be used to speed up testing. Full details can be found in the description of [#1218](https://github.com/PIVX-Project/PIVX/pull/1218).

RPC Changes
-----------

### `delegatoradd`

This command now takes an additional optional argument (string: `label`) that allows associating a label with the added owner address in the address book.

### `listdelegators`

This command now takes an optional argument (boolean: `fBlacklist`) that allows the command to show only owner addresses that have been removed from the whitelist (done by using the `delegatorremove` command).

The default behavior of this command if the new argument isn't provided remains the same as before.

*v4.0.1* Change log
==============

Detailed release notes follow. For convenience in locating the code changes and accompanying discussion, both the pull request and git merge commit are mentioned.

### Core
- #1203 `f72660f89` [Backport][Performance] Cache + guard best block hash. (furszy)
- #1205 `4f19cd0df` [Cleanup] Remove unnecessary QtCreator files (Fuzzbawls)
- #1233 `2e4d9142f` [Trivial] Remove spammy log in in StakeV1 (random-zebra)
- #1234 `2d6d48c74` [Cleanup] Remove precomputing (Fuzzbawls)
- #1237 `51e7ea2a0` [Cleanup] Remove precomputing II (random-zebra)
- #1243 `fcb21d851` [Core][Trivial] Don't log missing MNs during CleanAndRemove (random-zebra)
- #1245 `9cf807d80` [Core] Rework staking status (random-zebra)
- #1252 `e7e1dd4d1` [Trivial] Log log2_work value with 16 decimals (random-zebra)

### GUI
- #1184 `23313ac4f` [GUI][Trivial] Minor edits to written content (random-zebra)
- #1211 `a2912a9d6` [GUI] MN creation wizard (furszy)
- #1217 `d49e4a6d6` [Startup][GUI][Performance] Optimizations for huge wallets. (furszy)
- #1221 `e62da0bda` [GUI] Masternodes start all and start missing flows implemented. (furszy)
- #1223 `f2d429114` [GUI][Bug] Show locked balance in the available total amount. (furszy)
- #1224 `a3f09c422` [Trivial][GUI] Minor changes within 4.0 wallet FAQ (NoobieDev12)
- #1228 `ec3c7d67b` [GUI] Adding capability to decrease the screen size for small screens. (furszy)
- #1250 `276e1e08b` [GUI] Every MN action checking tier two synced. (furszy)
- #1253 `f65d0d828` [Qt] Initialize isLoading to false for CS view (Fuzzbawls)

### Wallet Code
- #1222 `69f897be3` [Wallet] Remove un-necessary CheckTransaction call when loading wallet. (Fuzzbawls)
- #1229 `87c369bcc` [Wallet] Graceful shutdown in the unlock corrupted wallet. (furszy)
- #1231 `ccb2402d4` [Wallet] IsEquivalentTo commented (furszy)
- #1240 `310deb9b0` [Model][Wallet][Performance] Several changes in txRecord updateStatus. (furszy)

### Build Systems
- #1199 `d40686f43` Clean up 4.0 compiler warnings (Cave Spectre)

### Documentation
- #1207 `1968f6107` [Doc] Update build-unix.md file (Fuzzbawls)

### RPC Interface
- #1206 `80414f979` [BUG][RPC] fix signature check (against old format) in mnbudgetrawvote (random-zebra)
- #1238 `fd43ba5f2` [RPC] Add optional arguments to delegatoradd and listdelegators (random-zebra)
- #1242 `2bfde4e7b` [RPC] Add coldstaking address support in importprivkey and importaddress (random-zebra)
- #1251 `441d790d8` [RPC] Notate all account stuff as deprecated (Fuzzbawls)

### Testing Systems
- #1218 `a70e82a86` [Tests] Functional Tests Suite Overhaul (random-zebra)

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
