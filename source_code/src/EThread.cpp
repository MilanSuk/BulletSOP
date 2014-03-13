/**
BulletSOP v2.0
Copyright (c)2010-2013 Suk Milan. All Rights Reserved.
This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the use of this software.
This file may not be redistributed in whole or significant part.
This notice may not be removed or altered from any source distribution.
*/
#include "stdafx.h"
#include "EThread.h"


#ifdef WIN32
	#include <windows.h>
#else
	#include <pthread.h>
#endif


#ifdef WIN32	
	DWORD WINAPI entryRun(LPVOID pthis)
#else			
	void* entryRun(void* pthis)
#endif
{
	reinterpret_cast<EThread*>(pthis)->run();
	return 0;
}



EThread::EThread()
{
	m_game_over = true;
	m_thread = 0;
}
EThread::~EThread()
{
	hard_end();
}


int EThread::start()
{
	hard_end();

	m_game_over = false;
	#ifdef WIN32
		m_thread = CreateThread(NULL, 0, entryRun, reinterpret_cast<void*>(this), 0, NULL); 
		return 1;
	#else
		//return pthread_create(&m_thread, NULL, EThread::entryRun, this);
		return pthread_create(&m_thread, NULL, entryRun, reinterpret_cast<void*>(this));
	#endif
}

void EThread::wait_end()
{
	if(m_thread)
	{
	#ifdef WIN32
		WaitForMultipleObjects(1, &m_thread, true, INFINITE);
	#else
		pthread_join(m_thread, NULL);
	#endif
	}

	m_game_over = true;
}

void EThread::hard_end()
{
	m_game_over = true;
	wait_end();
}


bool EThread::isRunning()
{
	return !m_game_over;
}
