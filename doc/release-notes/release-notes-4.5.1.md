VITAE Core version 4.5.1 is now available from:

  <https://github.com/vitaeteam/vitae/releases>

This is a new minor-revision version release, including various bug fixes and
performance improvements.

Please report bugs using the issue tracker at github:

  <https://github.com/vitaeteam/vitae/issues>

Recommended Update
==============

VITAE Core v4.5.1 is a mandatory release only for MacOS users who are using the latest version of OSX (Big Sur). It is optional for all other users. The primary changes are fixing OSX Big Sur compatibility.

How to Upgrade
==============

If you are running an older version, shut it down. Wait until it has completely shut down (which might take a few minutes for older versions), then run the installer (on Windows and MacOS) or just copy over vitaed/vitae-qt (on Linux).

Compatibility
==============

VITAE Core is extensively tested on multiple operating systems using
the Linux kernel, MacOS 10.13+, and Windows 8.1 and later.

Microsoft ended support for Windows 7 as of [January 14th, 2020].
Apple ended suppot for MacOS 10.12 as of [October, 2019].
No attempt is made to prevent installing or running the software on
Windows 7 or earlier versions, or MacOS 10.12 or earlier versions.
You can still do so at your own risk but be aware that there may be instabilities and issues.
Please do not report issues about unsupported versions of Windows or MacOS to the issue tracker.

VITAE Core should also work on most other Unix-like systems but is not
frequently tested on them.

**Currently there are issues with the gitian releases on MacOS. MacOS releases are built natively.**


Notable Changes
===============

MacOS Big Sur Compatibility
---------------------------
The primary change in this release is to help ensure compatibility with MacOS 11 (Big Sur) and the newest Mac hardware that uses the M1 chip.

4.5.1 Change log
=================

Detailed release notes follow. This overview includes changes that affect
behavior, not code moves, refactors and string updates. The original GitHub commit is
referenced to help locate each code change.

### GUI
- `30795b3` [Qt] Fixes for OSX Big Sur compatibility

### Miscellaneous
- `921600b` [Build] Added checkpoint

Credits
=======

Thanks to everyone who directly contributed to this release:
- Michael Trisko
- furszy
