/***************************************************************************

  M.A.M.E.32	-  Multiple Arcade Machine Emulator for Win32
  Win32 Portions Copyright (C) 1997-2003 Michael Soderstrom and Chris Kirmse

  This file is part of MAME32, and may only be used, modified and
  distributed under the terms of the MAME license, in "readme.txt".
  By continuing to use, modify or distribute this file you indicate
  that you have read the license and understand and accept it fully.

 ***************************************************************************/

/***************************************************************************

  M32Util.c

 ***************************************************************************/

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
#include <shellapi.h>
#include <shlwapi.h>
#include <assert.h>
#include <stdio.h>

#include "unzip.h"
#include "Screenshot.h"
#include "MAME32.h"
#include "M32Util.h"

#ifdef _MSC_VER
#include "msc.h"
#endif

/***************************************************************************
	function prototypes
 ***************************************************************************/

/***************************************************************************
	External variables
 ***************************************************************************/

/***************************************************************************
	Internal structures
 ***************************************************************************/

/***************************************************************************
	Internal variables
 ***************************************************************************/
#ifndef PATH_SEPARATOR
#ifdef _MSC_VER
#define PATH_SEPARATOR '\\'
#else
#define PATH_SEPARATOR '/'
#endif
#endif


/***************************************************************************
	External functions
 ***************************************************************************/

/*
	ErrorMsg
*/
void __cdecl ErrorMsg(const char* fmt, ...)
{
	static FILE*	pFile = NULL;
	DWORD			dwWritten;
	char			buf[5000];
	char			buf2[5000];
	va_list 		va;

	va_start(va, fmt);

	vsprintf(buf, fmt, va);

	MessageBox(GetActiveWindow(), buf, MAME32NAME, MB_OK | MB_ICONERROR);

	strcpy(buf2, MAME32NAME ": ");
	strcat(buf2,buf);
	strcat(buf2, "\n");

	WriteFile(GetStdHandle(STD_OUTPUT_HANDLE), buf2, strlen(buf2), &dwWritten, NULL);

	if (pFile == NULL)
		pFile = fopen("debug.txt", "wt");

	if (pFile != NULL)
	{
		fprintf(pFile, "%s", buf2);
		fflush(pFile);
	}

	va_end(va);
}

void __cdecl dprintf(const char* fmt, ...)
{
	char	buf[5000];
	va_list va;

	va_start(va, fmt);

	vsnprintf(buf,sizeof(buf)-1,fmt,va);
	strcat(buf, "\n");

	OutputDebugString(buf);

	va_end(va);
}

UINT GetDepth(HWND hWnd)
{
	UINT	nBPP;
	HDC 	hDC;

	hDC = GetDC(hWnd);
	
	nBPP = GetDeviceCaps(hDC, BITSPIXEL) * GetDeviceCaps(hDC, PLANES);

	ReleaseDC(hWnd, hDC);

	return nBPP;
}

/*
 * Return TRUE if comctl32.dll is version 4.71 or greater
 * otherwise return FALSE.
 */
LONG GetCommonControlVersion()
{
	HMODULE hModule = GetModuleHandle("comctl32");

	if (hModule)
	{
		FARPROC lpfnICCE = GetProcAddress(hModule, "InitCommonControlsEx");

		if (lpfnICCE)
		{
			FARPROC lpfnDLLI = GetProcAddress(hModule, "DllInstall");
			DLLGETVERSIONPROC pDllGetVersion = (DLLGETVERSIONPROC)GetProcAddress(hModule, "DllGetVersion");

			if (lpfnDLLI || pDllGetVersion) 
			{
				/* comctl 4.71 or greater */

				// see if we can find out exactly
				
				/* Because some DLLs might not implement this function, you
				   must test for it explicitly. Depending on the particular 
				   DLL, the lack of a DllGetVersion function can be a useful
				   indicator of the version. */

				if(pDllGetVersion)
				{
					DLLVERSIONINFO dvi;
					HRESULT hr;

					ZeroMemory(&dvi, sizeof(dvi));
					dvi.cbSize = sizeof(dvi);

					hr = (*pDllGetVersion)(&dvi);

					if (SUCCEEDED(hr))
					{
						return PACKVERSION(dvi.dwMajorVersion, dvi.dwMinorVersion);
					}
				}
				return PACKVERSION(4,71);
			}
			return PACKVERSION(4,7);
		}
		return PACKVERSION(4,0);
	}
	/* DLL not found */
	return PACKVERSION(0,0);
}

void DisplayTextFile(HWND hWnd, const char *cName)
{
	HINSTANCE hErr;
	const char	  *msg = 0;

	hErr = ShellExecute(hWnd, NULL, cName, NULL, NULL, SW_SHOWNORMAL);
	if ((int)hErr > 32)
		return;

	switch((int)hErr)
	{
	case 0:
		msg = "The operating system is out of memory or resources.";
		break;

	case ERROR_FILE_NOT_FOUND:
		msg = "The specified file was not found."; 
		break;

	case SE_ERR_NOASSOC :
		msg = "There is no application associated with the given filename extension.";
		break;

	case SE_ERR_OOM :
		msg = "There was not enough memory to complete the operation.";
		break;

	case SE_ERR_PNF :
		msg = "The specified path was not found.";
		break;

	case SE_ERR_SHARE :
		msg = "A sharing violation occurred.";
		break;

	default:
		msg = "Unknown error.";
	}
 
	MessageBox(NULL, msg, cName, MB_OK); 
}

char* MyStrStrI(const char* pFirst, const char* pSrch)
{
	char* cp = (char*)pFirst;
	char* s1;
	char* s2;
	
	while (*cp)
	{
		s1 = cp;
		s2 = (char*)pSrch;
		
		while (*s1 && *s2 && !_strnicmp(s1, s2, 1))
			s1++, s2++;
		
		if (!*s2)
			return cp;
		
		cp++;
	}
	return NULL;
}

char * ConvertToWindowsNewlines(const char *source)
{
	static char buf[9000];
	char *dest;

	dest = buf;
	while (*source != 0)
	{
		if (*source == '\n')
		{
			*dest++ = '\r';
			*dest++ = '\n';
		}
		else
			*dest++ = *source;
		source++;
	}
	*dest = 0;
	return buf;
}

/* Lop off path and extention from a source file name
 * This assumes their is a pathname passed to the function
 * like src\drivers\blah.c
 */
const char * GetDriverFilename(int nIndex)
{
    static char tmp[40];
    char *ptmp;

	const char *s = drivers[nIndex]->source_file;

    tmp[0] = '\0';

	ptmp = strrchr(s, '\\');
	if (ptmp == NULL)
		ptmp = strrchr(s, '/');
    if (ptmp == NULL)
		return s;

	ptmp++;
	strcpy(tmp,ptmp);
	return tmp;
}

BOOL DriverIsClone(int driver_index)
{
	return (drivers[driver_index]->clone_of->flags & NOT_A_DRIVER) == 0;
}

BOOL DriverIsBroken(int driver_index)
{
	return (drivers[driver_index]->flags & (GAME_NOT_WORKING | GAME_UNEMULATED_PROTECTION)) != 0;
}

BOOL DriverIsHarddisk(int driver_index)
{
	const struct RomModule *region;

	const struct GameDriver *gamedrv = drivers[driver_index];

	for (region = rom_first_region(gamedrv); region; region = rom_next_region(region))
		if (ROMREGION_ISDISKDATA(region))
			return TRUE;

	return FALSE;	
}

BOOL DriverHasOptionalBIOS(int driver_index)
{
	const struct GameDriver *gamedrv = drivers[driver_index];

	return gamedrv->bios != NULL;
}

BOOL DriverIsStereo(int driver_index)
{
    struct InternalMachineDriver drv;
    expand_machine_driver(drivers[driver_index]->drv, &drv);
	return (drv.sound_attributes & SOUND_SUPPORTS_STEREO) != 0;
}

BOOL DriverIsVector(int driver_index)
{
    struct InternalMachineDriver drv;
    expand_machine_driver(drivers[driver_index]->drv, &drv);
	return (drv.video_attributes & VIDEO_TYPE_VECTOR) != 0;
}

BOOL DriverUsesRoms(int driver_index)
{
	const struct GameDriver *gamedrv = drivers[driver_index];
	const struct RomModule *region, *rom;

	for (region = rom_first_region(gamedrv); region; region = rom_next_region(region))
		for (rom = rom_first_file(region); rom; rom = rom_next_file(rom))
			return TRUE;
	return FALSE;
}

BOOL DriverUsesSamples(int driver_index)
{
#if (HAS_SAMPLES == 1) || (HAS_VLM5030 == 1)

	int i;
    struct InternalMachineDriver drv;

	expand_machine_driver(drivers[driver_index]->drv,&drv);

	for (i = 0; i < MAX_SOUND && drv.sound[i].sound_type ; i++)
	{
		const char **samplenames = NULL;

#if (HAS_SAMPLES == 1)
		if (drv.sound[i].sound_type == SOUND_SAMPLES)
			samplenames = ((struct Samplesinterface *)drv.sound[i].sound_interface)->samplenames;
#endif

        /*
#if (HAS_VLM5030 == 1)
		if (drv.sound[i].sound_type == SOUND_VLM5030)
			samplenames = ((struct VLM5030interface *)drv.sound[i].sound_interface)->samplenames;
#endif
        */
		if (samplenames != 0 && samplenames[0] != 0)
			return TRUE;
	}

#endif

	return FALSE;
}

BOOL DriverUsesTrackball(int driver_index)
{
    const struct InputPortTiny *input_ports;

	if (drivers[driver_index]->input_ports == NULL)
        return FALSE;

    input_ports = drivers[driver_index]->input_ports;

	while (1)
    {
        UINT32 type;

        type = input_ports->type;

        if (type == IPT_END)
            break;

        type &= ~IPF_MASK;
        
        if (type == IPT_DIAL || type == IPT_PADDLE || 
			type == IPT_TRACKBALL_X || type == IPT_TRACKBALL_Y ||
            type == IPT_AD_STICK_X || type == IPT_AD_STICK_Y)
        {
            return TRUE;
        }
        
        input_ports++;
    }

    return FALSE;
}

BOOL DriverUsesLightGun(int driver_index)
{
    const struct InputPortTiny *input_ports;

	if (drivers[driver_index]->input_ports == NULL)
        return FALSE;

    input_ports = drivers[driver_index]->input_ports;

	while (1)
    {
        UINT32 type;

        type = input_ports->type;

        if (type == IPT_END)
			break;

		type &= ~IPF_MASK;
        
		if (type == IPT_LIGHTGUN_X || type == IPT_LIGHTGUN_Y)
            return TRUE;
        
        input_ports++;
    }

    return FALSE;
}

void FlushFileCaches(void)
{
	unzip_cache_clear();
}

void FreeIfAllocated(char **s)
{
	if (*s)
		free(*s);
	*s = NULL;
}

BOOL StringIsSuffixedBy(const char *s, const char *suffix)
{
	return (strlen(s) > strlen(suffix)) && (strcmp(s + strlen(s) - strlen(suffix), suffix) == 0);
}

/***************************************************************************
	Internal functions
 ***************************************************************************/

