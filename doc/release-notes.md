VITAE Core version *4.1.1* is now available from:  <https://github.com/vitae-project/vitae/releases>

This is a new revision version release, including various bug fixes and performance improvements, as well as updated translations.

Please report bugs using the issue tracker at github: <https://github.com/vitae-project/vitae/issues>

Mandatory Update
==============

PIVX Core v4.1.1 is a **mandatory** update for all users. This release contains new consensus rules and improvements that are not backwards compatible with older versions. Users will need to update their clients before enforcement of this update goes into effect.

Update enforcement is currently scheduled to go into effect at the following time:

```
Mainnet: Friday, June 5, 2020 9:00:00 PM GMT
```

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


Notable Changes
==============

We have identified and fixed Layer 2 check that was causing some nodes to disagree on which MN to pay and thus resulted in these nodes forking.

## Credits
Thanks to everyone who directly contributed to this release:
- Fuzzbawls
- furszy
- random-zebra

As well as everyone that helped translating on [Transifex](https://www.transifex.com/projects/p/vitae-project-translations/), the QA team during Testing and the Node hosts supporting our Testnet.
