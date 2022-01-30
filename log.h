/*
 *	log.h is part of cxmb
 *	Copyright (C) 2008  Poison
 *
 *	This program is free software: you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation, either version 3 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 *	Description:	
 *	Author:			Poison <hbpoison@gmail.com>
 *	Date Created:	2008-04-04
 */

//patpat mod for 3.71 - 6.37 <2011-02-19>
//neur0n mod for 6.39
//frostegater mod for 6.60
//LMAN mod for 6.61
//Yoti mod for INFINITY

#pragma once

#include <pspkernel.h>

#if _DEBUG >= 1
#define log(...)	while( log_running )\
						sceKernelDelayThread( 50000 );\
					sprintf( lstr, __VA_ARGS__ );\
					printlog( lstr )
#else
#define log(...)	;
#endif

#if _DEBUG >= 1
extern int log_running;

extern char lstr[128];

extern int printlog( const char * str );

extern int printFileArg( PspIoDrvFileArg * arg );

extern void dumpRange( const char * name, void * start, int len );

extern void dumpKmem();

extern void dumpUmem();

extern void dumpThreadList();

extern void dumpModuleList();

extern void dumpMemPartitionInfo();
#endif
