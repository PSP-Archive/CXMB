/*
 *	utils.c is part of cxmb
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
 *	Date Created:	2008-07-01
 */

//patpat mod for 3.71 - 6.37 <2011-02-19>
//neur0n mod for 6.39
//frostegater mod for 6.60
//LMAN mod for 6.61
//Yoti mod for INFINITY

#include <pspkernel.h>
#include <pspsdk.h>
#include <pspsysmem_kernel.h>
#include <psploadcore.h>
#include <systemctrl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "log.h"
#include "syspatch.h"
#include "utils.h"

const char * endwithistr( const char * str1, const char * str2 )
{
	if ( !*str2 || strlen( str2 ) > strlen( str1 ) )
	{
		return NULL;
	}
	const char * ostr1, * ostr2;
	for ( ostr1 = str1 + strlen( str1 ) - 1, ostr2 = str2 + strlen( str2 ) - 1; ostr1 >= str1 && ostr2 >= str2; ostr1 -- , ostr2 --)
	{
		if ( tolower( *ostr1 ) != tolower( *ostr2 ) )
		{
			break;
		}
	}
	if ( ostr2 == ( str2 - 1 ) )
	{
		return ostr1 + 1;
	}
	return NULL;
}

int cmpistr( const char * str1, const char * str2 )
{
	for( ; *str1 && *str2; str1 ++, str2 ++ )
	{
		if ( tolower( *str1 ) != tolower( *str2 ) )
			return 1;
	}
	if ( *str1 != *str2 )
		return 1;
	return 0;
}

int truncpath( char * str1, const char * str2 )
{
	char * ostr = strstr( str1, str2 );
	if ( ostr )
	{
		ostr[strlen( str2 )] = 0;
		return 1;
	}
	return -1;
}

int readLine( int fd, char * buf, int max_len )
{
 int i = 0, bytes = 0;
  sceIoLseek( fd, 0, PSP_SEEK_SET );
	while( i < max_len && ( bytes = sceIoRead( fd, buf + i, 1 ) ) == 1 )
	{
		if ( buf[i] == -1 || buf[i] == '\n' )
			break;
		i ++;
	}
	buf[i] = 0;
	if ( bytes != 1 && i == 0 )
		return -1;
	return i;
}

PspIoDrv * findDriver( char * drvname )
{
	unsigned int * ( * getDevice )( char * ) = ( void * )getFindDriverAddr();
	if ( !getDevice )
		return NULL;
	unsigned int * u;
	u = getDevice( drvname );
	if ( !u )
		return NULL;
	log( "%s found!\nu0: %08x\nu1: %08x\nu2: %08x\nu3: %08x\n",
		drvname, u[0], u[1], u[2], u[3] );
	return ( PspIoDrv * )u[1];
}

StartModuleHandler ( * setStartModuleHandler )( StartModuleHandler );
int ( * rebootPsp )( void );

void initUtils( void )
{
	setStartModuleHandler = (void*)sctrlHENFindFunction( "SystemControl", "SystemCtrlForKernel", 0x1C90BECB );
	rebootPsp = (void*)sctrlHENFindFunction( "scePower_Service", "scePower", 0x0442D852 );
}
