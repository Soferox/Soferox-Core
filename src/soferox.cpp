// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin Core developers
// Copyright (c) 2014-2018 The Soferox developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "soferox.h"

#include <boost/assign/list_of.hpp>

#include "arith_uint256.h"
#include "chain.h"
#include "chainparams.h"
#include "consensus/merkle.h"
#include "consensus/params.h"
#include "utilstrencodings.h"
#include "crypto/sha256.h"

#include "bignum.h"

#include "chainparamsseeds.h"

#ifdef _MSC_VER
#	include <intrin.h>
#endif

extern "C" {

#if !defined(UCFG_LIBEXT) && (defined(_M_IX86) || defined(_M_X64)) && defined(_MSC_VER)

	static __inline void Cpuid(int a[4], int level) {
#	ifdef _MSC_VER
		__cpuid(a, level);
#	else
		__cpuid(level, a[0], a[1], a[2], a[3]);
#	endif
	}

	char g_bHasSse2;

	static int InitBignumFuns() {
		int a[4];
		::Cpuid(a, 1);
		g_bHasSse2 = a[3] & 0x02000000;
		return 1;
	}

	static int s_initBignumFuns = InitBignumFuns();
#endif // defined(_M_IX86) || defined(_M_X64)
} // "C"

using namespace std;

static const int64_t nGenesisBlockRewardCoin = 1 * COIN;
int64_t minimumSubsidy = 5.0 * COIN;
static const int64_t nPremine = 40000000 * COIN;

int64_t static GetBlockSubsidy(int nHeight){


	if (nHeight == 0)
    {
        return nGenesisBlockRewardCoin;
    }

	if (nHeight == 1)
    {
        return nPremine;
		/*
		optimized standalone cpu miner 	60*512=30720
		standalone gpu miner 			120*512=61440
		first pool			 			70*512 =35840
		block-explorer		 			60*512 =30720
		mac wallet binary    			30*512 =15360
		linux wallet binary  			30*512 =15360
		web-site						100*512	=51200
		total									=240640
		*/
    }

	int64_t nSubsidy = 512 * COIN;

    // Subsidy is reduced by 6% every 10080 blocks, which will occur approximately every 1 week
    int exponent=(nHeight / 10080);
    for(int i=0;i<exponent;i++){
        nSubsidy=nSubsidy*47;
		nSubsidy=nSubsidy/50;
    }
    if(nSubsidy<minimumSubsidy){nSubsidy=minimumSubsidy;}
    return nSubsidy;
}

int64_t static GetBlockSubsidy120000(int nHeight)
{
	// Subsidy is reduced by 10% every day (1440 blocks)
	int64_t nSubsidy = 250 * COIN;
	int exponent = ((nHeight - 120000) / 1440);
	for(int i=0; i<exponent; i++)
		nSubsidy = (nSubsidy * 45) / 50;

	return nSubsidy;
}

int64_t static GetBlockSubsidy150000(int nHeight)
{
	static int heightOfMinSubsidy = INT_MAX;
	if (nHeight < heightOfMinSubsidy) {
		// Subsidy is reduced by 1% every week (10080 blocks)
		int64_t nSubsidy = 25 * COIN;
		int exponent = ((nHeight - 150000) / 10080);
		for (int i = 0; i < exponent; i++)
			nSubsidy = (nSubsidy * 99) / 100;

		if (nSubsidy >= minimumSubsidy)
			return nSubsidy;
		heightOfMinSubsidy = (min)(heightOfMinSubsidy, nHeight);
	}
	return minimumSubsidy;
}

CAmount GetBlockSubsidy(int nHeight, const Consensus::Params& consensusParams)
{
	return nHeight >= 150000 ? GetBlockSubsidy150000(nHeight)
		: nHeight >= 120000 ? GetBlockSubsidy120000(nHeight)
		: GetBlockSubsidy(nHeight);
}

//
// minimum amount of work that could possibly be required nTime after
// minimum work required was nBase
//
static const int64_t nTargetSpacing = 1 * 60; // soferox every 60 seconds

//!!!BUG this function is non-deterministic  because FP-arithetics
unsigned int static DarkGravityWave(const CBlockIndex* pindexLast, const CBlockHeader *pblock, const Consensus::Params& params) {
    /* current difficulty formula, darkcoin - DarkGravity, written by Evan Duffield - evan@darkcoin.io */
    const CBlockIndex *BlockLastSolved = pindexLast;
    const CBlockIndex *BlockReading = pindexLast;
    int64_t nBlockTimeAverage = 0;
    int64_t nBlockTimeAveragePrev = 0;
    int64_t nBlockTimeCount = 0;
    int64_t nBlockTimeSum2 = 0;
    int64_t nBlockTimeCount2 = 0;
    int64_t LastBlockTime = 0;
    int64_t PastBlocksMin = 12;
    int64_t PastBlocksMax = 120;
    int64_t CountBlocks = 0;
    CBigNum PastDifficultyAverage;
    CBigNum PastDifficultyAveragePrev;

    if (BlockLastSolved == NULL || BlockLastSolved->nHeight == 0 || BlockLastSolved->nHeight < PastBlocksMin) {
		return UintToArith256(params.powLimit).GetCompact();
	}

    for (unsigned int i = 1; BlockReading && BlockReading->nHeight > 0; i++) {
        if (PastBlocksMax > 0 && i > PastBlocksMax) { break; }
        CountBlocks++;

        if(CountBlocks <= PastBlocksMin) {
            if (CountBlocks == 1) { PastDifficultyAverage.SetCompact(BlockReading->nBits); }
            else { PastDifficultyAverage = ((CBigNum().SetCompact(BlockReading->nBits) - PastDifficultyAveragePrev) / CountBlocks) + PastDifficultyAveragePrev; }
            PastDifficultyAveragePrev = PastDifficultyAverage;
        }

        if(LastBlockTime > 0){
            int64_t Diff = (LastBlockTime - BlockReading->GetBlockTime());
            if(Diff < 0) Diff = 0;
            if(nBlockTimeCount <= PastBlocksMin) {
                nBlockTimeCount++;

                if (nBlockTimeCount == 1) { nBlockTimeAverage = Diff; }
                else { nBlockTimeAverage = ((Diff - nBlockTimeAveragePrev) / nBlockTimeCount) + nBlockTimeAveragePrev; }
                nBlockTimeAveragePrev = nBlockTimeAverage;
            }
            nBlockTimeCount2++;
            nBlockTimeSum2 += Diff;
        }
        LastBlockTime = BlockReading->GetBlockTime();

        if (BlockReading->pprev == NULL) { assert(BlockReading); break; }
        BlockReading = BlockReading->pprev;
    }

    CBigNum bnNew(PastDifficultyAverage);
    if (nBlockTimeCount != 0 && nBlockTimeCount2 != 0) {
            double SmartAverage = (((nBlockTimeAverage)*0.7)+((nBlockTimeSum2 / nBlockTimeCount2)*0.3));
            if(SmartAverage < 1) SmartAverage = 1;
            double Shift = nTargetSpacing/SmartAverage;

            int64_t nActualTimespan = (CountBlocks*nTargetSpacing)/Shift;
            int64_t nTargetTimespan = (CountBlocks*nTargetSpacing);
            if (nActualTimespan < nTargetTimespan/3)
                nActualTimespan = nTargetTimespan/3;
            if (nActualTimespan > nTargetTimespan*3)
                nActualTimespan = nTargetTimespan*3;

            // Retarget
            bnNew *= nActualTimespan;
            bnNew /= nTargetTimespan;
    }

    if (bnNew > CBigNum(params.powLimit)){
        bnNew = CBigNum(params.powLimit);
    }

    return bnNew.GetCompact();
}

unsigned int static DarkGravityWave3(const CBlockIndex* pindexLast, const CBlockHeader *pblock, const Consensus::Params& params) {
    /* current difficulty formula, darkcoin - DarkGravity v3, written by Evan Duffield - evan@darkcoin.io */
    const CBlockIndex *BlockLastSolved = pindexLast;
    const CBlockIndex *BlockReading = pindexLast;
    int64_t nActualTimespan = 0;
    int64_t LastBlockTime = 0;
    int64_t PastBlocksMin = 24;
    int64_t PastBlocksMax = 24;
    int64_t CountBlocks = 0;
    CBigNum PastDifficultyAverage;
    CBigNum PastDifficultyAveragePrev;

    if (BlockLastSolved == NULL || BlockLastSolved->nHeight == 0 || BlockLastSolved->nHeight < PastBlocksMin) {
		return UintToArith256(params.powLimit).GetCompact();
    }

    for (unsigned int i = 1; BlockReading && BlockReading->nHeight > 0; i++) {
        if (PastBlocksMax > 0 && i > PastBlocksMax) { break; }
        CountBlocks++;

        if(CountBlocks <= PastBlocksMin) {
            if (CountBlocks == 1) { PastDifficultyAverage.SetCompact(BlockReading->nBits); }
            else { PastDifficultyAverage = ((PastDifficultyAveragePrev * CountBlocks)+(CBigNum().SetCompact(BlockReading->nBits))) / (CountBlocks+1); }
            PastDifficultyAveragePrev = PastDifficultyAverage;
        }

        if(LastBlockTime > 0){
            int64_t Diff = (LastBlockTime - BlockReading->GetBlockTime());
            nActualTimespan += Diff;
        }
        LastBlockTime = BlockReading->GetBlockTime();

        if (BlockReading->pprev == NULL) { assert(BlockReading); break; }
        BlockReading = BlockReading->pprev;
    }

    CBigNum bnNew(PastDifficultyAverage);

    int64_t nTargetTimespan = CountBlocks*nTargetSpacing;

    if (nActualTimespan < nTargetTimespan/3)
        nActualTimespan = nTargetTimespan/3;
    if (nActualTimespan > nTargetTimespan*3)
        nActualTimespan = nTargetTimespan*3;

    // Retarget
    bnNew *= nActualTimespan;
    bnNew /= nTargetTimespan;

	if (bnNew > CBigNum(params.powLimit)) {
		bnNew = CBigNum(params.powLimit);
	}
    return bnNew.GetCompact();
}
//----------------------

unsigned int GetNextWorkRequired(const CBlockIndex* pindexLast, const CBlockHeader *pblock, const Consensus::Params& params) {
    if (params.fPowAllowMinDifficultyBlocks)  {

		 // Special difficulty rule for testnet:
		 // If the new block's timestamp is more than 2* 10 minutes
		 // then allow mining of a min-difficulty block.

		if (pblock->GetBlockTime() > pindexLast->GetBlockTime() + params.nPowTargetSpacing*2)
			return UintToArith256(params.powLimit).GetCompact();
    }

	if (pindexLast->nHeight >= (100000 - 1))
		return DarkGravityWave3(pindexLast, pblock, params);
    return DarkGravityWave(pindexLast, pblock, params);
}

static CBlock CreateGenesisBlock(const char* pszTimestamp, const CScript& genesisOutputScript, uint32_t nTime, uint32_t nNonce, uint32_t nBits, int32_t nVersion, const CAmount& genesisReward) {
	CMutableTransaction txNew;
	txNew.nVersion = 1;
	txNew.vin.resize(1);
	txNew.vout.resize(1);
	txNew.vin[0].scriptSig = CScript() << 486604799 << CScriptNum(4) << vector<unsigned char>((const unsigned char*)pszTimestamp, (const unsigned char*)pszTimestamp + strlen(pszTimestamp));
	txNew.vout[0].nValue = genesisReward;
	txNew.vout[0].scriptPubKey = genesisOutputScript;

	CBlock genesis;
	genesis.nTime    = nTime;
	genesis.nBits    = nBits;
	genesis.nNonce   = nNonce;
	genesis.nVersion = nVersion;
	genesis.vtx.push_back(MakeTransactionRef(std::move(txNew)));
	genesis.hashPrevBlock.SetNull();
	genesis.hashMerkleRoot = BlockMerkleRoot(genesis);
	return genesis;
}

/**
* Build the genesis block. Note that the output of its generation
* transaction cannot be spent since it did not originally exist in the
* database.
*
* CBlock(hash=000000000019d6, ver=1, hashPrevBlock=00000000000000, hashMerkleRoot=4a5e1e, nTime=1231006505, nBits=1d00ffff, nNonce=2083236893, vtx=1)
*   CTransaction(hash=4a5e1e, ver=1, vin.size=1, vout.size=1, nLockTime=0)
*     CTxIn(COutPoint(000000, -1), coinbase 04ffff001d0104455468652054696d65732030332f4a616e2f32303039204368616e63656c6c6f72206f6e206272696e6b206f66207365636f6e64206261696c6f757420666f722062616e6b73)
*     CTxOut(nValue=50.00000000, scriptPubKey=0x5F1DF16B2B704C8A578D0B)
*   vMerkleTree: 4a5e1e
*/
static CBlock CreateGenesisBlock(uint32_t nTime, uint32_t nNonce, uint32_t nBits, int32_t nVersion, const CAmount& genesisReward) {
	const char* pszTimestamp = "We always make good coin and future with perfect quality";
	const CScript genesisOutputScript = CScript() << ParseHex("04678afdb0fe5548271967f1a67130b7105cd6a828e03909a67962e0ea1f61deb649f6bc3f4cef38c4f35504e51ec112de5c384df7ba0b8d578a4c702b6bf11d5f") << OP_CHECKSIG;
	return CreateGenesisBlock(pszTimestamp, genesisOutputScript, nTime, nNonce, nBits, nVersion, genesisReward);
}

void CChainParams::UpdateVersionBitsParameters(Consensus::DeploymentPos d, int64_t nStartTime, int64_t nTimeout)
{
	consensus.vDeployments[d].nStartTime = nStartTime;
	consensus.vDeployments[d].nTimeout = nTimeout;
}

/**
 * Main network
 */
/**
 * What makes a good checkpoint block?
 * + Is surrounded by blocks with reasonable timestamps
 *   (no blocks before with a timestamp after, none after with
 *    timestamp before)
 * + Contains no strange transactions
 */

class CMainParams : public CChainParams {
public:
    CMainParams() {
        strNetworkID = "main";
	consensus.BIP16Height = 0;
	consensus.BIP34Height = 17;
	consensus.BIP34Hash = uint256();
	consensus.BIP66Height = 0;
	consensus.BIP65Height = 0;

        consensus.powLimit = uint256S("00000fffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.nPowTargetTimespan = 14 * 24 * 60 * 60; // two weeks
        consensus.nPowTargetSpacing = 60;
        consensus.fPowAllowMinDifficultyBlocks = false;
        consensus.nRuleChangeActivationThreshold = 1916; // 95% of 2016
	consensus.nMinerConfirmationWindow = 2016;
	consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
	consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 1199145601; // January 1, 2008
	consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = 1230767999; // December 31, 2008

	// Deployment of BIP68, BIP112, and BIP113.
	consensus.vDeployments[Consensus::DEPLOYMENT_CSV].bit = 0;
	consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nStartTime = Consensus::BIP9Deployment::ALWAYS_ACTIVE;
	consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nTimeout = Consensus::BIP9Deployment::NO_TIMEOUT;

	// Deployment of SegWit (BIP141 and BIP143)
	consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].bit = 1;
	consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nStartTime = Consensus::BIP9Deployment::ALWAYS_ACTIVE;
	consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nTimeout = Consensus::BIP9Deployment::NO_TIMEOUT;

        /**
        * The best chain should have at least this much work.
        * Don't set a value bigger than 0 if blockchain doesn't have any blocks yet.
        * Set your current ChainWork if you want nodes to wait until the whole blockchain is downloaded
        */
        consensus.nMinimumChainWork = uint256S("0x00");

        // By default assume that the signatures in ancestors of this block are valid.
        consensus.defaultAssumeValid = uint256S("0x000001731d9008f70a929cb01e2e7d6403e3b17f25360c43421bf34ad5316109"); //0 block

		/**
         * The message start string is designed to be unlikely to occur in normal data.
         * The characters are rarely used upper ASCII, not valid as UTF-8, and produce
         * a large 32-bit integer with any alignment.
         */
        pchMessageStart[0] = 0xf9;
        pchMessageStart[1] = 0xbe;
        pchMessageStart[2] = 0xb4;
        pchMessageStart[3] = 0xd4;

	nDefaultPort = 5332;
        nPruneAfterHeight = 10000000;

		// for (int nonce=1; nonce < 0x7FFFFFFF; ++nonce) {
		// 	genesis = CreateGenesisBlock(1530728779, nonce, 0x1e0fffff, 112, 0);
		// 	consensus.hashGenesisBlock = genesis.GetHash();
		// 	if (UintToArith256(consensus.hashGenesisBlock) < UintToArith256(consensus.powLimit)) {
		// 		printf("Wonderfull__mainnet_genesis.merklroot = %s\n", genesis.hashMerkleRoot.ToString().c_str());
		// 		printf("Wonderfull__mainnet_genesis.nonce = %d\n", genesis.nNonce);	
		// 		printf("Wonderfull__mainnet_genesis.version = %d\n", genesis.nVersion);
		// 		printf("Wonderfull__mainnet_genesis.bits = %x\n", genesis.nBits);
		// 		printf("Wonderfull__mainnet_genesis.time = %d\n", genesis.nTime);
		// 		printf("Wonderfull__mainnet_genesis.GetHash = %s\n", genesis.GetHash().ToString().c_str());
		// 		break;
		// 	}

		// }


		genesis = CreateGenesisBlock(1537197711, 475135, 0x1e0fffff, 112, 0);

        /**
         * Build the genesis block. Note that the output of its generation
         * transaction cannot be spent since it did not originally exist in the
         * database.
         *
         * CBlock(hash=000000000019d6, ver=1, hashPrevBlock=00000000000000, hashMerkleRoot=4a5e1e, nTime=1231006505, nBits=1d00ffff, nNonce=2083236893, vtx=1)
         *   CTransaction(hash=4a5e1e, ver=1, vin.size=1, vout.size=1, nLockTime=0)
         *     CTxIn(COutPoint(000000, -1), coinbase 04ffff001d0104455468652054696d65732030332f4a616e2f32303039204368616e63656c6c6f72206f6e206272696e6b206f66207365636f6e64206261696c6f757420666f722062616e6b73)
         *     CTxOut(nValue=50.00000000, scriptPubKey=0x5F1DF16B2B704C8A578D0B)
         *   vMerkleTree: 4a5e1e
         */
		/*!!!R
        CMutableTransaction txNew;
        txNew.vin.resize(1);
        txNew.vout.resize(1);
        genesis.vtx.push_back(txNew);
        genesis.hashPrevBlock.SetNull();
        genesis.hashMerkleRoot = genesis.BuildMerkleTree();
		*/

        consensus.hashGenesisBlock = genesis.GetHash();
		// printf("main_genesis.GetHash = %s\n", genesis.GetHash().ToString().c_str());
        // printf("main_genesis.merklroot = %s\n", genesis.hashMerkleRoot.ToString().c_str());
		assert(consensus.hashGenesisBlock == uint256S("0x000001731d9008f70a929cb01e2e7d6403e3b17f25360c43421bf34ad5316109"));
		assert(genesis.hashMerkleRoot == uint256S("0x7561b273ff730b41f39f620336b70456874cc51aa1622e745e23d1f1f263db8d"));

        // vSeeds.push_back("soferox.org");
		// vSeeds.push_back("electrum1.soferox.org");
		// vSeeds.push_back("electrum2.soferox.org");
		// vSeeds.push_back("jswallet.soferox.org");
		// vSeeds.push_back("soferosight.soferox.org");

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,63); // guarantees the first character is S
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,125); // guarantees the first character is s
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,190); // guarantees the first character is V
        base58Prefixes[EXT_PUBLIC_KEY] = boost::assign::list_of(0x04)(0x88)(0xB2)(0x1E).convert_to_container<std::vector<unsigned char> >();
        base58Prefixes[EXT_SECRET_KEY] = boost::assign::list_of(0x04)(0x88)(0xAD)(0xE4).convert_to_container<std::vector<unsigned char> >();

        bech32_hrp = "sfx";

        vFixedSeeds = std::vector<SeedSpec6>(pnSeed6_main, pnSeed6_main + ARRAYLEN(pnSeed6_main));

//!!!?        fMiningRequiresPeers = true;
        fDefaultConsistencyChecks = false;
        fRequireStandard = true;
        fMineBlocksOnDemand = false;

#ifdef _MSC_VER //!!!
		checkpointData = CCheckpointData{
#else
		checkpointData = (CCheckpointData) {
#endif
			{
				{0		, uint256S("0x000001731d9008f70a929cb01e2e7d6403e3b17f25360c43421bf34ad5316109")},
				// {28888, uint256S("0x00000000000228ce19f55cf0c45e04c7aa5a6a873ed23902b3654c3c49884502")},
				// {58888, uint256S("0x0000000000dd85f4d5471febeb174a3f3f1598ab0af6616e9f266b56272274ef")},
				// {111111, uint256S("0x00000000013de206275ee83f93bee57622335e422acbf126a37020484c6e113c")},
				// {1000000, uint256S("0x000000000df8560f2612d5f28b52ed1cf81b0f87ac0c9c7242cbcf721ca6854a")},
				// {2000000, uint256S("0x00000000000434d5b8d1c3308df7b6e3fd773657dfb28f5dd2f70854ef94cc66")},
			}
		};

		chainTxData = ChainTxData{
			1537197711, // * UNIX timestamp of last checkpoint block
			0,   // * total number of transactions between genesis and last checkpoint
						//   (the tx=... number in the SetBestChain debug.log lines)
			100.0     // * estimated number of transactions per day after checkpoint
		};
    }
};
static CMainParams mainParams;

/**
 * Testnet (v3)
 */
class CTestNetParams : public CMainParams {
public:
    CTestNetParams() {
        strNetworkID = "test";
        consensus.BIP16Height = 1;
		consensus.BIP34Height = 286;
		consensus.BIP34Hash = uint256S("0x0000004b7778ba253a75b716c55b2c6609b5fb97691b3260978f9ce4a633106d");
		consensus.BIP66Height = 286;
		consensus.BIP65Height = INT_MAX;	//!!!?

        consensus.nPowTargetSpacing = 60;
		consensus.fPowAllowMinDifficultyBlocks = true;
        consensus.nRuleChangeActivationThreshold = 1512; // 75% for testchains
		consensus.nMinerConfirmationWindow = 2016;
		consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
		consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 1199145601; // January 1, 2008
		consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = 1230767999; // December 31, 2008

		// Deployment of BIP68, BIP112, and BIP113.
		consensus.vDeployments[Consensus::DEPLOYMENT_CSV].bit = 0;
		consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nStartTime = 1535500800; // Aug 29, 2018
		consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nTimeout = 1546214400; // Dec 31, 2018

		// Deployment of SegWit (BIP141, BIP143, and BIP147)
		consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].bit = 1;
		consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nStartTime = 1535500800; // Aug 29, 2018
		consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nTimeout = 1546214400; // Dec 31, 2018

		consensus.powLimit = uint256S("000000ffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");

        // The best chain should have at least this much work.
        consensus.nMinimumChainWork = uint256S("0x000000000000000000000000000000000000000000000000000000000000ffff");

        // By default assume that the signatures in ancestors of this block are valid.
        consensus.defaultAssumeValid = uint256S("0x000000d6c9c013173e2313f24edc791a9134d379522901f7e40d9fc4f0e25bb0"); //525313

        pchMessageStart[0] = 0x0b;
        pchMessageStart[1] = 0x11;
        pchMessageStart[2] = 0x09;
        pchMessageStart[3] = 0x07;

        nDefaultPort = 16644;
        nPruneAfterHeight = 1000000;

/*!!!R		for (int nonce=1; nonce < 0x7FFFFFFF; ++nonce) {
			genesis = CreateGenesisBlock(1440000002, nonce, 0x1e00ffff, 3, 0);
			consensus.hashGenesisBlock = genesis.GetHash();
			if (UintToArith256(consensus.hashGenesisBlock) < UintToArith256(consensus.powLimit))
				break;
		}
		*/

        //! Modify the testnet genesis block so the timestamp is valid for a later start.

		// for (int nonce=1; nonce < 0x7FFFFFFF; ++nonce) {
		// 	genesis = CreateGenesisBlock(1530728750, nonce, 0x1e00ffff, 3, 0);
		// 	consensus.hashGenesisBlock = genesis.GetHash();
		// 	if (UintToArith256(consensus.hashGenesisBlock) < UintToArith256(consensus.powLimit)) {
		// 		printf("Wonderfull__testnet_genesis.GetHash = %s\n", genesis.GetHash().ToString().c_str());	
		// 		printf("Wonderfull__testnet_genesis.merklroot = %s\n", genesis.hashMerkleRoot.ToString().c_str());	
		// 		printf("Wonderfull__testnet_genesis.nonce = %d\n", genesis.nNonce);	
		// 		break;
		// 	}
	
		// }

		genesis = CreateGenesisBlock(1537197790, 2573276, 0x1e00ffff, 3, 0);
        consensus.hashGenesisBlock = genesis.GetHash();
		// printf("testnet_genesis.GetHash = %s\n", genesis.GetHash().ToString().c_str());
        // printf("testnet_genesis.merklroot = %s\n", genesis.hashMerkleRoot.ToString().c_str());
		// printf("testnet_nNounce = %d\n", genesis.nNonce);
        assert(consensus.hashGenesisBlock == uint256S("0x000000ae86d44fa10749fab5c0de4dafc6c96c008d6cf20c11c7190e221cf057"));

        vFixedSeeds.clear();
        vSeeds.clear();
		// vSeeds.push_back("testnet1.soferox.org");
		// vSeeds.push_back("testnet2.soferox.org");
		// vSeeds.push_back("testp2pool.soferox.org");
		// vSeeds.push_back("testp2pool2.soferox.org");

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,111);
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,196);
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,239);
        base58Prefixes[EXT_PUBLIC_KEY] = boost::assign::list_of(0x04)(0x35)(0x87)(0xCF).convert_to_container<std::vector<unsigned char> >();
        base58Prefixes[EXT_SECRET_KEY] = boost::assign::list_of(0x04)(0x35)(0x83)(0x94).convert_to_container<std::vector<unsigned char> >();

        bech32_hrp = "tsfx";

        vFixedSeeds = std::vector<SeedSpec6>(pnSeed6_test, pnSeed6_test + ARRAYLEN(pnSeed6_test));

//!!!?        fMiningRequiresPeers = false;			//SFX  Testnet can have single node
        fDefaultConsistencyChecks = false;
        fRequireStandard = false;
        fMineBlocksOnDemand = false;


#ifdef _MSC_VER
		checkpointData = CCheckpointData{
#else
		checkpointData = (CCheckpointData) {
#endif
			{
				 //{0		, uint256S("0xbef0b3124119637ecd010cf263a47192eca747e9ef9c4d4f43eacb8fc2266644")},
				// { 50000 , uint256S("0x00000081951486bb535f8cffec8ac0641bd24b814f89641f6cc2cad737f18950") },
			}
		};

		chainTxData = ChainTxData{
			1537197790,
			0,
			10
		};
    }
};
static CTestNetParams testNetParams;

/**
 * Regression test
 */
class CRegTestParams : public CMainParams {
public:
    CRegTestParams() {
        strNetworkID = "regtest";

        consensus.BIP16Height = 0; // always enforce P2SH BIP16 on regtest
        consensus.powLimit = uint256S("00ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.nPowTargetSpacing = 1;
		consensus.fPowAllowMinDifficultyBlocks = true;
        consensus.nRuleChangeActivationThreshold = 108; // 75% for testchains
		consensus.nMinerConfirmationWindow = 144; // Faster than normal for regtest (144 instead of 2016)
		consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
		consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 0;
		consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = Consensus::BIP9Deployment::NO_TIMEOUT;
		consensus.vDeployments[Consensus::DEPLOYMENT_CSV].bit = 0;
		consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nStartTime = 0;
		consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nTimeout = Consensus::BIP9Deployment::NO_TIMEOUT;
		consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].bit = 1;
		consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nStartTime = Consensus::BIP9Deployment::ALWAYS_ACTIVE;
		consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nTimeout = Consensus::BIP9Deployment::NO_TIMEOUT;

        // The best chain should have at least this much work.
        consensus.nMinimumChainWork = uint256S("0x00");

        // By default assume that the signatures in ancestors of this block are valid.
        consensus.defaultAssumeValid = uint256S("0x00");

		pchMessageStart[0] = 0xfa;
        pchMessageStart[1] = 0xbf;
        pchMessageStart[2] = 0xb5;
        pchMessageStart[3] = 0xda;
        nDefaultPort = 18888;

        nPruneAfterHeight = 1000;

		genesis = CreateGenesisBlock(1440000002, 6556309, 0x1e00ffff, 3, 0);
		consensus.hashGenesisBlock = genesis.GetHash();
		assert(consensus.hashGenesisBlock == uint256S("0x000000ffbb50fc9898cdd36ec163e6ba23230164c0052a28876255b7dcf2cd36"));

		vFixedSeeds.clear(); //!< Regtest mode doesn't have any fixed seeds.
		vSeeds.clear();      //!< Regtest mode doesn't have any DNS seeds.

        fDefaultConsistencyChecks = true;
        fRequireStandard = false;
        fMineBlocksOnDemand = true;

#ifdef _MSC_VER
		checkpointData = CCheckpointData{
#else
		checkpointData = (CCheckpointData) {
#endif
			{
				// {0, uint256S("0x000000ffbb50fc9898cdd36ec163e6ba23230164c0052a28876255b7dcf2cd36")},
			}
		};

		chainTxData = ChainTxData{
			0,
			0,
			0
		};

		base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1, 111);
		base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1, 196);
		base58Prefixes[SECRET_KEY] = std::vector<unsigned char>(1, 239);
		base58Prefixes[EXT_PUBLIC_KEY] = { 0x04, 0x35, 0x87, 0xCF };
		base58Prefixes[EXT_SECRET_KEY] = { 0x04, 0x35, 0x83, 0x94 };

        bech32_hrp = "sfxrt";
    }
};

static std::unique_ptr<CChainParams> globalChainParams;

const CChainParams &Params() {
	assert(globalChainParams);
	return *globalChainParams;
}

std::unique_ptr<CChainParams> CreateChainParams(const std::string& chain)
{
	if (chain == CBaseChainParams::MAIN)
		return std::unique_ptr<CChainParams>(new CMainParams());
	else if (chain == CBaseChainParams::TESTNET)
		return std::unique_ptr<CChainParams>(new CTestNetParams());
	else if (chain == CBaseChainParams::REGTEST)
		return std::unique_ptr<CChainParams>(new CRegTestParams());
	throw std::runtime_error(strprintf("%s: Unknown chain %s.", __func__, chain));
}

void SelectParams(const std::string& network)
{
	SelectBaseParams(network);
	globalChainParams = CreateChainParams(network);
}

void UpdateVersionBitsParameters(Consensus::DeploymentPos d, int64_t nStartTime, int64_t nTimeout)
{
	globalChainParams->UpdateVersionBitsParameters(d, nStartTime, nTimeout);
}
