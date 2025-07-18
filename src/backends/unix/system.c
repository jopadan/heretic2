/*
 * Copyright (C) 1997-2001 Id Software, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 *
 * =======================================================================
 *
 * This file implements all system dependent generic functions.
 *
 * =======================================================================
 */

#include <dirent.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/select.h> /* for fd_set */
#ifndef FNDELAY
#define FNDELAY O_NDELAY
#endif

#ifdef __APPLE__
#include <mach/clock.h>
#include <mach/mach.h>
#endif

#include "../../common/header/common.h"
#include "../../common/header/glob.h"

// Pointer to game library
static void *game_library;

// Evil hack to determine if stdin is available
qboolean stdin_active = true;

// Terminal supports colors
static qboolean color_active = false;

// Config dir
char cfgdir[MAX_OSPATH] = CFGDIR;

// Console logfile
extern FILE	*logfile;

/* ================================================================ */

void
Sys_Error(const char *error, ...)
{
	va_list argptr;
	char string[1024];

	/* change stdin to non blocking */
	fcntl(0, F_SETFL, fcntl(0, F_GETFL, 0) & ~FNDELAY);

#ifndef DEDICATED_ONLY
	CL_Shutdown();
#endif
	Qcommon_Shutdown();

	va_start(argptr, error);
	vsnprintf(string, 1024, error, argptr);
	va_end(argptr);
	fprintf(stderr, "Error: %s\n", string);

	exit(1);
}

void
Sys_Quit(void)
{
#ifndef DEDICATED_ONLY
	CL_Shutdown();
#endif

	if (logfile)
	{
		fclose(logfile);
		logfile = NULL;
	}

	Qcommon_Shutdown();
	fcntl(0, F_SETFL, fcntl(0, F_GETFL, 0) & ~FNDELAY);

	printf("------------------------------------\n");

	exit(0);
}

void
Sys_Init(void)
{
	char *envvar;

	envvar = getenv("TERM");
	if (envvar && strstr(envvar, "color"))
	{
		char buf[256];

		color_active = true;

		snprintf(buf, sizeof(buf),
			"\2Terminal supports colors: TERM='%s'\n", envvar);

		Sys_ConsoleOutput(buf);
		return;
	}

	envvar = getenv("COLORTERM");
	if (envvar && strlen(envvar))
	{
		char buf[256];
		color_active = true;

		snprintf(buf, sizeof(buf),
			"\2Terminal supports colors: COLORTERM='%s'\n", envvar);

		Sys_ConsoleOutput(buf);
		return;
	}

	Sys_ConsoleOutput("Terminal has no colors support.\n");

}

/* ================================================================ */

char *
Sys_ConsoleInput(void)
{
	static char text[256];
	int len;
	fd_set fdset;
	struct timeval timeout;

	if (!dedicated || !dedicated->value)
	{
		return NULL;
	}

	if (!stdin_active)
	{
		return NULL;
	}

	FD_ZERO(&fdset);
	FD_SET(0, &fdset); /* stdin */
	timeout.tv_sec = 0;
	timeout.tv_usec = 0;

	if ((select(1, &fdset, NULL, NULL, &timeout) == -1) || !FD_ISSET(0, &fdset))
	{
		return NULL;
	}

	len = read(0, text, sizeof(text));

	if (len == 0)   /* eof! */
	{
		stdin_active = false;
		return NULL;
	}

	if (len < 1)
	{
		return NULL;
	}

	text[len - 1] = 0; /* rip off the /n and terminate */

	return text;
}

void
Sys_ConsoleOutput(char *string)
{
	if ((string[0] == 0x01) || (string[0] == 0x02))
	{
		if (color_active)
		{
			if (string[0] == 0x01)
			{
				/* red */
				fputs("\033[31;1m", stdout);
			}
			else
			{
				/* green */
				fputs("\033[32;1m", stdout);
			}

			fputs(string + 1, stdout);

			/* reset to default terminal settings */
			fputs("\033[0m", stdout);
			return;
		}
	}

	fputs(string, stdout);
}

/* ================================================================ */

long long
Sys_Microseconds(void)
{
#ifdef __APPLE__
	// OSX didn't have clock_gettime() until recently, so use Mach's clock_get_time()
	// instead. fortunately its mach_timespec_t seems identical to POSIX struct timespec
	// so lots of code can be shared
	clock_serv_t cclock;
	mach_timespec_t now;
	static mach_timespec_t first;

	host_get_clock_service(mach_host_self(), SYSTEM_CLOCK, &cclock);
	clock_get_time(cclock, &now);
	mach_port_deallocate(mach_task_self(), cclock);

#else // not __APPLE__ - other Unix-likes will hopefully support clock_gettime()

	struct timespec now;
	static struct timespec first;
#ifdef _POSIX_MONOTONIC_CLOCK
	clock_gettime(CLOCK_MONOTONIC, &now);
#else
	clock_gettime(CLOCK_REALTIME, &now);
#endif

#endif // not __APPLE__

	if(first.tv_sec == 0)
	{
		long long nsec = now.tv_nsec;
		long long sec = now.tv_sec;
		// set back first by 1ms so neither this function nor Sys_Milliseconds()
		// (which calls this) will ever return 0
		nsec -= 1000000;
		if(nsec < 0)
		{
			nsec += 1000000000ll; // 1s in ns => definitely positive now
			--sec;
		}

		first.tv_sec = sec;
		first.tv_nsec = nsec;
	}

	long long sec = now.tv_sec - first.tv_sec;
	long long nsec = now.tv_nsec - first.tv_nsec;

	if(nsec < 0)
	{
		nsec += 1000000000ll; // 1s in ns
		--sec;
	}

	return sec*1000000ll + nsec/1000ll;
}

int
Sys_Milliseconds(void)
{
	return (int)(Sys_Microseconds()/1000ll);
}

void
Sys_Nanosleep(int nanosec)
{
	struct timespec t = {0, nanosec};
	nanosleep(&t, NULL);
}

/* ================================================================ */

/* The musthave and canhave arguments are unused in YQ2. We
   can't remove them since Sys_FindFirst() and Sys_FindNext()
   are defined in shared.h and may be used in custom game DLLs. */

static char findbase[MAX_OSPATH];
static char findpath[MAX_OSPATH];
static char findpattern[MAX_OSPATH];
static DIR *fdir;

char *
Sys_FindFirst(const char *path, unsigned musthave, unsigned canhave)
{
	struct dirent *d;
	char *p;

	if (fdir)
	{
		Sys_Error("Sys_BeginFind without close");
	}

	Q_strlcpy(findbase, path, sizeof(findbase));

	if ((p = strrchr(findbase, '/')) != NULL)
	{
		*p = 0;
		Q_strlcpy(findpattern, p + 1, sizeof(findpattern));
	}
	else
	{
		strcpy(findpattern, "*");
	}

	if (strcmp(findpattern, "*.*") == 0)
	{
		strcpy(findpattern, "*");
	}

	if ((fdir = opendir(findbase)) == NULL)
	{
		return NULL;
	}

	while ((d = readdir(fdir)) != NULL)
	{
		if (!*findpattern || glob_match(findpattern, d->d_name))
		{
			if ((strcmp(d->d_name, ".") != 0) || (strcmp(d->d_name, "..") != 0))
			{
				snprintf(findpath, sizeof(findpath), "%s/%s", findbase, d->d_name);
				return findpath;
			}
		}
	}

	return NULL;
}

char *
Sys_FindNext(unsigned musthave, unsigned canhave)
{
	struct dirent *d;

	if (fdir == NULL)
	{
		return NULL;
	}

	while ((d = readdir(fdir)) != NULL)
	{
		if (!*findpattern || glob_match(findpattern, d->d_name))
		{
			if ((strcmp(d->d_name, ".") != 0) || (strcmp(d->d_name, "..") != 0))
			{
				snprintf(findpath, sizeof(findpath), "%s/%s", findbase, d->d_name);
				return findpath;
			}
		}
	}

	return NULL;
}

void
Sys_FindClose(void)
{
	if (fdir != NULL)
	{
		closedir(fdir);
	}

	fdir = NULL;
}

/* ================================================================ */

void
Sys_UnloadGame(void)
{
	if (game_library)
	{
		dlclose(game_library);
	}

	game_library = NULL;
}

void *
Sys_GetGameAPI(void *parms)
{
	typedef void *(*fnAPI)(void *);
	fnAPI GetGameAPI;

	char name[MAX_OSPATH];
	const char *path, *str_p;
#ifdef __APPLE__
	const char *gamename = "game.dylib";
#else
	const char *gamename = "game.so";
#endif

	if (game_library)
	{
		Com_Error(ERR_FATAL, "Sys_GetGameAPI without Sys_UnloadingGame");
	}

	Com_Printf("Loading library: %s\n", gamename);

	/* now run through the search paths */
	path = NULL;

	while (1)
	{
		FILE *fp;

		path = FS_NextPath(path);

		if (!path)
		{
			return NULL;     /* couldn't find one anywhere */
		}

		snprintf(name, MAX_OSPATH, "%s/%s", path, gamename);

		/* skip it if it just doesn't exist */
		fp = fopen(name, "rb");

		if (fp == NULL)
		{
			continue;
		}

		fclose(fp);

#ifdef USE_SANITIZER
		game_library = dlopen(name, RTLD_NOW | RTLD_NODELETE);
#else
		game_library = dlopen(name, RTLD_NOW);
#endif

		if (game_library)
		{
			Com_MDPrintf("Loading library: %s\n", name);
			break;
		}
		else
		{
			Com_Printf("Loading library: %s\n: ", name);

			path = (char *)dlerror();
			str_p = strchr(path, ':');   /* skip the path (already shown) */

			if (str_p == NULL)
			{
				str_p = path;
			}
			else
			{
				str_p++;
			}

			Com_Printf("%s\n", str_p);

			return NULL;
		}
	}

	GetGameAPI = (fnAPI)dlsym(game_library, "GetGameAPI");

	if (!GetGameAPI)
	{
		Sys_UnloadGame();
		return NULL;
	}

	return GetGameAPI(parms);
}

/* ================================================================ */

void
Sys_Mkdir(const char *path)
{
	if (!Sys_IsDir(path))
	{
		if (mkdir(path, 0755) != 0)
		{
			Com_Error(ERR_FATAL, "Couldn't create dir %s\n", path);
		}
	}
}

qboolean
Sys_IsDir(const char *path)
{
	struct stat sb;

	if (stat(path, &sb) != -1)
	{
		if (S_ISDIR(sb.st_mode))
		{
			return true;
		}
	}

	return false;
}

qboolean
Sys_IsFile(const char *path)
{
	struct stat sb;

	if (stat(path, &sb) != -1)
	{
		if (S_ISREG(sb.st_mode))
		{
			return true;
		}
	}

	return false;
}

char *
Sys_GetHomeDir(void)
{
	static char gdir[MAX_OSPATH];
	char *home;

	home = getenv("HOME");

	if (!home)
	{
		return NULL;
	}

#ifndef __HAIKU__
	snprintf(gdir, sizeof(gdir), "%s/%s/", home, cfgdir);
#else
	snprintf(gdir, sizeof(gdir), "%s/config/settings/%s", home, cfgdir);
#endif
	Sys_Mkdir(gdir);

	return gdir;
}

void
Sys_Remove(const char *path)
{
	if (remove(path) == -1 && errno != ENOENT)
	{
		Com_Printf("%s: remove %s: %s\n",
			__func__, path, strerror(errno));
	}
}

int
Sys_Rename(const char *from, const char *to)
{
	return rename(from, to);
}

void
Sys_RemoveDir(const char *path)
{
	char filepath[MAX_OSPATH];
	struct dirent *file;
	DIR *directory;

	if (Sys_IsDir(path))
	{
		directory = opendir(path);
		if (directory)
		{
			while ((file = readdir(directory)) != NULL)
			{
				snprintf(filepath, MAX_OSPATH, "%s/%s", path, file->d_name);
				Sys_Remove(filepath);
			}

			closedir(directory);
			Sys_Remove(path);
		}
	}
}

qboolean
Sys_Realpath(const char *in, char *out, size_t size)
{
	char *converted = realpath(in, NULL);

	if (converted == NULL)
	{
		Com_Printf("Couldn't get realpath for %s\n", in);
		return false;
	}

	Q_strlcpy(out, converted, size);
	free(converted);

	return true;
}

/* ================================================================ */

void *
Sys_GetProcAddress(void *handle, const char *sym)
{
	if (handle == NULL)
	{
#ifdef RTLD_DEFAULT
		return dlsym(RTLD_DEFAULT, sym);
#else
		/* POSIX suggests that this is a portable equivalent */
		static void *global_namespace = NULL;

		if (global_namespace == NULL)
			global_namespace = dlopen(NULL, RTLD_GLOBAL|RTLD_LAZY);

		return dlsym(global_namespace, sym);
#endif
	}
	return dlsym(handle, sym);
}

void
Sys_FreeLibrary(void *handle)
{
	if (handle && dlclose(handle))
	{
		Com_Error(ERR_FATAL, "dlclose failed on %p: %s", handle, dlerror());
	}
}

void *
Sys_LoadLibrary(const char *path, const char *sym, void **handle)
{
	void *module, *entry;

	*handle = NULL;

#ifdef USE_SANITIZER
	module = dlopen(path, RTLD_LAZY | RTLD_NODELETE);
#else
	module = dlopen(path, RTLD_LAZY);
#endif

	if (!module)
	{
		Com_Printf("%s failed: %s\n", __func__, dlerror());
		return NULL;
	}

	if (sym)
	{
		entry = dlsym(module, sym);

		if (!entry)
		{
			Com_Printf("%s failed: %s\n", __func__, dlerror());
			dlclose(module);
			return NULL;
		}
	}
	else
	{
		entry = NULL;
	}

	Com_DPrintf("%s succeeded: %s\n", __func__, path);

	*handle = module;

	return entry;
}

/* ================================================================ */

void
Sys_GetWorkDir(char *buffer, size_t len)
{
	if (getcwd(buffer, len) != 0)
	{
		return;
	}

	buffer[0] = '\0';
}

qboolean
Sys_SetWorkDir(char *path)
{
	if (chdir(path) == 0)
	{
		return true;
	}

	return false;
}
