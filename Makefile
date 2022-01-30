#
#	This file is part of cxmb
#	Copyright (C) 2008  Poison
#
#	This program is free software: you can redistribute it and/or modify
#	it under the terms of the GNU General Public License as published by
#	the Free Software Foundation, either version 3 of the License, or
#	(at your option) any later version.
#
#	This program is distributed in the hope that it will be useful,
#	but WITHOUT ANY WARRANTY; without even the implied warranty of
#	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#	GNU General Public License for more details.
#
#	You should have received a copy of the GNU General Public License
#	along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

#	File:		Makefile.PSP
#	Author:		Poison <hbpoison@gmail.com>
#	Date Created:	2008-07-01

#patpat mod for 3.71 - 6.37 <2011-02-19>
#neur0n mod for 6.39
#frostegater mod for 6.60
#LMAN mod for 6.61
#Yoti mod for 3.71 - 6.61 CLASSIC & 6.61 INFINITY
 
TARGET			= cxmb
SRCS			= log.c syspatch.c utils.c ctf.c main.c
OBJS			= $(SRCS:.c=.o)
#DEBUG			= 1
BUILD_PRX		= 1
USE_KERNEL_LIBC	= 1
USE_KERNEL_LIBS = 1
PSP_FW_VERSION	= 371

PRX_EXPORTS		= exports.exp

INCDIR			= inc_lib
LIBDIR			= inc_lib

LDFLAGS			= -mno-crt0 -nostartfiles
LIBS			= -lpspsystemctrl_kernel
CFLAGS			= -Os -G0 -Wall -fno-strict-aliasing -fno-builtin-printf
CXXFLAGS		= $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS			= $(CFLAGS)

ifeq ($(DEBUG),)
DEBUG			= 0
endif

ifeq ($(CXMB_LITE),)
CXMB_LITE		= 0
endif

CFLAGS			+= -D_DEBUG=$(DEBUG) -D_CXMB_LITE=$(CXMB_LITE)
CXXFLAGS		+= -D_DEBUG=$(DEBUG) -D_CXMB_LITE=$(CXMB_LITE)

PSPSDK			= $(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
