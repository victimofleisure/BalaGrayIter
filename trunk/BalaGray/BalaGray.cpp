// Copyleft 2023 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      26jan23	initial version
		01		05may24	fix wrongly named member var (cosmetic)
		02		12dec24	add wrap prediction; improve error handling
		03		04aug25	add standard deviation

*/

// BalaGray.cpp : Defines the entry point for the console application.
// This app computes balanced Gray code sequences, for use in music theory.

#include "stdafx.h"	// precompiled header
#include "stdint.h"	// standard sizes
#include "vector"	// growable array
#include "fstream"	// file I/O
#include "assert.h"	// debugging
#include "WorkerSync.h"	// synchronized worker thread
#include <iomanip>

#define MORE_PLACES 1	// set non-zero to use more than four places
#define DO_PRUNING 1	// set non-zero to do branch pruning and reduce runtime
#define START_2_DOWN 1	// set non-zero to skip first two levels of crawl
#define SHOW_STATS 0	// set non-zero to compute and show crawl statistics
#define PREDICT_WRAP 1	// set non-zero to predict and abandon branches that won't wrap around Gray
#define OPT_STD_DEV 1	// set non-zero to optimize standard deviation: 1 == standard deviation is
						// max span tie-breaker; 2 == standard deviation only, ignoring max span

class CBalaGray {
public:
// Construction
	CBalaGray(const char *pszOutPath = NULL);

// Constants
	enum {
#if MORE_PLACES
		MAX_PLACES = 8,
#else
		MAX_PLACES = 4,
#endif
	};

// Types
	typedef uint8_t PLACE;	// 8 bits is enough for atonal music theory as bases don't exceed twelve
	typedef uint32_t SET_CODE;	// specifies a mixed-radix numeral's bases, using one nibble per place
	union NUMERAL {	// mixed-radix numeral with a variable number of places up to MAX_PLACES
		PLACE	b[MAX_PLACES];	// array of places; their bases are assumed to be known
#if MORE_PLACES
		uint64_t	dw;	// double word containing all places
#else
		uint32_t	dw;	// double word containing all places
#endif
	};
	typedef std::vector<NUMERAL> CNumeralArray;
	class CWinner {	// info about winning permutation
	public:
		CWinner();
		SET_CODE	m_nSetCode;	// set identifier in hexadecimal; specifies base of each place
		int		m_nPlaces;		// how many places numeral has
		int		m_nBaseSum;		// sum of numeral's bases
		int		m_nImbalance;	// difference between minimum and maximum transition counts
		int		m_nMaxTrans;	// maximum transition count
		int		m_nMaxSpan;		// maximum span length
#if OPT_STD_DEV
		double	m_fStdDev;		// standard deviation of span lengths compared to ideal mean
#endif
		bool	m_bIsProven;	// true if all permutations were tried
		CNumeralArray	m_arrNum;	// array of mixed-radix numerals
		friend std::ofstream& operator<<(std::ofstream& ofs, const CWinner& winner);
		friend std::ifstream& operator>>(std::ifstream& ifs, CWinner& winner);
	};
	class CWinnerArray : public std::vector<CWinner> {	// array of winners
	public:
		void	Read(const char *pszPath);
		void	Write(const char *pszPath) const;
		friend std::ofstream& operator<<(std::ofstream& ofs, const CWinnerArray& arrWin);
		friend std::ifstream& operator>>(std::ifstream& ifs, CWinnerArray& arrWin);
	};

// Attributes
	int		GetNumeralCount() const { return static_cast<int>(m_arrNum.size()); }
	void	SetPruneMaxTrans(int nThreshold) { m_nPruneMaxTrans = nThreshold; }
	void	SetPruneImbalance(int nThreshold) { m_nPruneImbalance = nThreshold; }
	static	int		GetBases(SET_CODE nSetCode, NUMERAL& arrBase);

// Operations
	void	Reset();
	int		Pack(const NUMERAL& num) const;
	NUMERAL	Unpack(int iNumeral) const;
	bool	Calc(int nPlaces, const PLACE *parrBase, CWinner& seqWinner);
	bool	CalcFromCode(SET_CODE SetCode, CWinner& seqWinner);
	void	Cancel() { m_bCancel = true; }

protected:
// Constants
	enum {
		ULONGLONG_BITS = sizeof(uint64_t) * 8,	// number of bits in a long long word
	};
	enum {	// pruning thresholds may require manual tuning; see notes in set list
		PRUNE_MAXTRANS = INT_MAX,	// prune branch if maximum transition count exceeds this value
		PRUNE_IMBALANCE = 3,	// prune branch if imbalance exceeds this value
	};

// Types
	struct STATE {	// crawler stack element
		PLACE	iNum;		// index into numeral array
		PLACE	iGray;		// index into Gray successor array
		NUMERAL	nTrans;		// transition counts, one per place
	};
	typedef std::vector<PLACE> CPlaceArray;	// array of places
	typedef std::vector<STATE> CStateArray;	// array of states

// Member data
	int		m_nPlaces;	// number of places
	int		m_nGraySuccessors;	// number of Gray successors a numeral can have
	int		m_nGrayStrideShift;	// stride of Gray successors array, as a per-row shift in bits
	int		m_nPruneMaxTrans;	// prune branch if its maximum transition count exceeds this threshold
	int		m_nPruneImbalance;	// prune branch if its imbalance exceeds this threshold
	CPlaceArray	m_arrBase;	// array of bases, one for each place of numeral
	CNumeralArray	m_arrNum;	// array of numerals
	CPlaceArray	m_arrGraySuccessor;	// 2D table of Gray successors for each numeral
	CStateArray	m_arrState;	// array of states; crawler stack
	std::ofstream	m_fOut;	// output file
	volatile bool	m_bCancel;	// cancel flag

// Helpers
	bool	MakeNumerals(int nPlaces, const PLACE *parrBase);
	void	MakeGraySuccessorTable();
	void	DumpGraySuccessorTable() const;
	void	DumpNumeral(const NUMERAL& num) const;
	void	DumpNumerals() const;
	void	DumpSet() const;
	void	DumpPermutation() const;
	void	WriteBalanceToLog(int nImbalance, int nMaxTrans, int nMaxSpan);
	void	WriteBalanceToLog(int nImbalance, int nMaxTrans, int nMaxSpan, double fStdDev);
	void	WritePermutationToLog();
	bool	IsGray(NUMERAL num1, NUMERAL num2) const;
	int		ComputeBalance(int iDepth, int& nMaxTrans, NUMERAL& nTransCounts) const;
	int		ComputeMaxSpan(int iDepth) const;
	double	ComputeStdDev() const;
	int		CalcDeviance(int nSamp) const;
};

CBalaGray::CBalaGray(const char *pszOutPath)
{
	if (pszOutPath == NULL)
		pszOutPath = "BalaGrayIter.txt";
	m_fOut.open(pszOutPath, std::ios_base::out);	// open output file
	assert(m_fOut.good());
	if (!m_fOut.good()) {
		printf("can't open output file '%s'\n", pszOutPath);
	}
	Reset();
	m_nPruneMaxTrans = PRUNE_MAXTRANS;
	m_nPruneImbalance = PRUNE_IMBALANCE;
}

void CBalaGray::Reset()
{
	m_nPlaces = 0;
	m_arrBase.clear();
	m_arrState.clear();
	m_bCancel = false;
}

int CBalaGray::Pack(const NUMERAL& num) const
{
	int	iNumeral = num.b[m_nPlaces - 1];	// initialize index to first place
	for (int iPlace = m_nPlaces - 2; iPlace >= 0; iPlace--) {	// for each subsequent place
		iNumeral *= m_arrBase[iPlace];	// multiply index by place's base
		iNumeral += num.b[iPlace];	// add place to index
	}
	return iNumeral;	// return numeral's index
}

CBalaGray::NUMERAL CBalaGray::Unpack(int iNumeral) const
{
	NUMERAL	num;
	num.dw = 0;
	for (int iPlace = 0; iPlace < m_nPlaces; iPlace++) {	// for each place
		int	nBase = m_arrBase[iPlace];
		num.b[iPlace] = iNumeral % nBase;
		iNumeral /= nBase;
	}
	return num;	// return numeral
}

int CBalaGray::GetBases(SET_CODE nSetCode, NUMERAL& arrBase)
{
	// A set code compactly specifies the bases of a mixed-radix numeral.
	// Each nibble of the set code specifies one of the numeral's bases.
	// Set codes are big endian. When a set code is shown in hexadecimal,
	// its leftmost digit corresponds to the numeral's least significant
	// place. For example, set code 0x234 produces this base array:
	// arrBase[0] = 2; arrBase[1] = 3; arrBase[2] = 4;
	arrBase.dw = 0;
	int	nPlaces = 0;
	while (nSetCode) {	// while set nibbles remain
		if (nPlaces >= MAX_PLACES)	// if set has too many places
			return 0;	// return zero to indicate error
		arrBase.b[nPlaces] = nSetCode & 0xf;	// mask off low-order nibble
		nPlaces++;	// another base was stored
		nSetCode >>= 4;	// shift code down to expose next nibble
	}
	// reverse order of base array to account for set code's big endianness
	for (int iPlace = 0; iPlace < nPlaces / 2; iPlace++) {
		PLACE	tmp = arrBase.b[iPlace];
		arrBase.b[iPlace] = arrBase.b[nPlaces - 1 - iPlace];
		arrBase.b[nPlaces - 1 - iPlace] = tmp;
	}
	return nPlaces;
}

bool CBalaGray::MakeNumerals(int nPlaces, const PLACE *parrBase)
{
	assert(parrBase != NULL);
	m_nPlaces = nPlaces;
	m_arrBase.resize(nPlaces);
	// compute range of mixed radix numeral from its bases
	int	nNums = 1;
	for (int iPlace = 0; iPlace < nPlaces; iPlace++) {	// for each place
		if (parrBase[iPlace] < 2) {	// radix must be at least binary
			printf("radix too small\n");
			return false;
		}
		m_arrBase[iPlace] = parrBase[iPlace];	// store base in member var
		nNums *= parrBase[iPlace];	// update range
	}
	// make array of all numerals representable with the specified bases
	if (nNums > ULONGLONG_BITS * 2 - 1) {	// limit is maximum shift, which is 127 bits
		printf("too many numerals\n");
		return false;
	}
	m_arrNum.resize(nNums);
	for (int iNum = 0; iNum < nNums; iNum++) {	// for each numeral
		m_arrNum[iNum] = Unpack(iNum);	// convert numeral index into corresponding numeral
	}
	return true;
}

void CBalaGray::MakeGraySuccessorTable()
{
	// Build 2D table of possible Gray successors from each numeral.
	// One row for each numeral, one column for each Gray successor.
	// Successors are stored not as numerals, but as numeral indices.
	// Each table element is an index into the numeral array.
	int	nPlaces = m_nPlaces;
	int	nGraySuccessors = 0;
	for (int iPlace = 0; iPlace < nPlaces; iPlace++) {	// for each place
		nGraySuccessors += m_arrBase[iPlace] - 1;	// one less than place's base
	}
	// Compute stride of Gray successors table; to avoid multiplication,
	// round up stride to nearest power of two and convert it to a shift.
	unsigned long	iFirstBitPos;
	_BitScanReverse(&iFirstBitPos, nGraySuccessors - 1);
	int	nStrideShift = 1 << iFirstBitPos;
	m_arrGraySuccessor.resize(m_arrNum.size() << nStrideShift);
	int	nNums = GetNumeralCount();
	for (int iNum = 0; iNum < nNums; iNum++) {	// for each numeral
		int	iCol = 0;
		NUMERAL	rowNum, colNum;
		rowNum.dw = m_arrNum[iNum].dw;
		for (int iPlace = 0; iPlace < nPlaces; iPlace++) {	// for each place
			int	nVals = m_arrBase[iPlace];	// number of values is place's base
			for (int iVal = 0; iVal < nVals; iVal++) {	// for each of place's values
				if (iVal != rowNum.b[iPlace]) {	// if value differs from row value
					colNum.dw = rowNum.dw;	// column numeral is same as row numeral
					colNum.b[iPlace] = iVal;	// except one place differs (Gray code)
					m_arrGraySuccessor[(iNum << nStrideShift) + iCol] = Pack(colNum);
					iCol++;	// next column
				}
			}
		}
	}
	m_nGraySuccessors = nGraySuccessors;	// save successor count in member var
	m_nGrayStrideShift = nStrideShift;	// save table stride too
}

void CBalaGray::DumpNumeral(const NUMERAL& num) const
{
	printf("[");
	for (int iPlace = 0; iPlace < m_nPlaces; iPlace++) {	// for each place
		printf("%d ", num.b[iPlace]);
	}
	printf("]");
}

void CBalaGray::DumpNumerals() const
{
	int	nNums = GetNumeralCount();
	for (int iNum = 0; iNum < nNums; iNum++) {
		for (int iPlace = 0; iPlace < m_nPlaces; iPlace++) {	// for each place
			printf("%d ", m_arrNum[iNum].b[iPlace]);
		}
		printf("\n");
	}
}

void CBalaGray::DumpGraySuccessorTable() const
{
	int	nNums = GetNumeralCount();
	for (int iNum = 0; iNum < nNums; iNum++) {	// for each numeral
		DumpNumeral(m_arrNum[iNum]);
		printf(": ");
		for (int iGray = 0; iGray < m_nGraySuccessors; iGray++) {	// for each Gray successor
			int	iSuccessor = m_arrGraySuccessor[(iNum << m_nGrayStrideShift) + iGray];
			DumpNumeral(m_arrNum[iSuccessor]);
		}
		printf("\n");
	}
}

void CBalaGray::DumpSet() const
{
	printf("[");
	for (int iPlace = 0; iPlace < m_nPlaces; iPlace++) {	// for each place
		printf("%X", m_arrBase[iPlace]);
	}
	printf("]\n");
}

void CBalaGray::DumpPermutation() const
{
	int	nPerms = GetNumeralCount();
	for (int iPlace = 0; iPlace < m_nPlaces; iPlace++) {	// for each place
		for (int iPerm = 0; iPerm < nPerms; iPerm++) {
			printf("%d ", int(m_arrNum[m_arrState[iPerm].iNum].b[iPlace]));
		}
		printf("\n");
	}
}

void CBalaGray::WriteBalanceToLog(int nImbalance, int nMaxTrans, int nMaxSpan)
{
	m_fOut << "balance = " << nImbalance << ", maxtrans = " << nMaxTrans << ", maxspan = " << nMaxSpan << '\n';
}

void CBalaGray::WriteBalanceToLog(int nImbalance, int nMaxTrans, int nMaxSpan, double fStdDev)
{
	m_fOut << "balance = " << nImbalance << ", maxtrans = " << nMaxTrans << ", maxspan = " << nMaxSpan << ", stddev = " << fStdDev << '\n';
}

void CBalaGray::WritePermutationToLog()
{
	int	nPerms = GetNumeralCount();
	for (int iPlace = 0; iPlace < m_nPlaces; iPlace++) {	// for each place
		for (int iPerm = 0; iPerm < nPerms; iPerm++) {
			m_fOut << int(m_arrNum[m_arrState[iPerm].iNum].b[iPlace]) << ' ';
		}
		m_fOut << '\n';
	}
	m_fOut << '\n';
}

__forceinline bool CBalaGray::IsGray(NUMERAL num1, NUMERAL num2) const
{
	// Returns true if the given numerals differ by exactly one place.
	bool	bDiff = false;
	int	nPlaces = m_nPlaces;
	for (int iPlace = 0; iPlace < nPlaces; iPlace++) {	// for each place
		if (num1.b[iPlace] != num2.b[iPlace]) {	// if places differ
			if (!bDiff) {	// if first difference
				bDiff = true;	// set flag
			} else {	// not first difference
				return false;	// not Gray; early out
			}
		}
	}
	return bDiff;
}

bool CBalaGray::Calc(int nPlaces, const PLACE *parrBase, CWinner& seqWinner)
{
	assert(parrBase != NULL);
	if (nPlaces < 2 || nPlaces > MAX_PLACES) {
		printf("invalid place count\n");
		return false;
	}
	if (!m_fOut.good()) {	// if output file isn't open
		return false;	// ctor already reported error
	}
	Reset();
	if (!MakeNumerals(nPlaces, parrBase))
		return false;
	MakeGraySuccessorTable();
//	DumpNumerals();
//	DumpGraySuccessorTable();
	int	nGraySuccessors = m_nGraySuccessors;
	int	nGrayStrideShift = m_nGrayStrideShift;
	DumpSet();
	int	nNumerals = GetNumeralCount();
	printf("nPlaces=%d\n", nPlaces);
	printf("nValues=%d\n", nNumerals);
	int	nBestImbalance = INT_MAX;
	int	nBestMaxTrans = INT_MAX;
	int	nBestMaxSpan = INT_MAX;
	double	fBestStdDev = DBL_MAX;
	CPlaceArray	m_arrBestPerm;
	m_arrBestPerm.resize(nNumerals);
	m_arrState.resize(nNumerals);
	uint64_t	nPasses = 0;
	uint64_t	nGrays = 0;
	uint64_t	nOptimals = 0;
	uint64_t	nNumeralUsedMask[2] = {0};	// need 128 bits, as number of numerals may exceed 64
#if PREDICT_WRAP
	uint64_t	nGrayWrapMask = 0;
	for (int iOrgSucc = 0; iOrgSucc < m_nGraySuccessors; iOrgSucc++) {	// for each successor of origin
		int	nShift = m_arrGraySuccessor[iOrgSucc];
		if (nShift >= ULONGLONG_BITS) {	// if shift too big
			printf("wrap prediction shift too big\n");
			return false;
		}
		nGrayWrapMask |= 1ull << nShift;	// set successor's corresponding bit in mask
	}
#endif
#if START_2_DOWN
	int	iDepth = 2;	// first two levels are constant to save time; all sequences start with 0, 1
	m_arrState[1].iNum = 1;
	m_arrState[1].nTrans.b[0] = 1;
	nNumeralUsedMask[0] = 0x3;
#else
	int	iDepth = 1;	// first level is constant to save time; all sequences start with 0
	nNumeralUsedMask[0] = 0x1;
#endif
	int	nStartDepth = iDepth;
	while (!m_bCancel) {	// while cancel not requested
#if SHOW_STATS
		nPasses++;
#endif
		int	iPrevNum = m_arrState[iDepth - 1].iNum;
		int	iGray = m_arrState[iDepth].iGray;
		int	iNum = m_arrGraySuccessor[(iPrevNum << nGrayStrideShift) + iGray];	// optimized 2D table addressing
		int	iUsedMask = iNum >= ULONGLONG_BITS;	// index selects one of two 64-bit masks
		uint64_t	nNumeralMask = 1ull << (iNum & (ULONGLONG_BITS - 1));
#if PREDICT_WRAP
		if (!(nNumeralUsedMask[iUsedMask] & nNumeralMask)	// if numeral hasn't been used yet on this branch
		&& (nNumeralUsedMask[0] & nGrayWrapMask) != nGrayWrapMask) {	// and at least one origin successor remains unused
#else
		if (!(nNumeralUsedMask[iUsedMask] & nNumeralMask)) {	// if numeral hasn't been used yet on this branch
#endif
			m_arrState[iDepth].iNum = iNum;	// save numeral index on stack
			int	nMaxTrans;
			NUMERAL	nTransCounts;
			int	nImbalance = ComputeBalance(iDepth, nMaxTrans, nTransCounts);
			if (iDepth < nNumerals - 1) {	// if incomplete permutation
#if DO_PRUNING
				if (nMaxTrans > m_nPruneMaxTrans || nImbalance > m_nPruneImbalance) {
					goto lblPrune;	// abandon this branch
				}
#endif
				// crawl one level deeper
				nNumeralUsedMask[iUsedMask] |= nNumeralMask;	// mark this numeral as used
				m_arrState[iDepth].nTrans.dw = nTransCounts.dw;	// save current transition counts on stack
				iDepth++;	// increment depth to next numeral
				m_arrState[iDepth].iGray = 0;	// reset index of Gray transitions
				m_arrState[iDepth].iNum = 0;	// reset numeral index
				continue;	// equivalent to recursion, but less overhead
			} else {	// reached a leaf: complete permutation, a potential winner
#if !PREDICT_WRAP	// only need to check for Gray wrap if wrap prediction is disabled
				// if branch doesn't wrap around Gray (first and last numeral differ by more than one place)
				if (!IsGray(m_arrNum[m_arrState[0].iNum], m_arrNum[m_arrState[nNumerals - 1].iNum])) {
					goto lblPrune;	// abandon this branch
				}
#endif
#if SHOW_STATS
				nGrays++;	// count another Gray permutation
#endif
				// if max transition count or imbalance are worse than our current bests
				if (nMaxTrans > nBestMaxTrans || nImbalance > nBestImbalance) {
					goto lblPrune;	// abandon this branch
				}
				int	nMaxSpan = ComputeMaxSpan(iDepth);	// compute maximum span length
#if OPT_STD_DEV == 1	// if standard deviation is max span tie-breaker
				// if max transition count and imbalance equal our current bests
				if (nMaxTrans == nBestMaxTrans && nImbalance == nBestImbalance) {
					if (nMaxSpan > nBestMaxSpan) {	// if max span worsened
						goto lblPrune;	// abandon this branch
					}
				}
				double fStdDev = ComputeStdDev();	// compute standard deviation
				if (nMaxTrans == nBestMaxTrans && nImbalance == nBestImbalance && nMaxSpan == nBestMaxSpan) {
					if (fStdDev >= fBestStdDev) {	// if standard deviation didn't improve
#if SHOW_STATS
						if (nMaxSpan == nBestMaxSpan)
							nOptimals++;
#endif
						goto lblPrune;	// abandon this branch
					}
				}
#elif OPT_STD_DEV == 2	// else if standard deviation only, ignoring max span
				double fStdDev = ComputeStdDev();	// compute standard deviation
				if (nMaxTrans == nBestMaxTrans && nImbalance == nBestImbalance) {
					if (fStdDev >= fBestStdDev) {	// if standard deviation didn't improve
#if SHOW_STATS
						if (nMaxSpan == nBestMaxSpan)
							nOptimals++;
#endif
						goto lblPrune;	// abandon this branch
					}
				}
#else	// else not optimizing standard deviation; max span only
				// if max transition count and imbalance equal our current bests
				if (nMaxTrans == nBestMaxTrans && nImbalance == nBestImbalance) {
					if (nMaxSpan >= nBestMaxSpan) {	// if max span didn't improve
#if SHOW_STATS
						if (nMaxSpan == nBestMaxSpan)
							nOptimals++;
#endif
						goto lblPrune;	// abandon this branch
					}
				}
#endif // OPT_STD_DEV
				// we have a winner, until a better permutation comes along
				nBestMaxTrans = nMaxTrans;	// update best max transition count
				nBestImbalance = nImbalance;	// update best imbalance
				nBestMaxSpan = nMaxSpan;	// update best maximum span length
#if OPT_STD_DEV
				fBestStdDev = fStdDev;
				printf("balance = %d, maxtrans = %d, maxspan = %d, stddev = %f\n", nImbalance, nMaxTrans, nMaxSpan, fStdDev);
				WriteBalanceToLog(nImbalance, nMaxTrans, nMaxSpan, fStdDev);
#else
				printf("balance = %d, maxtrans = %d, maxspan = %d\n", nImbalance, nMaxTrans, nMaxSpan);
				WriteBalanceToLog(nImbalance, nMaxTrans, nMaxSpan);
#endif // OPT_STD_DEV
				WritePermutationToLog();
				for (int iNum = 0; iNum < nNumerals; iNum++) {	// for each numeral
					m_arrBestPerm[iNum] = m_arrState[iNum].iNum;	// update best permutation's numeral indices
				}
#if SHOW_STATS
				nOptimals = 1;	// first instance of new optimality
#endif
			}
		}
		m_arrState[iDepth].iGray++;	// increment Gray transitions index
		if (m_arrState[iDepth].iGray >= nGraySuccessors) {	// if no Gray successors remain for this numeral
lblPrune:
			if (iDepth <= nStartDepth) {	// if we're at same level where we started
				break;	// exit main loop
			} else {	// sufficient levels remain above us
				iDepth--;	// back up a level
				// restore bitmask that keeps track of which numerals we've used on this branch
				int	iNum = m_arrState[iDepth].iNum;	// number of numerals may exceed 64
				int	iUsedMask = iNum >= ULONGLONG_BITS;	// index selects one of two 64-bit masks
				uint64_t	nNumeralMask = 1ull << (iNum & (ULONGLONG_BITS - 1));
				nNumeralUsedMask[iUsedMask] &= ~nNumeralMask;	// mark this numeral as available again
				m_arrState[iDepth].iGray++;	// increment was skipped by continue statement above
				if (m_arrState[iDepth].iGray >= nGraySuccessors) {	// if no Gray successors remain for this numeral
					goto lblPrune;	// keep backing up
				}
			}
		}
	}
#if SHOW_STATS
	printf("nPasses = %lld nGrays = %lld nOptimals = %lld\n", nPasses, nGrays, nOptimals);
#endif
	// pass winning sequence back to caller
	seqWinner.m_nPlaces = nPlaces;
	seqWinner.m_nBaseSum = 0;
	for (int iPlace = 0; iPlace < nPlaces; iPlace++) {
		seqWinner.m_nBaseSum += parrBase[iPlace];
	}
	seqWinner.m_nImbalance = nBestImbalance;
	seqWinner.m_nMaxTrans = nBestMaxTrans;
	seqWinner.m_nMaxSpan = nBestMaxSpan;
#if OPT_STD_DEV
	seqWinner.m_fStdDev = fBestStdDev;
#endif
	seqWinner.m_bIsProven = !m_bCancel;
	seqWinner.m_arrNum.resize(nNumerals);
	for (int iNum = 0; iNum < nNumerals; iNum++) {
		seqWinner.m_arrNum[iNum].dw = m_arrNum[m_arrBestPerm[iNum]].dw;
	}
	return true;
}

__forceinline int CBalaGray::ComputeBalance(int iDepth, int& nMaxTrans, NUMERAL& nTransCounts) const
{
	int	nPlaces = m_nPlaces;
	NUMERAL	nTrans;
	nTrans.dw = m_arrState[iDepth - 1].nTrans.dw;	// load latest transition counts from stack
	// compare current state to previous state
	NUMERAL	sPrev, sCur;
	sPrev.dw = m_arrNum[m_arrState[iDepth - 1].iNum].dw;
	sCur.dw = m_arrNum[m_arrState[iDepth].iNum].dw;
	for (int iPlace = 0; iPlace < nPlaces; iPlace++) {	// for each place
		if (sCur.b[iPlace] != sPrev.b[iPlace]) {	// if place transitioned
			nTrans.b[iPlace]++;	// increment place's transition count
		}
	}
	nTransCounts = nTrans;	// order matters; counts passed back to caller must exclude wraparound
	// account for wraparound; compare current state to initial state, which is assumed to be zero
	for (int iPlace = 0; iPlace < nPlaces; iPlace++) {	// for each place
		if (sCur.b[iPlace]) {	// if place transitioned
			nTrans.b[iPlace]++;	// increment place's transition count
		}
	}
	// now that we have latest transition counts, compute their min and max
	int	nMin = nTrans.b[0];	// initialize min and max to first transition count
	int	nMax = nTrans.b[0];
	for (int iPlace = 1; iPlace < nPlaces; iPlace++) {	// for each transition count, excluding first
		int	n = nTrans.b[iPlace];
		if (n < nMin)	// if less than min
			nMin = n;	// update min
		if (n > nMax)	// if greater than max
			nMax = n;	// udpate max
	}
	nMaxTrans = nMax;
	return nMax - nMin;	// return difference
}

__forceinline int CBalaGray::ComputeMaxSpan(int iDepth) const
{
	int	arrSpan[MAX_PLACES];
	int	arrFirstSpan[MAX_PLACES];
	for (int iPlace = 0; iPlace < m_nPlaces; iPlace++) {	// for each place
		arrSpan[iPlace] = 1;	// initial span length is one
		arrFirstSpan[iPlace] = 0;	// first span length not set
	}
	int	nMaxSpan = 1;
	NUMERAL	sFirst, sPrev;
	sFirst.dw = m_arrNum[m_arrState[0].iNum].dw;	// store first state
	sPrev.dw = sFirst.dw;
	for (int iState = 1; iState <= iDepth; iState++) {	// for each state, excluding first
		NUMERAL	s;
		s.dw = m_arrNum[m_arrState[iState].iNum].dw;	// compare this state to previous state
		for (int iPlace = 0; iPlace < m_nPlaces; iPlace++) {	// for each place
			if (s.b[iPlace] != sPrev.b[iPlace]) {	// if place transitioned
				if (arrSpan[iPlace] > nMaxSpan)	// if span length exceeds max
					nMaxSpan = arrSpan[iPlace];	// update max span length
				if (!arrFirstSpan[iPlace])	// if first span length hasn't been set
					arrFirstSpan[iPlace] = arrSpan[iPlace];	// save first span length
				arrSpan[iPlace] = 1;	// reset span length
			} else {	// place didn't transition
				arrSpan[iPlace]++;	// increment span length
			}
		}
		sPrev = s;	// update previous state
	}
	// wrap around from last to first state
	for (int iPlace = 0; iPlace < m_nPlaces; iPlace++) {	// for each place
		if (sFirst.b[iPlace] != sPrev.b[iPlace]) {	// if place transitioned
			if (arrSpan[iPlace] > nMaxSpan)	// if span length exceeds max
				nMaxSpan = arrSpan[iPlace];	// update max span length
		} else {	// place didn't transition
			arrSpan[iPlace] += arrFirstSpan[iPlace];	// compute wrapped span length
			if (arrSpan[iPlace] > nMaxSpan)	// if span length exceeds max
				nMaxSpan = arrSpan[iPlace];	// update max span length
		}
	}
	return nMaxSpan;
}

__forceinline int CBalaGray::CalcDeviance(int nSamp) const
{
	int	nDev = nSamp - m_nPlaces;	// deviation from mean
	return nDev * nDev;	// squared
}

double CBalaGray::ComputeStdDev() const
{
	int	arrSpan[MAX_PLACES];
	int	arrFirstSpan[MAX_PLACES];
	for (int iPlace = 0; iPlace < m_nPlaces; iPlace++) {	// for each place
		arrSpan[iPlace] = 1;	// initial span length is one
		arrFirstSpan[iPlace] = 0;	// first span length not set
	}
	NUMERAL	sFirst, sPrev;
	sFirst.dw = m_arrNum[m_arrState[0].iNum].dw;	// store first state
	sPrev.dw = sFirst.dw;
	double	fDevSum = 0;
	int nPerms = int(m_arrState.size());
	for (int iPerm = 1; iPerm < nPerms; iPerm++) {	// for each permutation, excluding first
		NUMERAL	s;
		s.dw = m_arrNum[m_arrState[iPerm].iNum].dw;	// compare this state to previous state
		for (int iPlace = 0; iPlace < m_nPlaces; iPlace++) {	// for each place
			if (s.b[iPlace] != sPrev.b[iPlace]) {	// if place transitioned
				if (!arrFirstSpan[iPlace]) {	// if first span length hasn't been set
					arrFirstSpan[iPlace] = arrSpan[iPlace];	// save first span length
				} else {
					fDevSum += CalcDeviance(arrSpan[iPlace]);
				}
				arrSpan[iPlace] = 1;	// reset span length
			} else {	// place didn't transition
				arrSpan[iPlace]++;	// increment span length
			}
		}
		sPrev = s;	// update previous state
	}
	// wrap around from last to first state
	for (int iPlace = 0; iPlace < m_nPlaces; iPlace++) {	// for each place
		if (sFirst.b[iPlace] != sPrev.b[iPlace]) {	// if place transitioned
			fDevSum += CalcDeviance(arrSpan[iPlace]);
		} else {	// place didn't transition
			arrFirstSpan[iPlace] += arrSpan[iPlace];	// compute wrapped span length
		}
		fDevSum += CalcDeviance(arrFirstSpan[iPlace]);
	}
	double	fVar = fDevSum / nPerms;
	double	fStdDev = sqrt(fVar);
	return fStdDev;
}

bool CBalaGray::CalcFromCode(SET_CODE nSetCode, CWinner& seqWinner)
{
	seqWinner.m_nSetCode = nSetCode;
	NUMERAL	arrBase;
	int	nPlaces = GetBases(nSetCode, arrBase);
	return Calc(nPlaces, arrBase.b, seqWinner);
}

void TestCalc()
{
	CBalaGray::SET_CODE nSetCode = 
//
// All cases want PRUNE_IMBALANCE = 3 unless specified otherwise below.
// Pruning greatly reduces runtime, but the results may not be optimal.
// Proven means exited normally with pruning disabled (DO_PRUNING = 0).
// Only ONE of the following set codes may be uncommented at a time.
//
//	0x22	// proven
//	0x23	// proven
//	0x24	// proven
//	0x33	// proven
//	0x25	// proven
//	0x34	// proven
//	0x26	// proven
//	0x35	// proven
//	0x44	// proven
//	0x27	// proven
//	0x36	// proven
//	0x45	// proven
//	0x28	// proven
//	0x37
//	0x46
//	0x55
//	0x29	// proven
//	0x38
//	0x47
//	0x56
//	0x2A	// proven
//	0x39
//	0x48
//	0x57
//	0x66
//	0x222	// proven
//	0x223	// proven
//	0x224	// proven
//	0x233	// proven
//	0x225	// proven
	0x234	// proven
//	0x333
//	0x226	// proven
//	0x235
//	0x244
//	0x334
//	0x227
//	0x236
//	0x245
//	0x335
//	0x344
//	0x228
//	0x237
//	0x246
//	0x255
//	0x336
//	0x345
//	0x444
//	0x2222	// proven
//	0x2223	// proven
//	0x2224
//	0x2233
//	0x2225
//	0x2234
//	0x2333
//	0x2226
//	0x2235	// slow
//	0x2244
//	0x2334	// slow; wants PRUNE_IMBALANCE = 4
//	0x3333	// slow
//
// *** following cases require MORE_PLACES to be non-zero ***
//
//	0x22222
//	0x22223	// wants PRUNE_IMBALANCE = 2
//	0x22224	// wants PRUNE_IMBALANCE = 2
//	0x22233	// wants PRUNE_IMBALANCE = 4
//	0x222222
//
	;	// if a set code isn't uncommented above, compiler error here
	CBalaGray	bg;
	CBalaGray::CWinner	seqWinner;
	bg.CalcFromCode(nSetCode, seqWinner);
	printf("done\npress Enter to continue\n");
	fgetc(stdin);
}

CBalaGray::CWinner::CWinner()
{ 
	m_nSetCode = 0;
	m_nPlaces = 0;
	m_nBaseSum = 0;
	m_nImbalance = 0;
	m_nMaxTrans = 0;
	m_nMaxSpan = 0;
#if OPT_STD_DEV
	m_fStdDev = 0;
#endif
	m_bIsProven = false;
}

std::ofstream& operator<<(std::ofstream& ofs, const CBalaGray::CWinner& winner)
{
	ofs << std::hex << winner.m_nSetCode << std::dec;
	ofs << ' ' << winner.m_nPlaces;
	ofs << ' ' << winner.m_nBaseSum;
	ofs << ' ' << winner.m_nImbalance;
	ofs << ' ' << winner.m_nMaxTrans;
	ofs << ' ' << winner.m_nMaxSpan;
#if OPT_STD_DEV
	ofs << ' ' << winner.m_fStdDev;
#endif
	ofs << ' ' << winner.m_bIsProven;
	int	nNums = static_cast<int>(winner.m_arrNum.size());
	ofs << ' ' << nNums << std::hex;
	for (int i = 0; i < nNums; i++) {
		ofs << ' ' << winner.m_arrNum[i].dw;
	}
	return ofs;
}

std::ifstream& operator>>(std::ifstream& ifs, CBalaGray::CWinner& winner)
{
	ifs >> std::hex >> winner.m_nSetCode >> std::dec;
	ifs >> winner.m_nPlaces;
	ifs >> winner.m_nBaseSum;
	ifs >> winner.m_nImbalance;
	ifs >> winner.m_nMaxTrans;
	ifs >> winner.m_nMaxSpan;
#if OPT_STD_DEV
	ifs >> winner.m_fStdDev;
#endif
	ifs >> winner.m_bIsProven;
	int	nNums;
	ifs >> nNums >> std::hex;
	winner.m_arrNum.resize(nNums);
	for (int i = 0; i < winner.m_arrNum.size(); i++) {
		ifs >> winner.m_arrNum[i].dw;
	}
	return ifs;
}

std::ofstream& operator<<(std::ofstream& ofs, const CBalaGray::CWinnerArray& arrWin)
{
	int	nElemSize = sizeof(CBalaGray::CWinner);
	int	nElems = static_cast<int>(arrWin.size());
	ofs << nElemSize;
	ofs << ' ' << nElems << std::endl;
	for (int iElem = 0; iElem < nElems; iElem++) {
		ofs << arrWin[iElem] << std::endl;
	}
	return ofs;
}

std::ifstream& operator>>(std::ifstream& ifs, CBalaGray::CWinnerArray& arrWin)
{
	int	nElemSize;
	int	nElems;
	ifs >> nElemSize;
	ifs >> nElems;
	if (nElemSize == sizeof(CBalaGray::CWinner)) {	// sanity check
		arrWin.resize(nElems);
		for (int iElem = 0; iElem < nElems; iElem++) {
			ifs >> arrWin[iElem];
		}
	}
	return ifs;
}

void CBalaGray::CWinnerArray::Read(const char *pszPath)
{
	std::ifstream	fIn(pszPath, std::ios_base::binary);
	assert(fIn.good());
	fIn >> *this;
}

void CBalaGray::CWinnerArray::Write(const char *pszPath) const
{
	std::ofstream	fOut(pszPath, std::ios_base::trunc | std::ios_base::binary);
	assert(fOut.good());
	fOut << *this;
}

CBalaGray::CWinnerArray	arrSeq;

void ThreadFunc(CBalaGray *pBG, CBalaGray::SET_CODE nSetCode, CWorkerSync *psync)
{
	CBalaGray::CWinner	seqWinner;
	pBG->CalcFromCode(nSetCode, seqWinner);
	arrSeq.push_back(seqWinner);
	psync->NotifyDone();
}

void CalcWithTimeout(CBalaGray::SET_CODE nSetCode)
{
	int nTimeoutMillis = 30 * 1000;	// default maximum runtime
	char	szCode[16];
	sprintf(szCode, "%X", nSetCode);
	std::string	sOutPath;
	sOutPath += "BalaGray ";
	sOutPath += szCode;
	sOutPath += ".txt";
	CBalaGray	bg(sOutPath.c_str());
	switch (nSetCode) {
	case 0x37:
	case 0x46:
	case 0x234:
	case 0x22222:
		// the above sets benefit from longer runtimes
		nTimeoutMillis = std::max(nTimeoutMillis, 120 * 1000);
		break;
	case 0x2225:
		nTimeoutMillis = std::max(nTimeoutMillis, 180 * 1000);
		break;
	case 0x336:
	case 0x2334:
	case 0x22233:
		bg.SetPruneImbalance(4);
		break;
	case 0x22224:
	case 0x22223:
		bg.SetPruneImbalance(2);
		break;
	}
#if OPT_STD_DEV
	nTimeoutMillis *= 2;	// standard deviation needs longer timeout
#endif
	CWorkerSync	sync;
	std::thread thrWorker(ThreadFunc, &bg, nSetCode, &sync);
	bool	bIsDone = sync.WaitForDone(nTimeoutMillis);
	if (bIsDone) {	// if worker finished normally
		printf("done\n");
	} else {	// timeout waiting for worker to finish
		printf("timeout\n");
	}
	bg.Cancel();	// request worker to exit
    thrWorker.join();
}

CBalaGray::SET_CODE arrSetCode[] = {
#define INTERVAL_SET(s) 0x##s,
#include "IntervalSetsList.h"
};

void MakeHTMLTable(const CBalaGray::CWinnerArray& arrSeq, const char *pszPath)
{
	static const char	arrBoolChar[2] = {'N', 'Y'};
	assert(pszPath != NULL);
	std::ofstream	fOut(pszPath, std::ios_base::trunc);
	if (!fOut.good()) {
		printf("can't create file '%s'\n", pszPath);
		return;
	}
	int	nSeqs = static_cast<int>(arrSeq.size());
	fOut << "<!DOCTYPE html>\n<html>\n<head>\n";
	fOut << "<title>Balanced Gray Interval Sets</title>\n";
	fOut << "<meta name=\"author\" content=\"Chris Korda\">\n"
		"<meta name=\"description\" content=\"Interval sets derived from balanced Gray code.\">\n"
		"<link href=\"../style.css\" rel=stylesheet title=default type=text/css>\n"
		"</head>\n<body style=\"text-size-adjust: none; -webkit-text-size-adjust: none;\">\n"	// need this for mobile, else text size varies
		"<table border=1 cellpadding=2 cellspacing=0>\n"
#if OPT_STD_DEV
		"<tr><th>Name</th><th>Size</th><th>Range</th><th>States</th><th>Imbalance</th><th>MaxSpan</th><th>StdDev</th><th>Proven</th><th>Set</th></tr>\n";
#else
		"<tr><th>Name</th><th>Size</th><th>Range</th><th>States</th><th>Imbalance</th><th>MaxSpan</th><th>Proven</th><th>Set</th></tr>\n";
#endif
	for (int iSeq = 0; iSeq < nSeqs; iSeq++) {
		const CBalaGray::CWinner&	seq = arrSeq[iSeq];
		int	nNumerals = static_cast<int>(seq.m_arrNum.size());
		fOut << "<tr><td>" << std::hex << std::uppercase << seq.m_nSetCode << std::dec
			<< "</td><td>" << seq.m_nPlaces
			<< "</td><td>" << seq.m_nBaseSum
			<< "</td><td>" << nNumerals
			<< "</td><td>" << seq.m_nImbalance
			<< "</td><td>" << seq.m_nMaxSpan
#if OPT_STD_DEV
			<< "</td><td>" << std::setprecision(3) << seq.m_fStdDev
#endif
			<< "</td><td>" << arrBoolChar[seq.m_bIsProven] << "</td><td>\n";
		for (int iPlace = 0; iPlace < seq.m_nPlaces; iPlace++) {
			if (iPlace)
				fOut << "\n<br>";
			for (int iNum = 0; iNum < nNumerals; iNum++) {
				if (iNum)
					fOut << "&nbsp;";
				fOut << int(seq.m_arrNum[iNum].b[iPlace]);
			}
		}
		fOut << "\n</td></tr>\n";
	}
	fOut << "</table>\n</body>\n</html>\n";
}

void MakeCSVTable(const CBalaGray::CWinnerArray& arrSeq, const char *pszPath)
{
	assert(pszPath != NULL);
	std::ofstream	fOut(pszPath, std::ios_base::trunc);
	if (!fOut.good()) {
		printf("can't create file '%s'\n", pszPath);
		return;
	}
	int	nSeqs = static_cast<int>(arrSeq.size());
#if OPT_STD_DEV
	fOut << "Name,Digit,Digits,Range,States,Imbalance,MaxSpan,StdDev,Proven\n";
#else
	fOut << "Name,Digit,Digits,Range,States,Imbalance,MaxSpan,Proven\n";
#endif
	for (int iSeq = 0; iSeq < nSeqs; iSeq++) {
		const CBalaGray::CWinner&	seq = arrSeq[iSeq];
		int	nNumerals = static_cast<int>(seq.m_arrNum.size());
		for (int iPlace = 0; iPlace < seq.m_nPlaces; iPlace++) {
			fOut << '[' << std::hex << std::uppercase << seq.m_nSetCode << std::dec << ']'
				<< ',' << iPlace
				<< ',' << seq.m_nPlaces
				<< ',' << seq.m_nBaseSum
				<< ',' << nNumerals
				<< ',' << seq.m_nImbalance
				<< ',' << seq.m_nMaxSpan
#if OPT_STD_DEV
				<< ',' << seq.m_fStdDev
#endif
				<< ',' << seq.m_bIsProven;
			for (int iNum = 0; iNum < nNumerals; iNum++) {
				fOut << ',' << int(seq.m_arrNum[iNum].b[iPlace]);
			}
			fOut << '\n';
		}
	}
}

void MakePolymeterImportTracksCSV(const CBalaGray::CWinnerArray& arrSeq, const char *pszPath)
{
	assert(pszPath != NULL);
	std::ofstream	fOut(pszPath, std::ios_base::trunc);
	if (!fOut.good()) {
		printf("can't create file '%s'\n", pszPath);
		return;
	}
	int	nSeqs = static_cast<int>(arrSeq.size());
	fOut << "Name,Type,Steps\n";
	for (int iSeq = 0; iSeq < nSeqs; iSeq++) {
		const CBalaGray::CWinner&	seq = arrSeq[iSeq];
		int	nNumerals = static_cast<int>(seq.m_arrNum.size());
		for (int iPlace = 0; iPlace < seq.m_nPlaces; iPlace++) {
			fOut << "\"BG [" << std::hex << std::uppercase << seq.m_nSetCode << std::dec << "] " << iPlace + 1 << "\",7,\"";
			for (int iNum = 0; iNum < nNumerals; iNum++) {
				if (iNum)
					fOut << ',';
				fOut << int(seq.m_arrNum[iNum].b[iPlace]) + 64;	// convert to signed step value
			}
			fOut << "\"\n";
		}
	}
}

void CalcAllSets()
{
	bool	bReadSavedData = false;	// set true to read back previously saved data
	const char *pszDataPath = "BalaGrayTable.dat";
	if (bReadSavedData) {
		arrSeq.Read(pszDataPath);
	} else {	// not reading saved data, so calculate interval sets
		int	nSets = _countof(arrSetCode);
		for (int iSet = 0; iSet < nSets; iSet++) {
			CalcWithTimeout(arrSetCode[iSet]);
		}
		arrSeq.Write(pszDataPath);	// save data
	}
	MakeHTMLTable(arrSeq, "BalaGraySetsTable.htm");
	MakeCSVTable(arrSeq, "BalaGraySetsTable.csv");
	MakePolymeterImportTracksCSV(arrSeq, "BalaGraySetsAsPolymeterTracks.csv");
}

int main(int argc, const char* argv[])
{
//	TestCalc();
//	CalcWithTimeout(0x3333);
	CalcAllSets();
	return 0;
}
