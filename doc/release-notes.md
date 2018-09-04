Soferox Core version 2.16.0 is now available from:

  <https://soferox.org/downloads/>

Please report bugs using the issue tracker at GitHub:

  <https://github.com/soferox/soferox/issues>

How to Upgrade
==============

If you are running an older version, shut it down. Wait until it has completely
shut down (which might take a few minutes for older versions), then run the
installer (on Windows) or just copy over `/Applications/Soferox-Qt` (on Mac)
or `soferoxd`/`soferox-qt` (on Linux).

The first time you run version 2.16.0 or newer, your chainstate database will be converted to a
new format, which will take anywhere from a few minutes to half an hour,
depending on the speed of your machine.

Note that the block database format also changed in version 2.1.0.6 and there is no
automatic upgrade code from before version 2.1.0.6 to version 2.16.0 or higher.
However, as usual, old wallet versions are still supported.

Downgrading warning
-------------------

Wallets created in 2.16.0 and later are not compatible with versions prior to 2.16.0
and will not work if you try to use newly created wallets in older versions. Existing
wallets that were created with older versions are not affected by this.

Compatibility
==============

Soferox Core is extensively tested on multiple operating systems using
the Linux kernel, macOS 10.8+, and Windows Vista and later. Windows XP is not supported.

Soferox Core should also work on most other Unix-like systems but is not
frequently tested on them.


Notable changes
===============

Example item
-------------

Example item for a notable change.

2.16.0 change log
------------------

(to be filled in at release time)

Credits
=======

Thanks to everyone who directly contributed to this release:

(to be filled in at release time)

As well as everyone that helped translating on [Transifex](https://www.transifex.com/projects/p/bitcoin/).
