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
 * Server startup.
 *
 * =======================================================================
 */

#include "header/server.h"

server_static_t svs; /* persistant server info */
server_t sv; /* local server */

static int
SV_FindIndex(const char *name, int start, int max, qboolean create)
{
	int i, protocol;

	if (!name || !name[0])
	{
		return 0;
	}

	protocol = sv_client ? sv_client->protocol : PROTOCOL_VERSION;

	for (i = 1; i < max && sv.configstrings[start + i][0]; i++)
	{
		if (!strcmp(sv.configstrings[start + i], name))
		{
			return i;
		}
	}

	if (!create)
	{
		return 0;
	}

	if (i == max)
	{
		Com_Error(ERR_DROP, "*Index: overflow");
	}

	Q_strlcpy(sv.configstrings[start + i], name, sizeof(sv.configstrings[start + i]));

	if (sv.state != ss_loading)
	{
		/* send the update to everyone */
		MSG_WriteChar(&sv.multicast, svc_configstring);
		/* i in native server range */
		MSG_WriteConfigString(&sv.multicast,
			P_ConvertConfigStringTo(start + i, protocol), name);
		SV_Multicast(vec3_origin, MULTICAST_ALL_R);
	}

	return i;
}

int
SV_ModelIndex(const char *name)
{
	return SV_FindIndex(name, CS_MODELS, MAX_MODELS, true);
}

int
SV_SoundIndex(const char *name)
{
	return SV_FindIndex(name, CS_SOUNDS, MAX_SOUNDS, true);
}

int
SV_ImageIndex(const char *name)
{
	return SV_FindIndex(name, CS_IMAGES, MAX_IMAGES, true);
}

/*
 * Combine entity_state and entity_rrstate_t
 */
void
SV_GetEntityState(const edict_t *svent, entity_xstate_t *state)
{
	memcpy(state, &svent->s, sizeof(entity_state_t));
	memcpy((byte *)state + sizeof(entity_state_t),
		&svent->rrs, sizeof(entity_rrstate_t));
}

/*
 * Entity baselines are used to compress the update messages
 * to the clients -- only the fields that differ from the
 * baseline will be transmitted
 */
static void
SV_CreateBaseline(void)
{
	edict_t *svent;
	int entnum;

	for (entnum = 1; entnum < ge->num_edicts; entnum++)
	{
		svent = EDICT_NUM(entnum);

		if (!svent->inuse)
		{
			continue;
		}

		if (!svent->s.modelindex && !svent->s.sound && !svent->s.effects)
		{
			continue;
		}

		svent->s.number = entnum;

		/* take current state as baseline */
		VectorCopy(svent->s.origin, svent->s.old_origin);
		if (entnum >= MAX_EDICTS)
		{
			Com_Error(ERR_DROP, "%s: bad entity %d >= %d\n",
				__func__, entnum, MAX_EDICTS);
		}

		SV_GetEntityState(svent, &sv.baselines[entnum]);
	}
}

static void
SV_CheckForSavegame(qboolean isautosave)
{
	char name[MAX_OSPATH];
	char savename[MAX_OSPATH];
	FILE *f;
	int i;

	Com_DPrintf("%s()\n", __func__);

	if (sv_noreload->value)
	{
		return;
	}

	if (Cvar_VariableValue("deathmatch"))
	{
		return;
	}

	Q_strlcpy(savename, sv.name, sizeof(savename));
	SV_CleanLevelFileName(savename);

	Com_sprintf(name, sizeof(name), "%s/save/current/%s.sav",
			FS_Gamedir(), savename);
	f = Q_fopen(name, "rb");

	if (!f)
	{
		return; /* no savegame */
	}

	fclose(f);

	SV_ClearWorld();

	/* get configstrings and areaportals */
	SV_ReadLevelFile();

	if (!sv.loadgame || (sv.loadgame && isautosave))
	{
		/* coming back to a level after being in a different
		   level, so run it for ten seconds */
		server_state_t previousState;

		previousState = sv.state;
		sv.state = ss_loading;

		for (i = 0; i < 100; i++)
		{
			ge->RunFrame();
		}

		sv.state = previousState;
	}
}

/*
 * Change the server to a new map, taking all connected
 * clients along with it.
 */
static void
SV_SpawnServer(char *server, char *spawnpoint, server_state_t serverstate,
		qboolean attractloop, qboolean loadgame, qboolean isautosave)
{
	const char *entity_orig;
	unsigned checksum;
	char *entity;
	int i, entitysize;

	if (attractloop)
	{
		Cvar_Set("paused", "0");
	}

	Com_Printf("------- server initialization ------\n");
	Com_DPrintf("SpawnServer: %s\n", server);

	if (sv.demofile)
	{
		FS_FCloseFile(sv.demofile);
	}

	svs.spawncount++; /* any partially connected client will be restarted */
	sv.state = ss_dead;
	Com_SetServerState(sv.state);

	/* wipe the entire per-level structure */
	memset(&sv, 0, sizeof(sv));
	svs.realtime = 0;
	sv.loadgame = loadgame;
	sv.attractloop = attractloop;

	/* save name for levels that don't set message */
	Q_strlcpy(sv.configstrings[CS_NAME], server, sizeof(sv.configstrings[CS_NAME]));

	if (Cvar_VariableValue("deathmatch"))
	{
		sprintf(sv.configstrings[CS_AIRACCEL], "%g", sv_airaccelerate->value);
		pm_airaccelerate = sv_airaccelerate->value;
	}
	else
	{
		strcpy(sv.configstrings[CS_AIRACCEL], "0");
		pm_airaccelerate = 0;
	}

	SZ_Init(&sv.multicast, sv.multicast_buf, sizeof(sv.multicast_buf));

	Q_strlcpy(sv.name, server, sizeof(sv.name));

	/* leave slots at start for clients only */
	for (i = 0; i < maxclients->value; i++)
	{
		/* needs to reconnect */
		if (svs.clients[i].state > cs_connected)
		{
			svs.clients[i].state = cs_connected;
		}

		svs.clients[i].lastframe = -1;
	}

	sv.time = 1000;

	Q_strlcpy(sv.name, server, sizeof(sv.name));
	Q_strlcpy(sv.configstrings[CS_NAME], server,
		sizeof(sv.configstrings[CS_NAME]));

	SV_ClearPersistantEffects();

	if (serverstate != ss_game)
	{
		sv.models[1] = CM_LoadMap("", false, &checksum); /* no real map */
	}
	else
	{
		Com_sprintf(sv.configstrings[CS_MODELS + 1],
				sizeof(sv.configstrings[CS_MODELS + 1]), "maps/%s.bsp", server);
		sv.models[1] = CM_LoadMap(sv.configstrings[CS_MODELS + 1],
				false, &checksum);
	}

	Com_sprintf(sv.configstrings[CS_MAPCHECKSUM],
			sizeof(sv.configstrings[CS_MAPCHECKSUM]),
			"%i", checksum);

	/* clear physics interaction links */
	SV_ClearWorld();

	for (i = 1; i < CM_NumInlineModels(); i++)
	{
		Com_sprintf(sv.configstrings[CS_MODELS + 1 + i],
				sizeof(sv.configstrings[CS_MODELS + 1 + i]),
				"*%i", i);
		sv.models[i + 1] = CM_InlineModel(sv.configstrings[CS_MODELS + 1 + i]);
	}

	/* spawn the rest of the entities on the map */
	sv.state = ss_loading;
	Com_SetServerState(sv.state);

	/* copy original entities string */
	entity_orig = CM_EntityString(&entitysize);
	if (entitysize < 0)
	{
		entitysize = 0;
	}
	entity = malloc(entitysize + 1);
	if (entitysize)
	{
		memcpy(entity, entity_orig, entitysize);
	}
	entity[entitysize] = 0; /* jit entity bug - null terminate the entity string! */
	/* load and spawn all other entities */
	ge->SpawnEntities(sv.name, entity, spawnpoint);
	free(entity);

	/* verify game didn't clobber important stuff */
	if ((int)checksum !=
		(int)strtol(sv.configstrings[CS_MAPCHECKSUM], (char **)NULL, 10))
	{
		Com_Error(ERR_DROP, "Game DLL corrupted server configstrings");
	}

	/* all precaches are complete */
	sv.state = serverstate;
	Com_SetServerState(sv.state);

	/* create a baseline for more efficient communications */
	SV_CreateBaseline();

	/* run two frames to allow everything to settle */
	ge->RunFrame();
	ge->RunFrame();

	/* check for a savegame */
	SV_CheckForSavegame(isautosave);

	/* set serverinfo variable */
	Cvar_FullSet("mapname", sv.name, CVAR_SERVERINFO | CVAR_NOSET);

	Com_Printf("------------------------------------\n\n");
}

/*
 * A brand new game has been started
 */
static void
SV_ClearGamemodeCvar(char *name, char *msg, int flags)
{
	Cvar_FullSet(name, "0", flags);

	strcat(msg, name);
	strcat(msg, " ");
}

static int
SV_ChooseGamemode(void)
{
	char msg[32], *choice;
	int gamemode;

	*msg = 0;

	if (Cvar_VariableValue("deathmatch"))
	{
		if (Cvar_VariableValue("coop"))
		{
			SV_ClearGamemodeCvar("coop", msg, CVAR_SERVERINFO | CVAR_LATCH);
		}

		if (Cvar_VariableValue("singleplayer"))
		{
			SV_ClearGamemodeCvar("singleplayer", msg, 0);
		}

		choice = "deathmatch";
		gamemode = GAMEMODE_DM;
	}
	else if (Cvar_VariableValue("coop"))
	{
		if (Cvar_VariableValue("singleplayer"))
		{
			SV_ClearGamemodeCvar("singleplayer", msg, 0);
		}

		choice = "coop";
		gamemode = GAMEMODE_COOP;
	}
	else
	{
		if (dedicated->value && !Cvar_VariableValue("singleplayer"))
		{
			Cvar_FullSet("deathmatch", "1", CVAR_SERVERINFO | CVAR_LATCH);

			choice = "deathmatch";
			gamemode = GAMEMODE_DM;
		}
		else
		{
			Cvar_FullSet("singleplayer", "1", CVAR_SERVERINFO | CVAR_LATCH);

			choice = "singleplayer";
			gamemode = GAMEMODE_SP;
		}
	}

	if (*msg)
	{
		Com_Printf("Gamemode ambiguity: Chose: %s, ignored: %s\n", choice, msg);
	}

	return gamemode;
}

void
SV_InitGame(void)
{
	int i, gamemode;
	edict_t *ent;
	char idmaster[32];
	/* TODO: Move effects load to game code */
	void *E_Load(void);

	if (svs.initialized)
	{
		/* cause any connected clients to reconnect */
		SV_Shutdown("Server restarted\n", true);
	}

#ifndef DEDICATED_ONLY
	else
	{
		/* make sure the client is down */
		CL_Drop();
		SCR_BeginLoadingPlaque();
	}
#endif

	/* get any latched variable changes (maxclients, etc) */
	Cvar_GetLatchedVars();

	svs.initialized = true;

	gamemode = SV_ChooseGamemode();

	/* init clients */
	if (gamemode == GAMEMODE_DM)
	{
		if (maxclients->value <= 1)
		{
			Cvar_FullSet("maxclients", "8", CVAR_SERVERINFO | CVAR_LATCH);
		}
		else if (maxclients->value > MAX_CLIENTS)
		{
			Cvar_FullSet("maxclients", va("%i", MAX_CLIENTS), CVAR_SERVERINFO | CVAR_LATCH);
		}
	}
	else if (gamemode == GAMEMODE_COOP)
	{
		if ((maxclients->value <= 1) || (maxclients->value > 4))
		{
			Cvar_FullSet("maxclients", "4", CVAR_SERVERINFO | CVAR_LATCH);
		}
	}
	else /* non-deathmatch, non-coop is one player */
	{
		Cvar_FullSet("maxclients", "1", CVAR_SERVERINFO | CVAR_LATCH);
	}

	svs.gamemode = gamemode;
	svs.spawncount = randk();
	svs.clients = Z_Malloc(sizeof(client_t) * maxclients->value);
	svs.num_client_entities = maxclients->value * UPDATE_BACKUP * 64;
	svs.client_entities = Z_Malloc( sizeof(entity_xstate_t) * svs.num_client_entities);

	/* init network stuff */
	if (dedicated->value)
	{
		if (gamemode == GAMEMODE_SP)
		{
			NET_Config(true);
		}
		else
		{
			NET_Config((maxclients->value > 1));
		}
	}
	else
	{
		NET_Config((maxclients->value > 1));
	}

	/* heartbeats will always be sent to the id master */
	svs.last_heartbeat = -99999; /* send immediately */
	Com_sprintf(idmaster, sizeof(idmaster), "192.246.40.37:%i", PORT_MASTER);
	NET_StringToAdr(idmaster, &master_adr[0]);

#ifndef DEDICATED_ONLY
	if (!E_Load())
	{
		Sys_Error("Unable to effects library");
	}
#endif

	/* init game */
	SV_InitGameProgs();

	for (i = 0; i < maxclients->value; i++)
	{
		ent = EDICT_NUM(i + 1);
		ent->s.number = i + 1;
		svs.clients[i].edict = ent;
		memset(&svs.clients[i].lastcmd, 0, sizeof(svs.clients[i].lastcmd));
	}
}

/*
 * the full syntax is:
 *
 * map [*]<map>$<startspot>+<nextserver>
 *
 * command from the console or progs.
 * Map can also be a.cin, .pcx, or .dm2 file
 * Nextserver is used to allow a cinematic to play, then proceed to
 * another level:
 *
 *  map tram.cin+jail_e3
 */
void
SV_Map(qboolean attractloop, char *levelstring, qboolean loadgame, qboolean isautosave)
{
	char level[MAX_QPATH];
	char *ch;
	size_t l;
	char spawnpoint[MAX_QPATH];
	const char *ext;

	sv.loadgame = loadgame;
	sv.attractloop = attractloop;

	if ((sv.state == ss_dead) && !sv.loadgame)
	{
		SV_InitGame(); /* the game is just starting */
	}

	Q_strlcpy(level, levelstring, sizeof(level));

	/* if there is a + in the map, set nextserver to the remainder */
	ch = strstr(level, "+");
	/* Loki Games Heretic 2 hack */
	if (!ch && strstr(level, "@"))
	{
		ch = strstr(level, "@");
	}

	if (ch)
	{
		*ch = 0;
		Cvar_Set("nextserver", va("gamemap \"%s\"", ch + 1));
	}
	else
	{
		// use next demo command if list of map commands as empty
		Cvar_Set("nextserver", (char*)Cvar_VariableString("nextdemo"));
		// and cleanup nextdemo
		Cvar_Set("nextdemo", "");
	}

	/* hack for end game screen in coop mode */
	if (Cvar_VariableValue("coop") && !Q_stricmp(level, "victory.pcx"))
	{
		Cvar_Set("nextserver", "gamemap \"*base1\"");
	}

	/* if there is a $, use the remainder as a spawnpoint */
	ch = strstr(level, "$");

	if (ch)
	{
		*ch = 0;
		Q_strlcpy(spawnpoint, ch + 1, sizeof(spawnpoint));
	}
	else
	{
		spawnpoint[0] = 0;
	}

	/* skip the end-of-unit flag if necessary */
	l = strlen(level);

	if (level[0] == '*')
	{
		memmove(level, level + 1, l);
		--l;
	}

	ext = (l <= 4) ? NULL : level + l - 4;

	if (ext && (!strcmp(ext, ".cin") ||
				!strcmp(ext, ".ogv") ||
				!strcmp(ext, ".avi") ||
				!strcmp(ext, ".roq") ||
				!strcmp(ext, ".mpg") ||
				!strcmp(ext, ".smk")))
	{
#ifndef DEDICATED_ONLY
		SCR_BeginLoadingPlaque(); /* for local system */
#endif
		SV_BroadcastCommand("changing\n");
		SV_SpawnServer(level, spawnpoint, ss_cinematic, attractloop, loadgame, isautosave);
	}
	else if (ext && !strcmp(ext, ".dm2"))
	{
#ifndef DEDICATED_ONLY
		SCR_BeginLoadingPlaque(); /* for local system */
#endif
		SV_BroadcastCommand("changing\n");
		SV_SpawnServer(level, spawnpoint, ss_demo, attractloop, loadgame, isautosave);
	}
	else if (ext && (!strcmp(ext, ".pcx") ||
					!strcmp(ext, ".lmp") ||
					!strcmp(ext, ".tga") ||
					!strcmp(ext, ".jpg") ||
					!strcmp(ext, ".png")))
	{
#ifndef DEDICATED_ONLY
		SCR_BeginLoadingPlaque(); /* for local system */
#endif
		SV_BroadcastCommand("changing\n");
		SV_SpawnServer(level, spawnpoint, ss_pic, attractloop, loadgame, isautosave);
	}
	else
	{
#ifndef DEDICATED_ONLY
		SCR_BeginLoadingPlaque(); /* for local system */
#endif
		SV_BroadcastCommand("changing\n");

		/* for some reason calling send messages here causes a lengthy reconnect delay */
		if (!(SV_Optimizations() & OPTIMIZE_RECONNECT))
		{
			SV_SendClientMessages();
			SV_SendPrepClientMessages();
		}

		SV_SpawnServer(level, spawnpoint, ss_game, attractloop, loadgame, isautosave);
		Cbuf_CopyToDefer();
	}

	SV_BroadcastCommand("reconnect\n");
}

