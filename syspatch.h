/*
 *	syspatch.h is part of cxmb
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

extern int fw_version;

extern int initPatches( void );

extern unsigned int getMagic( void );

extern unsigned int getFindDriverAddr( void );

extern unsigned int getSysconfOffset( void );
