Soferox integration/staging tree
=================================
Forked from Bitcoin reference wallet 0.8.6

Updated to Bitcoin reference wallet 0.11.0 on August 2015

Updated to Bitcoin reference wallet 0.13.3 on January 2017

Updated to Bitcoin reference wallet 0.16.0 on June 2018

Updated to own reference of 3.0.0 on December 2018

Updated to own reference of 3.0.1 on January 2019
Soferox Core Wallet

https://www.soferox.com

The algorithm was written as a candidate for sha3

https://bitcointalk.org/index.php?topic=525926.0

Copyright (c) 2009-2018 The Bitcoin Core Developers

Copyright (c) 2014-2018 The Soferox Core Developers

What is Soferox?
-----------------

Soferox is an experimental new digital currency that enables instant payments to
anyone, anywhere in the world. Soferox uses peer-to-peer technology to operate
with no central authority: managing transactions and issuing money are carried
out collectively by the network. Soferox Core is the name of open source
software which enables the use of this currency.

For more information, as well as an immediately useable, binary version of
the Soferox Core software, see http://www.soferox.com/download.

License
-------

Soferox Core is released under the terms of the MIT license. See [COPYING](COPYING) for more
information or see http://opensource.org/licenses/MIT.

Development Process
-------------------

Developers work in their own trees, then submit pull requests when they think
their feature or bug fix is ready.

If it is a simple/trivial/non-controversial change, then one of the Soferox
development team members simply pulls it.

If it is a *more complicated or potentially controversial* change, then the patch
submitter will be asked to start a discussion

The patch will be accepted if there is broad consensus that it is a good thing.
Developers should expect to rework and resubmit patches if the code doesn't
match the project's coding conventions or are controversial.

The `master` branch is regularly built and tested, but is not guaranteed to be
completely stable. [Tags](https://github.com/soferox/soferox/tags) are created
regularly to indicate new official, stable release versions of Soferox.

Development tips and tricks
---------------------------

**compiling for debugging**

Run configure with the --enable-debug option, then make. Or run configure with
CXXFLAGS="-g -ggdb -O0" or whatever debug flags you need.

**debug.log**

If the code is behaving strangely, take a look in the debug.log file in the data directory;
error and debugging message are written there.

The -debug=... command-line option controls debugging; running with just -debug will turn
on all categories (and give you a very large debug.log file).

The Qt code routes qDebug() output to debug.log under category "qt": run with -debug=qt
to see it.

**testnet and regtest modes**

Run with the -testnet option to run with "play soferoxs" on the test network, if you
are testing multi-machine code that needs to operate across the internet.

If you are testing something that can run on one machine, run with the -regtest option.
In regression test mode blocks can be created on-demand; see qa/rpc-tests/ for tests
that run in -regest mode.

**DEBUG_LOCKORDER**

Soferox Core is a multithreaded application, and deadlocks or other multithreading bugs
can be very difficult to track down. Compiling with -DDEBUG_LOCKORDER (configure
CXXFLAGS="-DDEBUG_LOCKORDER -g") inserts run-time checks to keep track of what locks
are held, and adds warning to the debug.log file if inconsistencies are detected.
