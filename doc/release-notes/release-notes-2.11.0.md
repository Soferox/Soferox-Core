Soferox Core version 2.11.0 is now available from:

  <https://soferox.org/downloads/>

This is a new major version release, bringing both new features and
bug fixes.

Please report bugs using the issue tracker at github:

  <https://github.com/soferox/soferox/issues>

Upgrading and downgrading
=========================

How to Upgrade
--------------

If you are running an older version, shut it down. Wait until it has completely
shut down (which might take a few minutes for older versions), then run the
installer (on Windows) or just copy over /Applications/Soferox-Qt (on Mac) or
soferoxd/soferox-qt (on Linux).

Downgrade warning
------------------

Because release 2.11.0 and later makes use of headers-first synchronization and
parallel block download (see further), the block files and databases are not
backwards-compatible with older versions of Soferox Core or other software:

* Blocks will be stored on disk out of order (in the order they are
received, really), which makes it incompatible with some tools or
other programs. Reindexing using earlier versions will also not work
anymore as a result of this.

* The block index database will now hold headers for which no block is
stored on disk, which earlier versions won't support.

If you want to be able to downgrade smoothly, make a backup of your entire data
directory. Without this your node will need start syncing (or importing from
bootstrap.dat) anew afterwards. It is possible that the data from a completely
synchronised 2.11.0 node may be usable in older versions as-is, but this is not
supported and may break as soon as the older version attempts to reindex.

Notable changes
===============

Block file pruning
----------------------

This release supports running a fully validating node without maintaining a copy 
of the raw block and undo data on disk. To recap, there are four types of data 
related to the blockchain in the soferox system: the raw blocks as received over 
the network (blk???.dat), the undo data (rev???.dat), the block index and the 
UTXO set (both LevelDB databases). The databases are built from the raw data.

Block pruning allows Soferox Core to delete the raw block and undo data once 
it's been validated and used to build the databases. At that point, the raw data 
is used only to relay blocks to other nodes, to handle reorganizations, to look 
up old transactions (if -txindex is enabled or via the RPC/REST interfaces), or 
for rescanning the wallet. The block index continues to hold the metadata about 
all blocks in the blockchain.

The user specifies how much space to allot for block & undo files. The minimum 
allowed is 55MB. Note that this is in addition to whatever is required for the 
block index and UTXO databases. The minimum was chosen so that Soferox Core will 
be able to maintain at least 2880 blocks on disk (two days worth of blocks at 1 
minute per block). In rare instances it is possible that the amount of space 
used will exceed the pruning target in order to keep the required last 288 
blocks on disk.

Block pruning works during initial sync in the same way as during steady state, 
by deleting block files "as you go" whenever disk space is allocated. Thus, if 
the user specifies 55MB, once that level is reached the program will begin 
deleting the oldest block and undo files, while continuing to download the 
blockchain.

For now, block pruning disables block relay.  In the future, nodes with block 
pruning will at a minimum relay "new" blocks, meaning blocks that extend their 
active chain. 

Block pruning is currently incompatible with running a wallet due to the fact 
that block data is used for rescanning the wallet and importing keys or 
addresses (which require a rescan.) However, running the wallet with block 
pruning will be supported in the near future, subject to those limitations.

Block pruning is also incompatible with -txindex and will automatically disable 
it.

Once you have pruned blocks, going back to unpruned state requires 
re-downloading the entire blockchain. To do this, re-start the node with 
-reindex. Note also that any problem that would cause a user to reindex (e.g., 
disk corruption) will cause a pruned node to redownload the entire blockchain. 
Finally, note that when a pruned node reindexes, it will delete any blk???.dat 
and rev???.dat files in the data directory prior to restarting the download.

To enable block pruning on the command line:

- `-prune=N`: where N is the number of MB to allot for raw block & undo data.

Modified RPC calls:

- `getblockchaininfo` now includes whether we are in pruned mode or not.
- `getblock` will check if the block's data has been pruned and if so, return an 
error.
- `getrawtransaction` will no longer be able to locate a transaction that has a 
UTXO but where its block file has been pruned. 

Pruning is disabled by default.

Big endian support
--------------------

Experimental support for big-endian CPU architectures was added in this
release. All little-endian specific code was replaced with endian-neutral
constructs. This has been tested on at least MIPS and PPC hosts. The build
system will automatically detect the endianness of the target.

Memory usage optimization
--------------------------

There have been many changes in this release to reduce the default memory usage
of a node, among which:

- Accurate UTXO cache size accounting (#6102); this makes the option `-dbcache`
  precise where this grossly underestimated memory usage before
- Reduce size of per-peer data structure (#6064 and others); this increases the
  number of connections that can be supported with the same amount of memory
- Reduce the number of threads (#5964, #5679); lowers the amount of (esp.
  virtual) memory needed

Fee estimation changes
----------------------

This release improves the algorithm used for fee estimation.  Previously, -1
was returned when there was insufficient data to give an estimate.  Now, -1
will also be returned when there is no fee or priority high enough for the
desired confirmation target. In those cases, it can help to ask for an estimate
for a higher target number of blocks. It is not uncommon for there to be no
fee or priority high enough to be reliably (85%) included in the next block and
for this reason, the default for `-txconfirmtarget=n` has changed from 1 to 2.

Rebranding to Soferox Core
---------------------------

To reduce confusion between Soferox-the-network and Soferox-the-software we
have renamed the reference client to Soferox Core.


OP_RETURN and data in the block chain
-------------------------------------
On OP_RETURN:  This change is not an endorsement of storing data in the
blockchain.  The OP_RETURN change creates a provably-prunable output,
to avoid data storage schemes -- some of which were already deployed --
that were storing arbitrary data such as images as forever-unspendable
TX outputs, bloating Soferox's UTXO database.

Storing arbitrary data in the blockchain is still a bad idea; it is less
costly and far more efficient to store non-currency data elsewhere.

Autotools build system
-----------------------

For 2.11.0 we switched to an autotools-based build system instead of individual
(q)makefiles.

Using the standard "./autogen.sh; ./configure; make" to build Soferox-Qt and
soferoxd makes it easier for experienced open source developers to contribute 
to the project.

Be sure to check doc/build-*.md for your platform before building from source.

Soferox-cli
-------------

Another change in the 2.11.0 release is moving away from the soferoxd executable
functioning both as a server and as a RPC client. The RPC client functionality
("tell the running soferox daemon to do THIS") was split into a separate
executable, 'soferox-cli'. The RPC client code will eventually be removed from
soferoxd, but will be kept for backwards compatibility for a release or two.

`walletpassphrase` RPC
-----------------------

The behavior of the `walletpassphrase` RPC when the wallet is already unlocked
has changed.

The 2.1.0.6 behavior of `walletpassphrase` is to fail when the wallet is already unlocked:

    > walletpassphrase 1000
    walletunlocktime = now + 1000
    > walletpassphrase 10
    Error: Wallet is already unlocked (old unlock time stays)

The new behavior of `walletpassphrase` is to set a new unlock time overriding
the old one:

    > walletpassphrase 1000
    walletunlocktime = now + 1000
    > walletpassphrase 10
    walletunlocktime = now + 10 (overriding the old unlock time)

Transaction malleability-related fixes
--------------------------------------

This release contains a few fixes for transaction ID (TXID) malleability 
issues:

- -nospendzeroconfchange command-line option, to avoid spending
  zero-confirmation change
- IsStandard() transaction rules tightened to prevent relaying and mining of
  mutated transactions
- Additional information in listtransactions/gettransaction output to
  report wallet transactions that conflict with each other because
  they spend the same outputs.
- Bug fixes to the getbalance/listaccounts RPC commands, which would report
  incorrect balances for double-spent (or mutated) transactions.
- New option: -zapwallettxes to rebuild the wallet's transaction information

Transaction Fees
----------------

This release drops the default fee required to relay transactions across the
network and for miners to consider the transaction in their blocks to
0.01mSFX per kilobyte.

Note that getting a transaction relayed across the network does NOT guarantee
that the transaction will be accepted by a miner; by default, miners fill
their blocks with 50 kilobytes of high-priority transactions, and then with
700 kilobytes of the highest-fee-per-kilobyte transactions.

The minimum relay/mining fee-per-kilobyte may be changed with the
minrelaytxfee option. Note that previous releases incorrectly used
the mintxfee setting to determine which low-priority transactions should
be considered for inclusion in blocks.

The wallet code still uses a default fee for low-priority transactions of
0.1mSFX per kilobyte. During periods of heavy transaction volume, even this
fee may not be enough to get transactions confirmed quickly; the mintxfee
option may be used to override the default.

Faster synchronization
----------------------

Soferox Core now uses 'headers-first synchronization'. This means that we first
ask peers for block headers and validate those. In a second stage, when the headers 
have been discovered, we download the blocks. However, as we already know about the 
whole chain in advance, the blocks can be downloaded in parallel from all available peers.

In practice, this means a much faster and more robust synchronization. On
recent hardware with a decent network link, it can be as little as 30 minutes
for an initial full synchronization. You may notice a slower progress in the
very first few minutes, when headers are still being fetched and verified, but
it should gain speed afterwards.

A few RPCs were added/updated as a result of this:
- `getblockchaininfo` now returns the number of validated headers in addition to
the number of validated blocks.
- `getpeerinfo` lists both the number of blocks and headers we know we have in
common with each peer. While synchronizing, the heights of the blocks that we
have requested from peers (but haven't received yet) are also listed as
'inflight'.
- A new RPC `getchaintips` lists all known branches of the block chain,
including those we only have headers for.

Transaction fee changes
-----------------------

This release automatically estimates how high a transaction fee (or how
high a priority) transactions require to be confirmed quickly. The default
settings will create transactions that confirm quickly; see the new
'txconfirmtarget' setting to control the tradeoff between fees and
confirmation times. Fees are added by default unless the 'sendfreetransactions' 
setting is enabled.

Prior releases used hard-coded fees (and priorities), and would
sometimes create transactions that took a very long time to confirm.

Statistics used to estimate fees and priorities are saved in the
data directory in the `fee_estimates.dat` file just before
program shutdown, and are read in at startup.

New command line options for transaction fee changes:
- `-txconfirmtarget=n` : create transactions that have enough fees (or priority)
so they are likely to begin confirmation within n blocks (default: 1). This setting
is over-ridden by the -paytxfee option.
- `-sendfreetransactions` : Send transactions as zero-fee transactions if possible 
(default: 0)

New RPC commands for fee estimation:
- `estimatefee nblocks` : Returns approximate fee-per-1,000-bytes needed for
a transaction to begin confirmation within nblocks. Returns -1 if not enough
transactions have been observed to compute a good estimate.
- `estimatepriority nblocks` : Returns approximate priority needed for
a zero-fee transaction to begin confirmation within nblocks. Returns -1 if not
enough free transactions have been observed to compute a good
estimate.

RPC access control changes
--------------------------

Subnet matching for the purpose of access control is now done
by matching the binary network address, instead of with string wildcard matching.
For the user this means that `-rpcallowip` takes a subnet specification, which can be

- a single IP address (e.g. `1.2.3.4` or `fe80::0012:3456:789a:bcde`)
- a network/CIDR (e.g. `1.2.3.0/24` or `fe80::0000/64`)
- a network/netmask (e.g. `1.2.3.4/255.255.255.0` or `fe80::0012:3456:789a:bcde/ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff`)

An arbitrary number of `-rpcallow` arguments can be given. An incoming connection will be accepted if its origin address
matches one of them.

For example:

| 2.1.0.6 and before                           | 2.11.0                                |
|--------------------------------------------|---------------------------------------|
| `-rpcallowip=192.168.1.1`                  | `-rpcallowip=192.168.1.1` (unchanged) |
| `-rpcallowip=192.168.1.*`                  | `-rpcallowip=192.168.1.0/24`          |
| `-rpcallowip=192.168.*`                    | `-rpcallowip=192.168.0.0/16`          |
| `-rpcallowip=*` (dangerous!)               | `-rpcallowip=::/0` (still dangerous!) |

Using wildcards will result in the rule being rejected with the following error in debug.log:

    Error: Invalid -rpcallowip subnet specification: *. Valid are a single IP (e.g. 1.2.3.4), a network/netmask (e.g. 1.2.3.4/255.255.255.0) or a network/CIDR (e.g. 1.2.3.4/24).


REST interface
--------------

A new HTTP API is exposed when running with the `-rest` flag, which allows
unauthenticated access to public node data.

It is served on the same port as RPC, but does not need a password, and uses
plain HTTP instead of JSON-RPC.

Assuming a local RPC server running on port 5442, it is possible to request:
- Blocks: http://localhost:5442/rest/block/*HASH*.*EXT*
- Blocks without transactions: http://localhost:5442/rest/block/notxdetails/*HASH*.*EXT*
- Transactions (requires `-txindex`): http://localhost:5442/rest/tx/*HASH*.*EXT*

In every case, *EXT* can be `bin` (for raw binary data), `hex` (for hex-encoded
binary) or `json`.

For more details, see the `doc/REST-interface.md` document in the repository.

RPC Server "Warm-Up" Mode
-------------------------

The RPC server is started earlier now, before most of the expensive
intialisations like loading the block index.  It is available now almost
immediately after starting the process.  However, until all initialisations
are done, it always returns an immediate error with code -28 to all calls.

This new behaviour can be useful for clients to know that a server is already
started and will be available soon (for instance, so that they do not
have to start it themselves).

Improved signing security
-------------------------

For 2.11.0 the security of signing against unusual attacks has been
improved by making the signatures constant time and deterministic.

This change is a result of switching signing to use libsecp256k1
instead of OpenSSL. Libsecp256k1 is a cryptographic library
optimized for the curve Soferox uses which was created by Bitcoin
Core developer Pieter Wuille.

There exist attacks[1] against most ECC implementations where an
attacker on shared virtual machine hardware could extract a private
key if they could cause a target to sign using the same key hundreds
of times. While using shared hosts and reusing keys are inadvisable
for other reasons, it's a better practice to avoid the exposure.

OpenSSL has code in their source repository for derandomization
and reduction in timing leaks that we've eagerly wanted to use for a
long time, but this functionality has still not made its
way into a released version of OpenSSL. Libsecp256k1 achieves
significantly stronger protection: As far as we're aware this is
the only deployed implementation of constant time signing for
the curve Soferox uses and we have reason to believe that
libsecp256k1 is better tested and more thoroughly reviewed
than the implementation in OpenSSL.

[1] https://eprint.iacr.org/2014/161.pdf

Watch-only wallet support
-------------------------

The wallet can now track transactions to and from wallets for which you know
all addresses (or scripts), even without the private keys.

This can be used to track payments without needing the private keys online on a
possibly vulnerable system. In addition, it can help for (manual) construction
of multisig transactions where you are only one of the signers.

One new RPC, `importaddress`, is added which functions similarly to
`importprivkey`, but instead takes an address or script (in hexadecimal) as
argument.  After using it, outputs credited to this address or script are
considered to be received, and transactions consuming these outputs will be
considered to be sent.

The following RPCs have optional support for watch-only:
`getbalance`, `listreceivedbyaddress`, `listreceivedbyaccount`,
`listtransactions`, `listaccounts`, `listsinceblock`, `gettransaction`. See the
RPC documentation for those methods for more information.

Compared to using `getrawtransaction`, this mechanism does not require
`-txindex`, scales better, integrates better with the wallet, and is compatible
with future block chain pruning functionality. It does mean that all relevant
addresses need to added to the wallet before the payment, though.

Consensus library
-----------------

Starting from 2.11.0, the Soferox Core distribution includes a consensus library.

The purpose of this library is to make the verification functionality that is
critical to Soferox's consensus available to other applications, e.g. 
alternative node implementations.

This library is called `libsoferoxconsensus.so` (or, `.dll` for Windows).
Its interface is defined in the C header [soferoxconsensus.h](https://github.com/Soferox/soferox/blob/master/src/script/soferoxconsensus.h).

In its initial version the API includes two functions:

- `soferoxconsensus_verify_script` verifies a script. It returns whether the indicated input of the provided serialized transaction 
correctly spends the passed scriptPubKey under additional constraints indicated by flags
- `soferoxconsensus_version` returns the API version, currently at an experimental `0`

The functionality is planned to be extended to e.g. UTXO management in upcoming releases, but the interface
for existing methods should remain stable.

Standard script rules relaxed for P2SH addresses
------------------------------------------------

The IsStandard() rules have been almost completely removed for P2SH
redemption scripts, allowing applications to make use of any valid
script type, such as "n-of-m OR y", hash-locked oracle addresses, etc.
While the Soferox protocol has always supported these types of script,
actually using them on mainnet has been previously inconvenient as
standard Soferox Core nodes wouldn't relay them to miners, nor would
most miners include them in blocks they mined.

soferox-tx
----------

It has been observed that many of the RPC functions offered by soferoxd are
"pure functions", and operate independently of the soferoxd wallet. This
included many of the RPC "raw transaction" API functions, such as
createrawtransaction.

soferox-tx is a newly introduced command line utility designed to enable easy
manipulation of soferox transactions. A summary of its operation may be
obtained via "soferox-tx --help" Transactions may be created or signed in a
manner similar to the RPC raw tx API. Transactions may be updated, deleting
inputs or outputs, or appending new inputs and outputs. Custom scripts may be
easily composed using a simple text notation, borrowed from the soferox test
suite.

This tool may be used for experimenting with new transaction types, signing
multi-party transactions, and many other uses. Long term, the goal is to
deprecate and remove "pure function" RPC API calls, as those do not require a
server round-trip to execute.

Other utilities "soferox-key" and "soferox-script" have been proposed, making
key and script operations easily accessible via command line.

Mining and relay policy enhancements
------------------------------------

Soferox Core's block templates are now for version 3 blocks only, and any mining
software relying on its `getblocktemplate` must be updated in parallel to use
libblkmaker either version 0.4.2 or any version from 0.5.1 onward.
If you are solo mining, this will affect you the moment you upgrade Soferox
Core, which must be done prior to BIP66 achieving its 951/1001 status.
If you are mining with the stratum mining protocol: this does not affect you.
If you are mining with the getblocktemplate protocol to a pool: this will affect
you at the pool operator's discretion, which must be no later than BIP66
achieving its 951/1001 status.

The `prioritisetransaction` RPC method has been added to enable miners to
manipulate the priority of transactions on an individual basis.

Soferox Core now supports BIP 22 long polling, so mining software can be
notified immediately of new templates rather than having to poll periodically.

Support for BIP 23 block proposals is now available in soferox Core's
`getblocktemplate` method. This enables miners to check the basic validity of
their next block before expending work on it, reducing risks of accidental
hardforks or mining invalid blocks.

Two new options to control mining policy:
- `-datacarrier=0/1` : Relay and mine "data carrier" (OP_RETURN) transactions
if this is 1.
- `-datacarriersize=n` : Maximum size, in bytes, we consider acceptable for
"data carrier" outputs.

The relay policy has changed to more properly implement the desired behavior of not 
relaying free (or very low fee) transactions unless they have a priority above the 
AllowFreeThreshold(), in which case they are relayed subject to the rate limiter.

BIP 66: strict DER encoding for signatures
------------------------------------------

Soferox Core 2.11.0 implements BIP 66, which introduces block version 3, and a new
consensus rule, which prohibits non-DER signatures. Such transactions have been
non-standard since Soferox v2.1.0.6 (released in June 2014), but were
technically still permitted inside blocks.

This change breaks the dependency on OpenSSL's signature parsing, and is
required if implementations would want to remove all of OpenSSL from the
consensus code.

The same miner-voting mechanism as in BIP 34 is used: when 751 out of a
sequence of 1001 blocks have version number 3 or higher, the new consensus
rule becomes active for those blocks. When 951 out of a sequence of 1001
blocks have version number 3 or higher, it becomes mandatory for all blocks.

Backward compatibility with current mining software is NOT provided, thus miners
should read the first paragraph of "Mining and relay policy enhancements" above.

Privacy: Disable wallet transaction broadcast
----------------------------------------------

This release adds an option `-walletbroadcast=0` to prevent automatic
transaction broadcast and rebroadcast (#5951). This option allows separating
transaction submission from the node functionality.

Making use of this, third-party scripts can be written to take care of
transaction (re)broadcast:

- Send the transaction as normal, either through RPC or the GUI
- Retrieve the transaction data through RPC using `gettransaction` (NOT
  `getrawtransaction`). The `hex` field of the result will contain the raw
  hexadecimal representation of the transaction
- The transaction can then be broadcasted through arbitrary mechanisms
  supported by the script

One such application is selective Tor usage, where the node runs on the normal
internet but transactions are broadcasted over Tor.

For an example script see [bitcoin-submittx](https://github.com/laanwj/bitcoin-submittx).

Privacy: Stream isolation for Tor
----------------------------------

This release adds functionality to create a new circuit for every peer
connection, when the software is used with Tor. The new option,
`-proxyrandomize`, is on by default.

When enabled, every outgoing connection will (potentially) go through a
different exit node. That significantly reduces the chance to get unlucky and
pick a single exit node that is either malicious, or widely banned from the P2P
network. This improves connection reliability as well as privacy, especially
for the initial connections.

**Important note:** If a non-Tor SOCKS5 proxy is configured that supports
authentication, but doesn't require it, this change may cause that proxy to reject
connections. A user and password is sent where they weren't before. This setup
is exceedingly rare, but in this case `-proxyrandomize=0` can be passed to
disable the behavior.

As well as everyone that helped translating on [Transifex](https://www.transifex.com/projects/p/bitcoin/).

