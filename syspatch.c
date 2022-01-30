/*
 *	syspatch.c is part of cxmb
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
#include <psputilsforkernel.h>
#include <pspsysmem_kernel.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <systemctrl.h>
#include "utils.h"
#include "syspatch.h"

int fw_version = FW_371;

int ( * getDevkitVersion )( void );

int initPatches( void )
{
	getDevkitVersion = ( void * )sctrlHENFindFunction( "sceSystemMemoryManager", "SysMemUserForUser", 0x3FC9AE6A );
	fw_version = getDevkitVersion();

	return fw_version;
}

unsigned int getMagic( void )
{
	unsigned int magic = 0;
	if ( fw_version == FW_371 )
		magic = 0xDEAD0371;
	else if ( fw_version == FW_380 )
		magic = 0xDEAD0380;
	else if ( fw_version == FW_390 )
		magic = 0xDEAD0390;
	else if ( fw_version == FW_401 )
		magic = 0xDEAD0401;
	else if ( fw_version == FW_500 )
		magic = 0xDEAD0500;
	else if (  fw_version == FW_502 
			|| fw_version == FW_503 )
		magic = 0xDEAD0502;
	else if ( fw_version == FW_550 )
		magic = 0xDEAD0550;
	else if ( fw_version == FW_620 )
		magic = 0xDEAD0620;
	else if ( fw_version == FW_631 )
		magic = 0xDEAD0631;
	else if ( fw_version == FW_635 )
		magic = 0xDEAD0635;
	else if ( fw_version == FW_637 )
		magic = 0xDEAD0637;
	else if ( fw_version == FW_638 )
		magic = 0xDEAD0638;
	else if ( fw_version == FW_639 )
		magic = 0xDEAD0639;
	else if ( fw_version == FW_660 )
		magic = 0xDEAD0660;
	else if ( fw_version == FW_661 )
		magic = 0xDEAD0661;
	return magic;
}

unsigned int getFindDriverAddr( void )
{
	tSceModule * pMod = ( tSceModule * )sceKernelFindModuleByName( "sceIOFileManager" );
	unsigned int addr = 0;
	if ( !pMod )
		return 0;
	if ( fw_version == FW_371 )
		addr = pMod->text_addr + 0x00002844;
	else if (  fw_version == FW_380
			|| fw_version == FW_390 )
		addr = pMod->text_addr + 0x00002808;
	else if ( fw_version == FW_401 )
		addr = pMod->text_addr + 0x000027EC;
	else if (  fw_version == FW_500
			|| fw_version == FW_502
			|| fw_version == FW_503
			|| fw_version == FW_550 )
		addr = pMod->text_addr + 0x00002838;
	else if (  fw_version == FW_620
			|| fw_version == FW_631 )
		addr = pMod->text_addr + 0x00002A38;
	else if (  fw_version == FW_635
			|| fw_version == FW_637
			|| fw_version == FW_638
			|| fw_version == FW_639 )
		addr = pMod->text_addr + 0x00002A44;
	else if ( fw_version == FW_660 
			|| fw_version == FW_661 )
		addr = pMod->text_addr + 0x00002A4C;
	
	return addr;
}

unsigned int getSysconfOffset( void )
{
	unsigned int offset = 0;
	if ( fw_version == FW_371 )
		offset = 0x00004780;
	else if (  fw_version == FW_380
			|| fw_version == FW_390 )
		offset = 0x0000476C;
	else if ( fw_version == FW_401 )
		offset = 0x00007708;
	else if (  fw_version == FW_500
			|| fw_version == FW_502
			|| fw_version == FW_503 )
		offset = 0x00026050;
	else if ( fw_version == FW_550 )
		offset = 0x000257B8;
	else if ( fw_version == FW_620 )
		offset = 0x0002BDB4;
	else if (  fw_version == FW_631
			|| fw_version == FW_635
			|| fw_version == FW_637
			|| fw_version == FW_638
			|| fw_version == FW_639 )
		offset = 0x0002C724;
	else if (  fw_version == FW_660 
			|| fw_version == FW_661 )
		offset = 0x0002CB84;// valid
		
	return offset;
}
