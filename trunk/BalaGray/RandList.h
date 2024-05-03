// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      02jun05	initial version
        01      01jul12	fix Rand to avoid possible overrun
        02      06jan13	add cast to GetSize for x64
		03		07jun21	rename rounding functions
		04		03feb23	use vector

		random sequence without duplicates
 
*/

#pragma once

class CRandList {
public:
	CRandList();
	CRandList(int nSize);
	static	int		Rand(int nVals);
	void	Init(int nSize);
	int		GetNext();
	int		GetSize() const;
	int		GetAvail() const;

protected:
	std::vector<int>	m_arrList;	// array of randomly generated elements
	int		m_nAvail;	// number of elements that haven't been used
};

inline int CRandList::GetSize() const
{
	return(static_cast<int>(m_arrList.size()));	// cast to 32-bit
}

inline int CRandList::GetAvail() const
{
	return(m_nAvail);
}

inline CRandList::CRandList()
{
	m_nAvail = 0;
}

inline CRandList::CRandList(int nSize)
{
	Init(nSize);
}

inline void CRandList::Init(int nSize)
{
	m_arrList.resize(nSize);
	for (int iElem = 0; iElem < nSize; iElem++)
		m_arrList[iElem] = iElem;
	m_nAvail = 0;
}

inline int CRandList::Rand(int nVals)
{
	if (nVals <= 0)
		return(-1);
	int	i = int(rand() / double(RAND_MAX) * nVals);
	return(std::min(i, nVals - 1));
}

inline int CRandList::GetNext()
{
	if (!m_nAvail)
		m_nAvail = GetSize();
	assert(m_nAvail > 0);
	int	iNext = Rand(m_nAvail);
	m_nAvail--;
	int	nTmp = m_arrList[iNext];
	m_arrList[iNext] = m_arrList[m_nAvail];
	m_arrList[m_nAvail] = nTmp;
	return(nTmp);
}
