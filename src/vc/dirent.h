/*
 * Defines and structures used to implement the
 * functionality standard in direct.h for:
 *
 * opendir(), readdir(), closedir() and rewinddir().
 *
 * 06/17/2000 by Mike Haaland <mhaaland@hypertech.com>
 */
#pragma once

#ifdef _WIN32

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef _WIN32_WINNT
#if _MSC_VER >= 1800
 // Windows 2000 _WIN32_WINNT_WIN2K
 #define _WIN32_WINNT 0x0500
#elif _MSC_VER < 1600
 #define _WIN32_WINNT 0x0400
#else
 #define _WIN32_WINNT 0x0403
#endif
#define WINVER _WIN32_WINNT
#endif
#include <windows.h>
#include <direct.h>
#include <sys/types.h>

#if !defined(__GNUC__)
/* Convienience macros used with stat structures */
#define S_ISDIR(x) ((x) & _S_IFDIR)
#define S_ISREG(x) ((x) & _S_IFREG)
#endif

/* Structure to keep track of the current directory status */
typedef struct my_dir {
    HANDLE          handle;
    WIN32_FIND_DATA findFileData;
    BOOLEAN         firstTime;
    char            pathName[MAX_PATH];
} DIR;

/* Standard directory name entry returned by readdir() */
struct dirent {
  unsigned int d_namlen;
  char d_name[MAX_PATH];
};

/* function prototypes */
int		        closedir(DIR *dirp);
DIR *		    opendir(const char *dirname);
struct dirent *	readdir(DIR *dirp);
void		    rewinddir(DIR *dirp);

#endif
