/*
 *	ctf.c is part of cxmb
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

#include "log.h"
#include "utils.h"
#include "ctf.h"

#if _CXMB_LITE == 0
#define CXMB_MKCTF_BUF_SIZE	0x003C0000
#define CXMB_MAX_FILE_SIZE	0x00200000

static const char * diff_files[] = {
	"paf.prx",
	"common_gui.prx",
	"vshmain.prx"
};
static int diff_count = 3;

static const char * endwithistrs( const char * str, const char ** strs, int count )
{
	int i;
	const char * ret;
	for ( i = 0; i < count; i ++ )
	{
		ret = endwithistr( str, strs[i] );
		if ( ret )
			return strs[i];
	}
	return NULL;
}

static const char * cmpistrs( const char * str, const char ** strs, int count )
{
	int i, ret;
	for ( i = 0; i < count; i ++ )
	{
		ret = cmpistr( str, strs[i] );
		if ( !ret )
			return strs[i];
	}
	return NULL;
}

int readPrx( const char * file, int heap_id, u8 ** outbuf )
{
	log( "read %s to memory!\n", file );
	int fd = sceIoOpen( file, PSP_O_RDONLY, 0644 );
	if ( fd < 0 )
	{
		log( "failed in opening %s\n", file );
		return -1;
	}
	u8 * inbuf = ( u8 * )sceKernelAllocHeapMemory( heap_id, CXMB_MKCTF_BUF_SIZE / 2 );
	int size = sceIoRead( fd, inbuf, CXMB_MKCTF_BUF_SIZE / 2 );
	if ( inbuf[0x150] == 0x1F && inbuf[0x151] == 0x8B )
	{
		log( "uncompress gzip file!\n" );
		*outbuf = ( u8 * )sceKernelAllocHeapMemory( heap_id, CXMB_MKCTF_BUF_SIZE / 2 );
		int size = sceKernelGzipDecompress( *outbuf, CXMB_MKCTF_BUF_SIZE / 2, &inbuf[0x150], 0 );
		log( "uncompressed size %08x!\n", size );
		sceKernelFreeHeapMemory( heap_id, inbuf );
		if ( size > 0 && size < CXMB_MAX_FILE_SIZE )
		{
			return size;
		}
		else
		{
			sceKernelFreeHeapMemory( heap_id, outbuf );
			return -1;
		}
	}
	else
	{
		*outbuf = inbuf;
		return size;
	}
	return -1;
}

int makeDiff( const char * file, const char * ori, int heap_id, int ctf )
{
	log( "detect %s, start to make diff!\n", file );
	u8 * buf = NULL, * buf_ori = NULL;
	int sub = 0;
	int fp = sceIoOpen( file, PSP_O_RDONLY, 0644 );
	if ( fp < 0 )
	{
		log( "failed in opening %s\n", file );
		return -1;
	}
	sceIoLseek( fp, 56, PSP_SEEK_SET );
	sceIoRead( fp, &sub, 4 );
	sceIoClose( fp );
	int bytes = readPrx( file, heap_id, &buf );
	if ( bytes < 0 )
	{
		log( "failed in read %s!\n", file );
		return -1;
	}
	if ( readPrx( ori, heap_id, &buf_ori ) != bytes )
	{
		log( "failed in read %s!\n", ori );
		sceKernelFreeHeapMemory( heap_id, buf );
		sceKernelFreeHeapMemory( heap_id, buf_ori );
		return -1;
	}
	int diff_count = 0;
	unsigned int offset = 0, rec_attr[2];
	int rec = 0;
	memset( rec_attr, 0, 8 );
	while( offset < bytes )
	{
		if ( buf[offset] != buf_ori[offset] )
		{
			if ( !rec )
			{
				rec_attr[0] = offset - sub;
				rec_attr[1] = 0;
				rec = 1;
			}
			rec_attr[1] ++;
		}
		else
		{
			if ( rec )
			{
				log( "diff_start: %08x\nsize: %08x\n", rec_attr[0], rec_attr[1] );
				sceIoWrite( ctf, rec_attr, 8 );
				sceIoWrite( ctf, &buf[rec_attr[0] + sub], rec_attr[1] );
				diff_count ++;
				rec = 0;
			}
		}
		offset ++;
	}
	sceKernelFreeHeapMemory( heap_id, buf_ori );
	sceKernelFreeHeapMemory( heap_id, buf );
	return diff_count;
}

int makeCxmbThemeFile( unsigned int cxmb_magic, const char * cxmb_theme_file, const char * cxmb_drive )
{
	const char * folders_name[] = {
		"/data/cert",
		"/dic",
		"/font",
		"/kd",
		"/kd/resource",
		"/vsh/etc",
		"/vsh/module",
		"/vsh/resource"
	};
	int folders_count = 8;
	const char * support_exts[] = {
		".prx",
		".rco",
		".bmp",
		".pmf",
		".res",
		".pgf",
		".bwfon",
		".rsc",
		".dat",
		".img",
		".bin",
		".cet",
		".dic"
	};
	int exts_count = 13;
	int dfd, heap_id, fd, i, bytes, file_count = 0;
	unsigned int ptf_h[5];
	char path[128], file[128], preview[64], tmp[64];
	u8 * buf;
	// dectect if theme file in conf exist
	int ctf = sceIoOpen( cxmb_theme_file, PSP_O_RDONLY, 0644 );
	if ( ctf >= 0 )
	{
		log( "theme file exist!\n" );
		sceIoClose( ctf );
		return 0;
	}
	
	char cxmb_support_folder[18];
	sprintf( cxmb_support_folder, "%s:/cxmb/support", cxmb_drive );
  sprintf( tmp, "%s:/cxmb", cxmb_drive );
	dfd = sceIoDopen( (const char*)tmp );
	log("cxmb folder:%s\n", tmp);
	if ( dfd < 0 )
	{
		log( "no cxmb folder found!\n" );
		return 0;
	}
	sceIoDclose( dfd );

	sprintf( preview, "%s:/cxmb%s", cxmb_drive, &cxmb_theme_file[14] );
	preview[strlen( preview ) - 3] = 'p';
	log( "preview: %s\n", preview );
	fd = sceIoOpen( preview, PSP_O_RDONLY, 0644 );
	if ( fd < 0 )
	{
		log( "no preview ptf file found!\n" );
		return 0;
	}
	sceIoLseek( fd, 0x100, PSP_SEEK_SET );
	sceIoRead( fd, ptf_h, 20 );

	// create CXMB_MKCTF_BUF_SIZE + 32kb heap
	heap_id = sceKernelCreateHeap( 2, CXMB_MKCTF_BUF_SIZE + 1024 * 32 , 1, "cxmb_tmp_heap");
	if ( heap_id < 0 )
	{
		log( "failed in create heap in making cxmb theme file!\n" );
		return -1;
	}

	CtfHeader * ch = ( CtfHeader * )sceKernelAllocHeapMemory( heap_id, sizeof( CtfHeader ) * 64 );
	memset( ch, 0, sizeof( CtfHeader ) * 64 );
	SceIoDirent * ent = ( SceIoDirent * )sceKernelAllocHeapMemory( heap_id, sizeof( SceIoDirent ) );
	memset( ent, 0, sizeof( SceIoDirent ) );
  
  sprintf( tmp, "%s:/PSP/THEME", cxmb_drive );
	sceIoMkdir( (const char*)tmp, 0777 );
	ctf = sceIoOpen( cxmb_theme_file, PSP_O_RDWR | PSP_O_CREAT | PSP_O_TRUNC, 0777 );
	if ( ctf < 0 )
	{
		log( "failed in opening %s\n", cxmb_theme_file );
		sceKernelFreeHeapMemory( heap_id, ent );
		sceKernelFreeHeapMemory( heap_id, ch );
		sceKernelDeleteHeap( heap_id );
		return -1;
	}
	else
	{
		if ( ptf_h[2] == 0 )
			ptf_h[2] = sceIoLseek( fd, 0, PSP_SEEK_END );
		log( "ptf sections size %08x\n", ptf_h[2] );
		buf = sceKernelAllocHeapMemory( heap_id, ptf_h[2] );
		if ( buf )
		{
			sceIoLseek( fd, 0, PSP_SEEK_SET );
			sceIoRead( fd, buf, ptf_h[2] );
			sceIoWrite( ctf, buf, ptf_h[2] );
			sceIoClose( fd );
			sceKernelFreeHeapMemory( heap_id, buf );

			sceIoLseek( ctf, 0x10, PSP_SEEK_SET );
			sceIoWrite( ctf, &cxmb_magic, 4 );
			sceIoLseek( ctf, 0x1C, PSP_SEEK_SET );
			sceIoWrite( ctf, &ptf_h[2], 4 );

			memset( &ptf_h[2], 0, 12 );
			sceIoLseek( ctf, 0x100, PSP_SEEK_SET );
			sceIoWrite( ctf, ptf_h, 20 );
			sceIoLseek( ctf, 0, PSP_SEEK_END );

			for ( i = 0; i < folders_count; i ++ )
			{
				sprintf( path, "%s:/cxmb%s", cxmb_drive, folders_name[i] );
				dfd = sceIoDopen( path );
				if ( dfd < 0 )
				{
					log( "folder %s not found!\n", path );
					continue;
				}
				log( "parsing %s\n", path );
				while ( sceIoDread( dfd, ent ) > 0 )
				{
					log( "found %s\n", ent->d_name );
					if ( ( ent->d_stat.st_attr & FIO_SO_IFDIR ) || ent->d_name[0] == '.' )
					{
						log( "ignore %s\n", ent->d_name );
						continue;
					}
					if ( endwithistrs( ent->d_name, support_exts, exts_count ) )
					{
						sprintf( file, "%s/%s", path, ent->d_name );
						sprintf( ch[file_count].name, "%s/%s", folders_name[i], ent->d_name );
						ch[file_count].start = sceIoLseek( ctf, 0, PSP_SEEK_CUR );
						ch[file_count].size = 0;
						if ( cmpistrs( ent->d_name, diff_files, diff_count ) )
						{
							char ori_file[128];
							sprintf( ori_file, "%s/%s", cxmb_support_folder, ent->d_name );
							ch[file_count].size = makeDiff( file, ori_file, heap_id, ctf );
						}
						else
						{
							log( "dealing with %s\n", ent->d_name );
							fd = sceIoOpen( file, PSP_O_RDONLY, 0644 );
							if ( fd < 0 )
							{
								log( "failed in opening %s\n", file );
								continue;
							}
							buf = ( u8 * )sceKernelAllocHeapMemory( heap_id, CXMB_MKCTF_BUF_SIZE );
							bytes = sceIoRead( fd, buf, CXMB_MKCTF_BUF_SIZE );
							while( bytes > 0 )
							{
								ch[file_count].size += sceIoWrite( ctf, buf, bytes );
								bytes = sceIoRead( fd, buf, CXMB_MKCTF_BUF_SIZE );
							}
							sceKernelFreeHeapMemory( heap_id, buf );
							sceIoClose( fd );
						}
						if ( ch[file_count].size > 0 && ch[file_count].size < CXMB_MAX_FILE_SIZE )
						{
							log( "start: %08x size: %08x\n", ch[file_count].start, ch[file_count].size );
							file_count ++;
						}
					}
					else
					{
						log( "ignore %s\n", ent->d_name );
					}
				}
				sceIoDclose( dfd );
			}
		}
		else
		{
			log( "failed in allocating %08x heap\n", ptf_h[2] );
		}
	}

	log( "file_count: %d\n", file_count );
	if ( file_count > 0 )
	{
		u8 sha1[20];
		sceKernelUtilsSha1Digest( ( u8 * )ch, sizeof( CtfHeader ) * file_count, sha1 );
		sceIoWrite( ctf, ch, sizeof( CtfHeader ) * file_count );
		sceIoLseek( ctf, 0x14, PSP_SEEK_SET );
		sceIoWrite( ctf, &sha1[0], 4 );
		sceIoWrite( ctf, &file_count, 4 );
		sceIoClose( ctf );
	}
	else
	{
		sceIoClose( ctf );
		sceIoRemove( cxmb_theme_file );
	}
	sceKernelFreeHeapMemory( heap_id, ent );
	sceKernelFreeHeapMemory( heap_id, ch );
	sceKernelDeleteHeap( heap_id );
	return 0;
}
#endif
