/**
BulletSOP v2.0
Copyright (c)2010-2013 Suk Milan. All Rights Reserved.
This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the use of this software.
This file may not be redistributed in whole or significant part.
This notice may not be removed or altered from any source distribution.
*/
#pragma once


class EThread {
public:
	EThread();
	virtual ~EThread();

public:
	int start();
	void wait_end();
	void hard_end();
	bool isRunning();

	virtual void run() = 0;

protected:
	bool m_game_over;

private:
	#ifdef WIN32
		void* m_thread;
	#else
		pthread_t m_thread;
	#endif
};
