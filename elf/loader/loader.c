/*==================================================================
==											==
==	Copyright(c)2004  Adam Metcalf(gamblore_@hotmail.com)		==
==	Copyright(c)2004  Thomas Hawcroft(t0mb0la@yahoo.com)		==
==	This file is subject to terms and conditions shown in the	==
==	file LICENSE which should be kept in the top folder of	==
==	this distribution.							==
==											==
==	Portions of this code taken from PS2Link:				==
==				pkoLoadElf						==
==				wipeUserMemory					==
==				(C) 2003 Tord Lindstrom (pukko@home.se)	==
==				(C) 2003 adresd (adresd_ps2dev@yahoo.com)	==
==	Portions of this code taken from Independence MC exploit	==
==				tLoadElf						==
==				LoadAndRunHDDElf					==
==				(C) 2003 Marcus Brown <mrbrown@0xd6.org>	==
==											==
==================================================================*/
#include "tamtypes.h"
#include "debug.h"
#include "kernel.h"
#include "sifrpc.h"
#include "loadfile.h"
#include "fileio.h"
#include "iopcontrol.h"
#include "stdarg.h"
#include "string.h"
#include "malloc.h"
#include "libmc.h"
#include "iopheap.h"
#include "sys/fcntl.h"
#include "sys/stat.h"
#include "sys/ioctl.h"
#include "fileXio_rpc.h"
#include "errno.h"
#include "libhdd.h"
#include "sbv_patches.h"

//#define DEBUG
#ifdef DEBUG
#define dbgprintf(args...) scr_printf(args)
#define dbginit_scr() init_scr()
#else
#define dbgprintf(args...) do { } while(0)
#define dbginit_scr() do { } while(0)
#endif

// ELF-header structures and identifiers
#define ELF_MAGIC	0x464c457f
#define ELF_PT_LOAD	1

typedef struct
{
	u8	ident[16];
	u16	type;
	u16	machine;
	u32	version;
	u32	entry;
	u32	phoff;
	u32	shoff;
	u32	flags;
	u16	ehsize;
	u16	phentsize;
	u16	phnum;
	u16	shentsize;
	u16	shnum;
	u16	shstrndx;
	} elf_header_t;

typedef struct
{
	u32	type;
	u32	offset;
	void	*vaddr;
	u32	paddr;
	u32	filesz;
	u32	memsz;
	u32	flags;
	u32	align;
	} elf_pheader_t;

t_ExecData elfdata;

extern u8 *fakehost_irx;
extern int size_fakehost_irx;
extern u8 *poweroff_irx;
extern int size_poweroff_irx;

int fileMode =  FIO_S_IRUSR | FIO_S_IWUSR | FIO_S_IXUSR | FIO_S_IRGRP | FIO_S_IWGRP | FIO_S_IXGRP | FIO_S_IROTH | FIO_S_IWOTH | FIO_S_IXOTH;
char elfName[256];
char elfPath[256];
char HDDpath[256];
char partition[128];

const char *eeloadimg = "rom0:UDNL rom0:EELOADCNF";
char *imgcmd;

static int pkoLoadElf(char *path);

int userThreadID = 0;
static char userThreadStack[16*1024] __attribute__((aligned(16)));

#define MAX_ARGS 16
#define MAX_ARGLEN 256

struct argData
{
    int flag;                     // Contains thread id atm
    int argc;
    char *argv[MAX_ARGS];
} __attribute__((packed)) userArgs;

////////////////////////////////////////////////////////////////////////
// Read ELF from hard drive to required location(s) in memory.
// Modified version of loader from Independence
//	(C) 2003 Marcus R. Brown <mrbrown@0xd6.org>
//
static int tLoadElf(char *filename)
{
	u8 *boot_elf = (u8 *)0x1800000;
	elf_header_t *eh = (elf_header_t *)boot_elf;
	elf_pheader_t *eph;
	char temp[256], *p;

	int fd, size, i, ret;

	// ntba2 -start-
	fileXioUmount("pfs0:");
	memset( temp, 0, sizeof(temp) );
	i = strcspn( filename, ":" );
	strncpy( temp, filename, i+1 );
	fileXioUmount(temp);
	strcpy(temp, "pfs0");
	p = strstr( filename, ":" );
	strcat(temp, p);
	strcpy(filename, temp);
	scr_printf("Loading %s\n", filename);
	scr_printf("Mounting Partition %s\n", partition);
	ret = fileXioMount("pfs0:", partition, FIO_MT_RDONLY);
	scr_printf("fileXioMount returned %i\n", ret);
	// ntba2 -end-

	if ((fd = fileXioOpen(filename, O_RDONLY, fileMode)) < 0)
	{
		dbgprintf("Failed in fileXioOpen %s\n",filename);
		goto error;
		}
	dbgprintf("Opened file %s\n",filename);
	size = fileXioLseek(fd, 0, SEEK_END);
	dbgprintf("File size = %i\n",size);
	if (!size)
	{
		dbgprintf("Failed in fileXioLseek\n");
		fileXioClose(fd);
		goto error;
		}
	fileXioLseek(fd, 0, SEEK_SET);
	fileXioRead(fd, boot_elf, 52);
	dbgprintf("Read elf header from file\n");
	fileXioLseek(fd, eh->phoff, SEEK_SET);
	eph = (elf_pheader_t *)(boot_elf + eh->phoff);
	size=eh->phnum*eh->phentsize;
	size=fileXioRead(fd, (void *)eph, size);
	dbgprintf("Read %i bytes of program header(s) from file\n",size);
	for (i = 0; i < eh->phnum; i++)
	{
		if (eph[i].type != ELF_PT_LOAD)
		continue;

		fileXioLseek(fd, eph[i].offset, SEEK_SET);
		size=eph[i].filesz;
		size=fileXioRead(fd, eph[i].vaddr, size);
		dbgprintf("Read %i bytes to %x\n", size, eph[i].vaddr);
		if (eph[i].memsz > eph[i].filesz)
			memset(eph[i].vaddr + eph[i].filesz, 0,
					eph[i].memsz - eph[i].filesz);
		}		

	fileXioClose(fd);
//	fileXioUmount("pfs0:");		We leave the filesystem mounted now for fakehost

	if (_lw((u32)&eh->ident) != ELF_MAGIC)		// this should have already been
	{								// done by menu, but a double-check
		dbgprintf("Not a recognised ELF.\n");	// doesn't do any harm
		goto error;
		}
	
	dbgprintf("entry=%x\n",eh->entry);
	elfdata.epc=eh->entry;
	return 0;
error:
	elfdata.epc=0;
	return -1;
	}

////////////////////////////////////////////////////////////////////////
// Load the actual elf, and create a thread for it
// Return the thread id
// PS2Link (C) 2003 Tord Lindstrom (pukko@home.se)
//         (C) 2003 adresd (adresd_ps2dev@yahoo.com)
static int
pkoLoadElf(char *path)
{
    ee_thread_t th_attr;
    int ret=0;
    int pid;

// Some lines added here to check for loading from HDD, at the moment it should only
// call tLoadElf, but support for loading ELF from mc0, host or any other device may
// well be added in future
    if(!strncmp(path, "host", 4)) ret = SifLoadElf(path, &elfdata);
    else if(!strncmp(path, "mc", 2)) ret = SifLoadElf(path, &elfdata);
	else if(!strncmp(path, "mass", 4)) ret = SifLoadElf(path, &elfdata);
    else if(!strncmp(path, "cdrom", 5)) ret = SifLoadElf(path, &elfdata);
    else if(!strncmp(path, "cdfs", 4)) ret = SifLoadElf(path, &elfdata);

	// ntba2
    else if(!strncmp(path, "pfs", 3)) ret = tLoadElf(path);
    else ret = SifLoadElf(path, &elfdata);

    FlushCache(0);
    FlushCache(2);

    dbgprintf("EE: LoadElf returned %d\n", ret);

    dbgprintf("EE: Creating user thread (ent: %x, gp: %x, st: %x)\n", 
              elfdata.epc, elfdata.gp, elfdata.sp);

    if (elfdata.epc == 0) {
        dbgprintf("EE: Could not load file\n");
        return -1;
    }

    th_attr.func = (void *)elfdata.epc;
    th_attr.stack = userThreadStack;
    th_attr.stack_size = sizeof(userThreadStack);
    th_attr.gp_reg = (void *)elfdata.gp;
    th_attr.initial_priority = 64;

    pid = CreateThread(&th_attr);
    if (pid < 0) {
        dbgprintf("EE: Create user thread failed %d\n", pid);
        return -1;
    }
    dbgprintf("EE: Created user thread: %d\n", pid);

    return pid;
}

////////////////////////////////////////////////////////////////////////
// Clear user memory
// PS2Link (C) 2003 Tord Lindstrom (pukko@home.se)
//         (C) 2003 adresd (adresd_ps2dev@yahoo.com)
void
wipeUserMem(void)
{
    int i;
    for (i = 0x100000; i < 0x2000000 ; i += 64) {
        asm (
            "\tsq $0, 0(%0) \n"
            "\tsq $0, 16(%0) \n"
            "\tsq $0, 32(%0) \n"
            "\tsq $0, 48(%0) \n"
            :: "r" (i) );
    }
}

////////////////////////////////////////////////////////////////////////
// C standard strrchr func.. returns pointer to the last occurance of a
// character in a string, or NULL if not found
// PS2Link (C) 2003 Tord Lindstrom (pukko@home.se)
//         (C) 2003 adresd (adresd_ps2dev@yahoo.com)
char *strrchr(const char *sp, int i)
{
	const char *last = NULL;
	char c = i;

	while (*sp)
	{
		if (*sp == c)
		{
			last = sp;
			}
		sp++;
		}

	if (*sp == c)
	{
		last = sp;
		}

	return (char *) last;
}

void poweroffHandler(int i)
{
	hddPowerOff();
}

// *************************************************************************
// *** MAIN
// *************************************************************************
int main(int argc, char *argv[])
{
	char s[256],fakepart[128], *ptr;
	int pid=-1,ret;

// Initialise
	SifInitRpc(0);
	hddPreparePoweroff();
	hddSetUserPoweroffCallback((void *)poweroffHandler,(void *)pid);
	init_scr();
	wipeUserMem();
	dbgprintf("Welcome to PS2Menu Loader v2.1\nPlease wait...loading.\n");
	dbgprintf("Init MrBrown sbv_patches\n");
	sbv_patch_enable_lmb();			// not sure we really need to do this again
	sbv_patch_disable_prefix_check();	// here, but will it do any harm?
	dbgprintf("Loading poweroff.irx %i bytes\n", size_poweroff_irx);
	SifExecModuleBuffer(&poweroff_irx, size_poweroff_irx, 0, NULL, &ret);

	strcpy(s,argv[0]);
	dbgprintf("argv[0] = %s\n",s);
	if (argc==1)
	{						// should be two params passed by menu
		while(1);				// leave this here for adding mc0, host or other
							// to be added in future
		}
	if (argc==2)				// if call came from hddmenu.elf
	{						// arg1=path to ELF, arg2=partition to mount
		strcpy(partition,argv[1]);
		dbgprintf("argv[1] = %s\n", partition);
		strcpy(HDDpath,s);
		}
	scr_printf("Loading %s\n",HDDpath);
	pid = pkoLoadElf(HDDpath);
	dbgprintf("pkoLoadElf returned %i\n",pid);
	if (pid < 0)
	{
		scr_printf("failed\n");
		dbgprintf("Could not execute file %s\n", HDDpath);
		return -1;
		}
	if(!strncmp(HDDpath, "pfs0", 4))
	{
		strcpy(fakepart,HDDpath);
		ptr=strrchr(fakepart,'/');
		if(ptr==NULL) strcpy(fakepart,"pfs0:");
		else
		{
			ptr++;
			*ptr='\0';
			}
		ptr=strrchr(s,'/');
		if(ptr==NULL) ptr=strrchr(s,':');
		if(ptr!=NULL)
		{
			ptr++;
			strcpy(HDDpath,"host:");
			strcat(HDDpath,ptr);
			}
		dbgprintf("Loading fakehost.irx %i bytes\n", size_fakehost_irx);
		dbgprintf("%s\n", fakepart);
		SifExecModuleBuffer(&fakehost_irx, size_fakehost_irx, strlen(fakepart), fakepart, &ret);
		}
	
	FlushCache(0);
	FlushCache(2);
	
	userThreadID = pid;

	userArgs.argc=1;
	userArgs.argv[0]=HDDpath;
	userArgs.flag = (int)&userThreadID;

	ret = StartThread(userThreadID, &userArgs);
	if (ret < 0)
	{
		scr_printf("failed\n");
		dbgprintf("EE: Start user thread failed %d\n", ret);
		DeleteThread(userThreadID);
		return -1;
		}
	SleepThread();
	
	return 0;
}
