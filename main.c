/*
 *	main.c is part of cxmb
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
//ErikPshat mod for 3.71 - 6.61 CLASSIC & 6.61 INFINITY
 
#include <pspkernel.h>
#include <pspsdk.h>
#include <psputilsforkernel.h>
#include <pspsysmem_kernel.h>
#include <psploadcore.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <systemctrl.h>

#include "log.h"
#include "ctf.h"
#include "syspatch.h"
#include "utils.h"

PSP_MODULE_INFO("cxmb", 0x1000, 1, 2);
PSP_MAIN_THREAD_ATTR(0);

#define CXMB_DEFAULT_THEME	"/PSP/THEME/Default.ctf"

typedef struct CtfHandler {
	void * arg;
	int num;
	int offset;
} CtfHandler;

static unsigned int cxmb_magic;
static unsigned int ctf_sig = 0;
static unsigned int header_size = 0;
static unsigned int handler_count = 0;
static CtfHeader * ctf_header = NULL;
static CtfHandler * ctf_handler = NULL;
static int mem_id = -1, sema = -1;
static PspIoDrv * lflash_drv;
static PspIoDrv * fatms_drv;
static PspIoDrv * fatef_drv;
static PspIoDrvArg * ms_drv = NULL;
static char selected_theme_file[64];
static void * t_record = NULL;
static char cxmb_theme_file[64]; //"ms0:/PSP/THEME/Default.ctf"
static char theme_file[64]; //"/PSP/THEME/Default.ctf"
static char cxmb_conf_file[64];	//"ms0:/seplugins/cxmb/conf.txt"
static char cxmb_drive[3];
static char ctf_drive[4];
static int isGO;

StartModuleHandler previous = NULL;

static int inCtf( const char * file )
{
	if ( !ctf_header )
		return -1;
	int i;
	for ( i = 0; i < header_size; i ++ )
	{
		if ( !cmpistr( file, ctf_header[i].name ) )
			return i;
	}
	return -1;
}

static int isRedirected( PspIoDrvFileArg * arg )
{
	if ( !ctf_handler )
		return -1;
	int i;
	for( i = 0; i < handler_count; i ++ )
	{
		if( arg->arg == ctf_handler[i].arg )
			return i;
	}
	return -1;
}

void uninstall_cxmb()
{
	//free the cxmb heap
	sceKernelFreeHeapMemory( mem_id, ctf_handler );
	sceKernelFreeHeapMemory( mem_id, ctf_header );
	sceKernelDeleteHeap( mem_id );
	ctf_handler = NULL;
	ctf_header = NULL;
}

int ( * msIoOpen )( PspIoDrvFileArg * arg, char * file, int flags, SceMode mode );
int ( * msIoGetstat )( PspIoDrvFileArg * arg, const char * file, SceIoStat * stat );

int msIoOpen_new( PspIoDrvFileArg * arg, char * file, int flags, SceMode mode )
{
	ms_drv = arg->drv;
	return msIoOpen( arg, file, flags, mode );
}

int msIoGetstat_new( PspIoDrvFileArg * arg, const char * file, SceIoStat * stat )
{
  log( "patpat:msIoGetstat_new %s.%d.\n",arg->drv->drv->name, arg->fs_num );
	int ret = 0;
  unsigned int magic;
	if ( endwithistr( file, ".ctf" ) )
	{
		int size = 0;
		if(strcmp(arg->drv->drv->name, "fatms") == 0)
		{
      sprintf( selected_theme_file, "ms0:%s", file );
      strcpy( ctf_drive, "ms0" );
    }
    else
    {
      sprintf( selected_theme_file, "ef0:%s", file );
      strcpy( ctf_drive, "ef0" );
    }
		int fd = sceIoOpen( selected_theme_file, PSP_O_RDONLY, 0644 );
		if ( fd >= 0 )
		{
      sceIoLseek( fd, 0x10, PSP_SEEK_SET );
      sceIoRead( fd, &magic, 4 );
      if ( magic != cxmb_magic )
      {
        int fw = initPatches();
        if ( ( (( fw != FW_500 && fw != FW_502 && fw != FW_503 ) || ( magic != 0xDEAD0500 && magic != 0xDEAD0502 && magic != 0xDEAD0503 )) 
			&& (( fw != FW_635 && fw != FW_637 && fw != FW_638 && fw != FW_639 ) || ( magic != 0xDEAD0635 && magic != 0xDEAD0637 && magic != 0xDEAD0638 && magic != 0xDEAD0639 )) 
			&& (( fw != FW_660 && fw != FW_661 ) || ( magic != 0xDEAD0660 && magic != 0xDEAD0661 )) ) 
			&& !endwithistr( file, "random.ctf" ) ) 
        {
          log( "%s version not match!\n", selected_theme_file );
          ret = -1;
        }
        else
        {
          ret = msIoGetstat( arg, file, stat );
        }
      }
			sceIoLseek( fd, 0x1C, PSP_SEEK_SET );
			sceIoRead( fd, &size, 4 );
			sceIoClose( fd );
		}
		log( "getstat %s size: %08x version: %08x\n", selected_theme_file, size, magic );

		strcpy( selected_theme_file, file );
		if ( size > 0 )
			stat->st_size = size;
		else stat->st_size = 0x00020000;
	}
	else
	{
    ret = msIoGetstat( arg, file, stat );
	}
	return ret;
}

int ( * efIoOpen )( PspIoDrvFileArg * arg, char * file, int flags, SceMode mode );
int ( * efIoGetstat )( PspIoDrvFileArg * arg, const char * file, SceIoStat * stat );

int efIoOpen_new( PspIoDrvFileArg * arg, char * file, int flags, SceMode mode )
{
	ms_drv = arg->drv;
	return efIoOpen( arg, file, flags, mode );
}

int efIoGetstat_new( PspIoDrvFileArg * arg, const char * file, SceIoStat * stat )
{
  log( "patpat:efIoGetstat_new %s.%d.\n",arg->drv->drv->name, arg->fs_num );
	int ret = 0;
  unsigned int magic;
	if ( endwithistr( file, ".ctf" ) )
	{
		int size = 0;
		if(strcmp(arg->drv->drv->name, "fatms") == 0)
		{
      sprintf( selected_theme_file, "ms0:%s", file );
      strcpy( ctf_drive, "ms0" );
    }
    else
    {
      sprintf( selected_theme_file, "ef0:%s", file );
      strcpy( ctf_drive, "ef0" );
    }
		int fd = sceIoOpen( selected_theme_file, PSP_O_RDONLY, 0644 );
		if ( fd >= 0 )
		{
      sceIoLseek( fd, 0x10, PSP_SEEK_SET );
      sceIoRead( fd, &magic, 4 );
      if ( magic != cxmb_magic )
      {
        int fw = initPatches();
        if ( ( (( fw != FW_500 && fw != FW_502 && fw != FW_503 ) || ( magic != 0xDEAD0500 && magic != 0xDEAD0502 && magic != 0xDEAD0503 )) 
			&& (( fw != FW_635 && fw != FW_637 && fw != FW_638 && fw != FW_639 ) || ( magic != 0xDEAD0635 && magic != 0xDEAD0637 && magic != 0xDEAD0638 && magic != 0xDEAD0639 )) 
			&& (( fw != FW_660 && fw != FW_661 ) || ( magic != 0xDEAD0660 && magic != 0xDEAD0661 )) ) 
			&& !endwithistr( file, "random.ctf" ) ) 
        {
          log( "%s version not match!\n", selected_theme_file );
          ret = -1;
        }
        else
        {
          ret = msIoGetstat( arg, file, stat );
        }
      }
			sceIoLseek( fd, 0x1C, PSP_SEEK_SET );
			sceIoRead( fd, &size, 4 );
			sceIoClose( fd );
		}
		log( "getstat %s size: %08x version: %08x\n", selected_theme_file, size, magic );

		strcpy( selected_theme_file, file );
		if ( size > 0 )
			stat->st_size = size;
		else stat->st_size = 0x00020000;
	}
	else
	{
    ret = msIoGetstat( arg, file, stat );
	}
	return ret;
}

int ( * IoOpen )( PspIoDrvFileArg * arg, char * file, int flags, SceMode mode );
int ( * IoRead )( PspIoDrvFileArg * arg, char * data, int len );
SceOff ( * IoLseek )( PspIoDrvFileArg * arg, SceOff ofs, int whence );
int ( * IoIoctl )( PspIoDrvFileArg * arg, unsigned int cmd, void * indata, int inlen, void * outdata, int outlen );
int ( * IoClose )( PspIoDrvFileArg * arg );
int ( * IoGetstat )( PspIoDrvFileArg * arg, const char * file, SceIoStat * stat );

int IoOpen_new( PspIoDrvFileArg * arg, char * file, int flags, SceMode mode )
{
	PspIoDrvArg * drv = arg->drv;
	if( ctf_handler && arg->fs_num == 0 )
	{
		ctf_handler[handler_count].num = inCtf( file );
		if ( ctf_handler[handler_count].num >= 0 )
		{
			log( "replace %s\n", file );
			arg->drv = ms_drv;
			int ret = fatms_drv->funcs->IoOpen( arg, theme_file, flags, mode);
			if ( ret < 0 )
			{
				arg->drv = drv;
			}
			else
			{
				ctf_handler[handler_count].offset = fatms_drv->funcs->IoLseek( arg, ctf_header[ctf_handler[handler_count].num].start, PSP_SEEK_SET );
				ctf_handler[handler_count].arg = arg->arg;
				handler_count ++;
				if ( handler_count % 32 == 0 )
				{
					CtfHandler * tmp = sceKernelAllocHeapMemory( mem_id, sizeof( CtfHandler ) * ( handler_count + 32 ) );
					memcpy( tmp, ctf_handler, sizeof( CtfHandler ) * handler_count );
					sceKernelFreeHeapMemory( mem_id, ctf_handler );
					ctf_handler = tmp;
				}
				arg->drv = drv;
				return ret;
			}
		}
	}
	int ret = IoOpen( arg, file, flags, mode );
	if ( strcmp( file, "/vsh/theme/custom_theme.dat" ) == 0 && flags == ( PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC ) )
	{
		t_record = arg->arg;
		log( "open %s flags %08x\ntheme file selected: %s. Classic CFW.\n", file, flags, selected_theme_file );
	}
	else if ( strcmp( file, "/vsn/theme/custom_theme.dat" ) == 0 && flags == ( PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC ) )
	{
		t_record = arg->arg;
		log( "open %s flags %08x\ntheme file selected: %s. Infinity CFW.\n", file, flags, selected_theme_file );
	}
	return ret;
}

int IoReopen( PspIoDrvFileArg * arg, int * num )
{
	unsigned int sig = 0;
	int err = 0;
	CtfHandler tmp;
	memcpy( &tmp, &ctf_handler[*num], sizeof( CtfHandler ) );
	int fd = -1, i = 0;
	for ( i = 0; fd < 0 && i < 10; i ++ )
	{
		sceKernelDelayThread( 100000 );
		fd = sceIoOpen( cxmb_theme_file, PSP_O_RDONLY, 0644 );
	}
	if ( fd >= 0 )
	{
		sceIoLseek( fd, 8, PSP_SEEK_SET );
		sceIoRead( fd, &sig, 4 );
		sceIoClose( fd );
		if ( sig == ctf_sig )
		{
			*num = isRedirected( arg );
			if ( *num >= 0 && ctf_handler[*num].num == tmp.num )
			{
				lflash_drv->funcs->IoClose( arg );
				lflash_drv->funcs->IoOpen( arg, ctf_header[tmp.num].name, PSP_O_RDONLY, 0644 );
				ctf_handler[*num].offset = tmp.offset;
				arg->drv = ms_drv;
        int ret = fatms_drv->funcs->IoLseek( arg, ctf_handler[*num].offset, PSP_SEEK_SET );
				if ( ret >= 0 )
				{
					return ret;
				}
				else
				{
					log( "failed in seek the reopen file!\noffset: %08x\n, ret: %08x\n", tmp.offset, ret );
					uninstall_cxmb();
					return ret;
				}
			}
			else
			{
				log( "failed in reopen ctf theme file!\n" );
				err = -2;
			}
		}
		else
		{
			log( "ctf theme file changed!\nori %08x now: %08x\n", ctf_sig, sig );
			err = -3;
		}
	}
	else
	{
		log( "ctf theme file removed!\n" );
		err = -4;
	}
	lflash_drv->funcs->IoClose( arg );
	uninstall_cxmb();
	lflash_drv->funcs->IoOpen( arg, ctf_header[tmp.num].name, PSP_O_RDONLY, 0644 );
	log( "uninstalled!\n" );
	return err;
}

int IoRead_new( PspIoDrvFileArg * arg, char * data, int len )
{
	PspIoDrvArg * drv = arg->drv;
	int num = isRedirected( arg );
	if( num >= 0 && arg->fs_num == 0 )
	{
		arg->drv = ms_drv;
    int ret = fatms_drv->funcs->IoLseek( arg, 0, PSP_SEEK_CUR );
		if ( ret < 0 )
		{
			log( "error: %08x when read %s\n", ret, ctf_header[ctf_handler[num].num].name );
			arg->drv = drv;
			ret = IoReopen( arg, &num );
			if ( ret < 0 )
			{
				return ret;
			}
		}
		int sub = ret + len - ctf_header[ctf_handler[num].num].start - ctf_header[ctf_handler[num].num].size;
		if ( sub <= 0 )
		{
			sub = 0;
		}
		ret =  fatms_drv->funcs->IoRead( arg, data, len - sub );
		ctf_handler[num].offset += ret;
		arg->drv = drv;
		return ret;
	}
	int ret = IoRead( arg, data, len );
	return ret;
}

SceOff IoLseek_new( PspIoDrvFileArg * arg, SceOff ofs, int whence )
{
	PspIoDrvArg * drv = arg->drv;
	int num = isRedirected( arg );
	if( num >= 0 && arg->fs_num == 0 )
	{
		arg->drv = ms_drv;
    int ret = fatms_drv->funcs->IoLseek( arg, 0, PSP_SEEK_CUR );
		if ( ret < 0 )
		{
			log( "error: %08x when seek %s\n", ret, ctf_header[ctf_handler[num].num].name );
			arg->drv = drv;
			ret = IoReopen( arg, &num );
			if ( ret < 0 )
			{
				return ret;
			}
		}
		switch ( whence )
		{
			case PSP_SEEK_SET:
				fatms_drv->funcs->IoLseek( arg, ctf_header[ctf_handler[num].num].start, PSP_SEEK_SET );
				break;
			case PSP_SEEK_END:
				fatms_drv->funcs->IoLseek( arg, ctf_header[ctf_handler[num].num].start + ctf_header[ctf_handler[num].num].size, PSP_SEEK_SET );
				break;
		}
		ret = fatms_drv->funcs->IoLseek( arg, ofs, PSP_SEEK_CUR );
		if ( ret < ctf_header[ctf_handler[num].num].start )
		{
			ret = fatms_drv->funcs->IoLseek( arg, ctf_header[ctf_handler[num].num].start, PSP_SEEK_SET );
		}
		else if ( ret > ( ctf_header[ctf_handler[num].num].start + ctf_header[ctf_handler[num].num].size ) )
		{
			ret = fatms_drv->funcs->IoLseek( arg, ctf_header[ctf_handler[num].num].start + ctf_header[ctf_handler[num].num].size, PSP_SEEK_SET );
		}
		ctf_handler[num].offset = ret;
		ret -= ctf_header[ctf_handler[num].num].start;
		arg->drv = drv;
		return ret;
	}
	int ret =  IoLseek( arg, ofs, whence );
	return ret;
}

int IoIoctl_new( PspIoDrvFileArg * arg, unsigned int cmd, void * indata, int inlen, void * outdata, int outlen )
{
	PspIoDrvArg * drv = arg->drv;
	int num = isRedirected( arg );
	if( num >= 0 && arg->fs_num == 0 )
	{
		arg->drv = ms_drv;
		int ret = fatms_drv->funcs->IoIoctl( arg, cmd, indata, inlen, outdata, outlen );
		
		if ( ret < 0 )
			log( "error: %08x when ioctl cmd %08x %s\n", ret, cmd, ctf_header[ctf_handler[num].num].name );
			
		arg->drv = drv;
		return ret;
	}
	int ret =  IoIoctl( arg, cmd, indata, inlen, outdata, outlen );
	return ret;
}

int IoClose_new( PspIoDrvFileArg * arg )
{
	PspIoDrvArg * drv = arg->drv;
	int num = isRedirected( arg );
	if( num >= 0 && arg->fs_num == 0 )
	{
		arg->drv = ms_drv;
		handler_count --;
		memcpy( &ctf_handler[num], &ctf_handler[num + 1], sizeof( CtfHandler ) * ( handler_count - num ) );
    int ret = fatms_drv->funcs->IoClose( arg );
		arg->drv = drv;
		return ret;
	}
	if ( arg->arg == t_record )
	{
		log( "write finished!\n" );
		int fd = sceIoOpen( cxmb_conf_file, PSP_O_RDWR | PSP_O_CREAT | PSP_O_TRUNC, 0777 );
		if ( fd < 0 )
		{
			log( "failed in openning %s\n", cxmb_conf_file );
		}
		else
		{
      char stmp[4];
			sceIoWrite( fd, selected_theme_file, strlen( selected_theme_file ) );
			log("patpat:ctf_drive:%s selected_theme_file:%s\n", ctf_drive, selected_theme_file);
			sprintf(stmp, "\n%s", ctf_drive);
			sceIoWrite( fd, stmp, 4 );
			sceIoClose( fd );
		}
		IoClose( arg );
		sceKernelSignalSema( sema, 1 );
	}
	arg->drv = drv;
	int ret = IoClose(arg);
	return ret;
}

int IoGetstat_new( PspIoDrvFileArg * arg, const char * file, SceIoStat * stat )
{
	int ret = IoGetstat( arg, file, stat );
	int num = inCtf( file );
	if( num >= 0 && arg->fs_num == 0 )
	{
		stat->st_size = ctf_header[num].size;
	}
	return ret;
}

int parseDiff( const char * file, tSceModule * mod )
{
	int off = inCtf( file );
	if ( off < 0 )
	{
		log( "there's no patch for %s\n", file );
		return 0;
	}
	
	int ctf = sceIoOpen( cxmb_theme_file, PSP_O_RDONLY, 0644 );
	if ( ctf < 0 )
	{
		log( "no ctf file found!\n" );
		return -1;
	}
	sceIoLseek( ctf, ctf_header[off].start, PSP_SEEK_SET );
	
	log( "patch %s!\nstart: %08x\nsize: %08x\n", file, ctf_header[off].start, ctf_header[off].size );
	
	unsigned int attr[2];
	int i = 0;
	while( i < ctf_header[off].size )
	{
		sceIoRead( ctf, attr, 8 );
		sceIoRead( ctf, ( void * )( mod->text_addr + attr[0] ), attr[1] );
		i ++;
	}
	sceIoClose( ctf );
	
	sceKernelIcacheInvalidateAll();
	sceKernelDcacheWritebackInvalidateAll();
	
	log( "%s patched!\n", file );
	return 0;
}

int OnModuleStart( tSceModule * mod )
{
	log( "on module %s start\n", mod->modname );
	if ( strcmp( mod->modname, "scePaf_Module" ) == 0 )
	{
		parseDiff("/vsh/module/paf.prx", mod );
	}
	else if ( strcmp( mod->modname, "sceVshCommonGui_Module" ) == 0 )
	{
		parseDiff("/vsh/module/common_gui.prx", mod );
	}
	else if ( strcmp( mod->modname, "vsh_module" ) == 0 )
	{
		parseDiff("/vsh/module/vshmain.prx", mod );
		
	}
	else if ( strcmp( mod->modname, "sysconf_plugin_module" ) == 0 )
	{
		unsigned int offset = getSysconfOffset();
		if ( fw_version == FW_500 || fw_version == FW_502 || fw_version == FW_503 || fw_version == FW_550 
			|| fw_version == FW_620 || fw_version == FW_631 || fw_version == FW_635 || fw_version == FW_637 
			|| fw_version == FW_638 || fw_version == FW_639 || fw_version == FW_660 || fw_version == FW_661)
		{
			unsigned int addr = mod->text_addr + offset;
			char *sfx = (char *)addr;
			sfx[0] = 'C';
		}
		else
		{
			unsigned int h_addr = _lw( mod->text_addr + offset );
			unsigned int l_addr = _lw( mod->text_addr + offset + 0xC );
			unsigned int addr = ( ( h_addr & 0xFFFF ) << 16 ) | ( l_addr & 0xFFFF );
			char * sfx = "CTF";
			_sw( *( unsigned int * )sfx, addr );
		}
		sceKernelIcacheInvalidateAll();
		sceKernelDcacheWritebackInvalidateAll();
		log( "patched sysconf\n" );
	}
	if ( !previous )
		return 0;
	return previous( mod );
}

int randomCtf( char * theme )
{
	int dfd = sceIoDopen( "ms0:/PSP/THEME" );
	if ( dfd < 0 )
		return -1;

	int th_count = 0;
	SceIoDirent ent;
	memset( &ent, 0, sizeof( SceIoDirent ) );
	while ( sceIoDread( dfd, &ent ) > 0 )
	{
		if ( !( ent.d_stat.st_attr & FIO_SO_IFDIR ) && ent.d_name[0] != '.' && cmpistr( ent.d_name, "random.ctf" ) && endwithistr( ent.d_name, ".ctf" ) )
		{
			th_count ++;
		}
		log( "name %s %d\n", ent.d_name, th_count );
		memset( &ent, 0, sizeof( SceIoDirent ) );
	}
	sceIoDclose( dfd );
	if ( th_count <= 0 )
		return -1;
	if ( th_count > 1 )
	{
		time_t tm = sceKernelLibcTime( NULL );
		th_count = tm % th_count;
	}
	else th_count = 0;
	dfd = sceIoDopen( "ms0:/PSP/THEME" );
	memset( &ent, 0, sizeof( SceIoDirent ) );
	while ( sceIoDread( dfd, &ent ) > 0 )
	{
		log( "name %s %d\n", ent.d_name, th_count );
		if ( !( ent.d_stat.st_attr & FIO_SO_IFDIR ) && ent.d_name[0] != '.' && cmpistr( ent.d_name, "random.ctf" ) && endwithistr( ent.d_name, "ctf" ) )
		{
			if ( th_count == 0 )
			{
				sprintf( theme, "/PSP/THEME/%s", ent.d_name );
				break;
			}
			else th_count --;
		}
		memset( &ent, 0, sizeof( SceIoDirent ) );
	}
	sceIoDclose( dfd );
	return 0;
}

int install_cxmb( void )
{
  log("Going to install cxmb!\n");
  int fw = initPatches();
  if( fw == FW_550 )
  {
    fatms_drv = sctrlHENFindDriver( "fatms" );    
    fatef_drv = sctrlHENFindDriver( "fatef" );    
    lflash_drv = sctrlHENFindDriver( "flashfat" );
	}
	else
	{
    fatms_drv = findDriver( "fatms" );    
    fatef_drv = findDriver( "fatef" );    
    lflash_drv = findDriver( "flashfat" );
	}
	if(!fatef_drv) 
  {
    isGO = -1;
    log("Can not found fatef, it is not PSPGO!\n");
  }
  else
  {
    isGO = 0;
    log("Found fatef, is PSPGO!\n");
  }

  int dfd = sceIoDopen( "ef0:/seplugins/cxmb" );
  if (dfd < 0)
  {
    log("Can not open ef0:/seplugins/cxmb\n");
    strcpy( cxmb_conf_file, "ms0:/seplugins/cxmb/conf.txt" );
    strcpy( cxmb_drive, "ms0" );
  }
  else
  {
    log("open ef0:/seplugins/cxmb\n");
    strcpy( cxmb_conf_file, "ef0:/seplugins/cxmb/conf.txt" );
    strcpy( cxmb_drive, "ef0" );
  }
  
  log("cxmb_conf_file:%s.\n", cxmb_conf_file);
  sceIoDclose( dfd );

	int fd = sceIoOpen( cxmb_conf_file, PSP_O_RDONLY, 0644 );
	if ( fd < 0 )
	{
		log( "no conf file found!\n" );
		fd = sceIoOpen( cxmb_conf_file, PSP_O_RDWR | PSP_O_CREAT | PSP_O_TRUNC, 0777 );
		if ( fd < 0 )
		{
			log( "failed in creating conf file!\n" );
			return -1;
		}
		strcpy( theme_file, CXMB_DEFAULT_THEME );
		sceIoWrite( fd, theme_file, strlen( theme_file ) + 1 );
	}
	
	readLine( fd, theme_file, 64 );
	
	int i = 0;
	sceIoLseek(fd, 0, PSP_SEEK_SET);
  while( i < 64 && sceIoRead(fd, ctf_drive, 1) )
  {
    if(ctf_drive[0] == '\n')
    {
      sceIoRead(fd, ctf_drive, 3);
      ctf_drive[3] = 0;
      log("patpat:cxmb_drive:%s.%d.\n", ctf_drive, i);
      break;
    }
    i++;
  }
	if( strcmp(ctf_drive, "ef0") != 0 || isGO < 0 )
  {
    strcpy(ctf_drive, "ms0");
	}
    
	if ( truncpath( theme_file, ".ctf" ) < 0 )
	{
		if ( truncpath( theme_file, ".CTF" ) < 0 )
		{
			strcpy( theme_file, CXMB_DEFAULT_THEME );
		}
	}
	sceIoClose( fd );
	
	if ( endwithistr( theme_file, "random.ctf" ) )
	{
		log( "random\n" );
		randomCtf( theme_file );
	}
	
	sprintf( cxmb_theme_file, "%s:%s", ctf_drive, theme_file );
	log( "Theme file: %s\n", cxmb_theme_file );
	
	#if _CXMB_LITE == 0
	makeCxmbThemeFile( cxmb_magic, cxmb_theme_file, cxmb_drive );
	#endif
	
	if ( !lflash_drv || (!fatms_drv && !fatef_drv) )
		return -1;
		
	if(fatef_drv)	
	{
    efIoOpen	= fatef_drv->funcs->IoOpen;
    efIoGetstat	= fatef_drv->funcs->IoGetstat;
	}
	if(fatms_drv)	
	{
    msIoOpen	= fatms_drv->funcs->IoOpen;
    msIoGetstat	= fatms_drv->funcs->IoGetstat;
	}
	IoOpen		= lflash_drv->funcs->IoOpen;
	IoRead		= lflash_drv->funcs->IoRead;
	IoLseek		= lflash_drv->funcs->IoLseek;
	IoIoctl		= lflash_drv->funcs->IoIoctl;
	IoClose		= lflash_drv->funcs->IoClose;
	IoGetstat	= lflash_drv->funcs->IoGetstat;
	
	int intr = sceKernelCpuSuspendIntr();

	if(fatef_drv)	
	{	
    fatef_drv->funcs->IoOpen	= efIoOpen_new;
    fatef_drv->funcs->IoGetstat	= efIoGetstat_new;
	}
	if(fatms_drv)	
	{
    fatms_drv->funcs->IoOpen	= msIoOpen_new;
    fatms_drv->funcs->IoGetstat	= msIoGetstat_new;
	}
	lflash_drv->funcs->IoOpen	= IoOpen_new;
	lflash_drv->funcs->IoRead	= IoRead_new;
	lflash_drv->funcs->IoLseek	= IoLseek_new;
	lflash_drv->funcs->IoIoctl	= IoIoctl_new;
	lflash_drv->funcs->IoClose	= IoClose_new;
	lflash_drv->funcs->IoGetstat= IoGetstat_new;
	
	sceKernelCpuResumeIntr( intr );
	sceKernelIcacheInvalidateAll();
	sceKernelDcacheWritebackInvalidateAll();
	log( "redirected io_driver!\n" );
	if(strcmp(ctf_drive, "ef0") == 0)	
	{	
    sceIoOpen( "ef0:/dummy.prx", PSP_O_RDONLY, 0644 );
    log( "ef_drv_arg: %08x\n", ( unsigned int )ms_drv );
	}	
	else
	{	
    sceIoOpen( "ms0:/dummy.prx", PSP_O_RDONLY, 0644 );
    log( "ms_drv_arg: %08x\n", ( unsigned int )ms_drv );
	}

	previous = setStartModuleHandler( OnModuleStart );
	log("startModuleHandler setup!\n");
	
	fd = sceIoOpen( cxmb_theme_file, PSP_O_RDONLY, 0644 );
	if ( fd < 0 )
	{
    sprintf( cxmb_theme_file, "ms0:%s", theme_file );
    fd = sceIoOpen( cxmb_theme_file, PSP_O_RDONLY, 0644 );
    if ( fd < 0 )
    {
      log( "no ctf file found!\n" );
      return 0;
    }
	}
	
	mem_id = sceKernelCreateHeap( 2, 1024 * 5 , 1, "cxmb_heap");
	if ( mem_id < 0 )
	{
		log( "failed in creating cxmb_heap!\n" );
		sceIoClose( fd );
		return -1;
	}
	
	unsigned int magic;
	sceIoLseek( fd, 0x10, PSP_SEEK_SET );
	sceIoRead( fd, &magic, 4 );
	if ( magic != cxmb_magic )
	{
    int fw = initPatches();
        if ( (( fw != FW_500 && fw != FW_502 && fw != FW_503 ) || ( magic != 0xDEAD0500 && magic != 0xDEAD0502 && magic != 0xDEAD0503 )) 
			&& (( fw != FW_635 && fw != FW_637 && fw != FW_638 && fw != FW_639 ) || ( magic != 0xDEAD0635 && magic != 0xDEAD0637 && magic != 0xDEAD0638 && magic != 0xDEAD0639 )) 
			&& (( fw != FW_660 && fw != FW_661 ) || ( magic != 0xDEAD0660 && magic != 0xDEAD0661 )) ) 
    {
      log( "magic not match!\n" );
      sceIoClose( fd );
      return -1;
		}
	}
	sceIoRead( fd, &ctf_sig, 4 );
	sceIoRead( fd, &header_size, 4 );
	log( "header size: %d\n", header_size );
	
	ctf_header = sceKernelAllocHeapMemory( mem_id, sizeof( CtfHeader ) * header_size );
	int offset = - sizeof( CtfHeader ) * header_size;
	sceIoLseek( fd, offset, PSP_SEEK_END );
	sceIoRead( fd, ctf_header, sizeof( CtfHeader ) * header_size );
	sceIoClose( fd );
	
	log( "read ctf_header!\n" );
	
	ctf_handler = sceKernelAllocHeapMemory( mem_id, sizeof( CtfHandler ) * 32 );
	memset( ctf_handler, 0, sizeof( CtfHandler ) * 32 );
	
	return 0;
}

int main_thread(SceSize args, void *argp)
{
	sema = sceKernelCreateSema( "cxmb_reboot", 0, 0, 1, NULL );
	sceKernelWaitSemaCB( sema, 1, NULL );
	int fw = initPatches();
	if( fw == FW_502 || fw == FW_503 || fw == FW_620 || fw == FW_631 || fw == FW_635 || fw == FW_637 || fw == FW_638 || fw == FW_639 || fw == FW_660 || fw == FW_661)
		sceKernelDelayThread( 3650000 );
	else
		sceKernelDelayThread( 2500000 );
		
	log( "reboot!\n" );
	if( fw != FW_502 && fw != FW_503 && fw != FW_620 && fw != FW_631 && fw != FW_635 && fw != FW_637 && fw != FW_638 && fw != FW_639 && fw != FW_660 && fw != FW_661)
		rebootPsp();
	else
		sceKernelExitVSHKernel(NULL);
		
	return 0;
}

int module_start( SceSize args, void *argp )
{
  log( "CXMB debug Version: 3.71 - 6.61 Classic & 6.61 Infinity R2\n" );
	int fw = initPatches();
	sceIoAssign("ms0:", "msstor0p1:", "fatms0:", IOASSIGN_RDWR, NULL, 0);
	log( "Firmware Version: %08x\n", fw );
	cxmb_magic = getMagic();
	if ( cxmb_magic == 0 )
	{
		log( "Not supported firmware version!\n" );
	}
	log( "CXMB Magic: %08x\n", cxmb_magic );
	initUtils();
	install_cxmb();
	int thid = sceKernelCreateThread("cxmb_thread", main_thread, 47, 0x400, 0x00100001, NULL);
	if( thid >= 0 )
		sceKernelStartThread(thid, 0, 0);
	return 0;
}

int module_stop( SceSize args, void *argp )
{
	return 0;
}
