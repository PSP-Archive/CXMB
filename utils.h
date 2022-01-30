/*
 *	utils.h is part of cxmb
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

#pragma once

#include <pspkernel.h>
#include <psploadexec_kernel.h>

enum PspFwVersion
{
	FW_371 = 0x03070110,
	FW_380 = 0x03080010,
	FW_390 = 0x03090010,
	FW_401 = 0x04000110,
	FW_500 = 0x05000010,
	FW_502 = 0x05000210,
	FW_503 = 0x05000310,
	FW_550 = 0x05050010,
	FW_620 = 0x06020010,
	FW_631 = 0x06030110,
	FW_635 = 0x06030510,
	FW_637 = 0x06030710,
	FW_638 = 0x06030810,
	FW_639 = 0x06030910,  
	FW_660 = 0x06060010,
	FW_661 = 0x06060110,
};

typedef struct tSceModule {
	struct tSceModule	*next;
	unsigned short		attribute;
	unsigned char		version[2];
	char				modname[27];
	char				terminal;
	unsigned int		unknown1;
	unsigned int		unknown2;
	int					modid;
	unsigned int		unknown3[4];
	void *				ent_top;
	unsigned int		ent_size;
	void *				stub_top;
	unsigned int		stub_size;
	unsigned int		unknown4[5];
	unsigned int		entry_addr;
	unsigned int		gp_value;
	unsigned int		text_addr;
	unsigned int		text_size;
	unsigned int		data_size;
	unsigned int		bss_size;
	unsigned int		nsegment;
	unsigned int		segmentaddr[4];
	unsigned int		segmentsize[4];
} tSceModule;

typedef int ( * StartModuleHandler )( tSceModule * );

extern StartModuleHandler ( * setStartModuleHandler )( StartModuleHandler );

extern int ( * rebootPsp )( void );

extern const char * endwithistr( const char * str1, const char * str2 );

extern int cmpistr( const char * str1, const char * str2 );

extern int truncpath( char * str1, const char * str2 );

extern int readLine( int fd, char * buf, int max_len );

extern PspIoDrv * findDriver( char * drvname );

extern void initUtils( void );
