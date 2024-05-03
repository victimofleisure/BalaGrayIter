// Copyleft 2023 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      28jan23	initial version

*/

#pragma once

#include <thread>
#include <chrono>
#include <condition_variable>

class CWorkerSync {
public:
	CWorkerSync() { m_bIsDone = false; }
	bool	WaitForDone(unsigned int nTimeoutMillis);
	void	NotifyDone();

protected:
	std::condition_variable  m_cvDone;
	std::mutex m_mtxDone;
	bool	m_bIsDone;
};

inline bool CWorkerSync::WaitForDone(unsigned int nTimeoutMillis)
{
	// main thread calls this method to wait for worker thread to finish its work
	std::unique_lock<std::mutex> lk(m_mtxDone);	// required
	// must pass this pointer to lambda function so it can access our member vars
	return m_cvDone.wait_for(lk, std::chrono::milliseconds(nTimeoutMillis), [this]{ return m_bIsDone; });
}

inline void CWorkerSync::NotifyDone()
{
	// worker thread calls this method to notify main thread that work is finished
    std::lock_guard<std::mutex> lk(m_mtxDone);	// required
    m_bIsDone = true;	// set done flag
    m_cvDone.notify_all();  // signal main thread
}
