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
 * Interface between the server and the game module.
 *
 * =======================================================================
 */

#include "header/server.h"

#ifndef DEDICATED_ONLY
void SCR_DebugGraph(float value, int color);
#endif

game_export_t *ge;

/*
 * Sends the contents of the mutlicast buffer to a single client
 */
static void
PF_Unicast(edict_t *ent, qboolean reliable)
{
	int p;
	client_t *client;

	if (!ent)
	{
		return;
	}

	p = NUM_FOR_EDICT(ent);

	if ((p < 1) || (p > maxclients->value))
	{
		return;
	}

	client = svs.clients + (p - 1);

	if (reliable)
	{
		SZ_Write(&client->netchan.message, sv.multicast.data,
				sv.multicast.cursize);
	}
	else
	{
		SZ_Write(&client->datagram, sv.multicast.data, sv.multicast.cursize);
	}

	SZ_Clear(&sv.multicast);
}

/*
 * Debug print to server console
 */
static void
PF_dprintf(const char *fmt, ...)
{
	char msg[1024];
	va_list argptr;

	va_start(argptr, fmt);
	vsnprintf(msg, sizeof(msg), fmt, argptr);
	va_end(argptr);

	Com_Printf("%s", msg);
}

/*
 * Print to a single client
 */
static void
PF_cprintf(edict_t *ent, int level, const char *fmt, ...)
{
	char msg[1024];
	va_list argptr;
	int n;

	n = 0;

	if (ent)
	{
		n = NUM_FOR_EDICT(ent);

		if ((n < 1) || (n > maxclients->value))
		{
			Com_Error(ERR_DROP, "cprintf to a non-client");
		}
	}

	va_start(argptr, fmt);
	vsnprintf(msg, sizeof(msg), fmt, argptr);
	va_end(argptr);

	if (ent)
	{
		SV_ClientPrintf(svs.clients + (n - 1), level, "%s", msg);
	}
	else
	{
		Com_Printf("%s", msg);
	}
}

/*
 * centerprint to a single client
 */
static void
PF_centerprintf(edict_t *ent, const char *fmt, ...)
{
	char msg[1024];
	va_list argptr;
	int n;

	n = NUM_FOR_EDICT(ent);

	if ((n < 1) || (n > maxclients->value))
	{
		return;
	}

	va_start(argptr, fmt);
	vsnprintf(msg, sizeof(msg), fmt, argptr);
	va_end(argptr);

	MSG_WriteByte(&sv.multicast, svc_centerprint);
	MSG_WriteString(&sv.multicast, msg);
	PF_Unicast(ent, true);
}

/*
 * Abort the server with a game error
 */
YQ2_ATTR_NORETURN_FUNCPTR static void
PF_error(const char *fmt, ...)
{
	char msg[1024];
	va_list argptr;

	va_start(argptr, fmt);
	vsnprintf(msg, sizeof(msg), fmt, argptr);
	va_end(argptr);

	Com_Error(ERR_DROP, "Game Error: %s", msg);
}

/*
 * Also sets mins and maxs for inline bmodels
 */
static void
PF_setmodel(edict_t *ent, const char *name)
{
	int i;

	if (!name)
	{
		Com_Printf("%s: Name is NULL\n", __func__);
		return;
	}

	i = SV_ModelIndex(name);

	ent->s.modelindex = i;

	/* if it is an inline model, get
	   the size information for it */
	if (name[0] == '*')
	{
		cmodel_t *mod;

		mod = CM_InlineModel(name);
		VectorCopy(mod->mins, ent->mins);
		VectorCopy(mod->maxs, ent->maxs);
		SV_LinkEdict(ent);
	}
}

/* Direct set value for config string, index in library range */
static void
PF_Configstring(int index, const char *val)
{
	int internal_index;

	if (!val)
	{
		val = "";
	}

	internal_index = P_ConvertConfigStringFrom(index, SV_GetRecomendedProtocol());

	if ((internal_index < 0) || (internal_index >= MAX_CONFIGSTRINGS))
	{
		Com_Error(ERR_DROP, "configstring: bad index %i\n", internal_index);
	}

	/* change the string in sv */
	strcpy(sv.configstrings[internal_index], val);

	if (sv.state != ss_loading)
	{
		/* send the update to everyone */
		SZ_Clear(&sv.multicast);
		MSG_WriteChar(&sv.multicast, svc_configstring);
		/* index in protocol range */
		MSG_WriteConfigString(&sv.multicast, index, val);

		SV_Multicast(vec3_origin, MULTICAST_ALL_R);
	}
}

/* Direct get value for config string, index in library range */
static const char *
PF_ConfigStringGet(int index)
{
	index = P_ConvertConfigStringFrom(index, SV_GetRecomendedProtocol());

	if ((index < 0) || (index >= MAX_CONFIGSTRINGS))
	{
		Com_Error(ERR_DROP, "configstring: bad index %i\n", index);
	}

	/* change the string in sv */
	return sv.configstrings[index];
}

static void
PF_GetModelFrameInfo(int index, int num, float *mins, float *maxs)
{
	if (index < MAX_MODELS)
	{
		if (sv.configstrings[CS_MODELS + index][0] == '*')
		{
			if (maxs && mins)
			{
				cmodel_t *mod;

				mod = CM_InlineModel(sv.configstrings[CS_MODELS + index]);
				VectorCopy(mod->mins, mins);
				VectorCopy(mod->maxs, maxs);
			}
		}
		else
		{
			Mod_GetModelFrameInfo(sv.configstrings[CS_MODELS + index],
				num, mins, maxs);
		}
	}
}

static const dmdxframegroup_t *
PF_GetModelInfo(int index, int *num, float *mins, float *maxs)
{
	if (index < MAX_MODELS)
	{
		if (sv.configstrings[CS_MODELS + index][0] == '*')
		{
			if (maxs && mins)
			{
				cmodel_t *mod;

				mod = CM_InlineModel(sv.configstrings[CS_MODELS + index]);
				VectorCopy(mod->mins, mins);
				VectorCopy(mod->maxs, maxs);
			}
		}
		else
		{
			return Mod_GetModelInfo(sv.configstrings[CS_MODELS + index],
				num, mins, maxs);
		}
	}

	if (num)
	{
		*num = 0;
	}

	return NULL;
}

static void
PF_WriteChar(int c)
{
	MSG_WriteChar(&sv.multicast, c);
}

static void
PF_WriteByte(int c)
{
	MSG_WriteByte(&sv.multicast, c);
}

static void
PF_WriteShort(int c)
{
	MSG_WriteShort(&sv.multicast, c);
}

static void
PF_WriteLong(int c)
{
	MSG_WriteLong(&sv.multicast, c);
}

static void
PF_WriteFloat(float f)
{
	MSG_WriteFloat(&sv.multicast, f);
}

static void
PF_WriteString(const char *s)
{
	MSG_WriteString(&sv.multicast, s);
}

static void
PF_WritePos(const vec3_t pos)
{
	MSG_WritePos(&sv.multicast, pos, SV_GetRecomendedProtocol());
}

static void
PF_WriteDir(const vec3_t dir)
{
	MSG_WriteDir(&sv.multicast, dir);
}

static void
PF_WriteAngle(float f)
{
	MSG_WriteAngle(&sv.multicast, f);
}

/*
 * Also checks portalareas so that doors block sight
 */
static qboolean
PF_inPVS(vec3_t p1, vec3_t p2)
{
	int leafnum;
	int cluster;
	int area1, area2;
	byte *mask;

	leafnum = CM_PointLeafnum(p1);
	cluster = CM_LeafCluster(leafnum);
	area1 = CM_LeafArea(leafnum);
	mask = CM_ClusterPVS(cluster);

	leafnum = CM_PointLeafnum(p2);
	cluster = CM_LeafCluster(leafnum);
	area2 = CM_LeafArea(leafnum);

	// cluster -1 means "not in a visible leaf" or something like that (void?)
	// so p1 and p2 probably don't "see" each other.
	// either way, we must avoid using a negative index into mask[]!
	if (cluster < 0 || (!(mask[cluster >> 3] & (1 << (cluster & 7)))))
	{
		return false;
	}

	if (!CM_AreasConnected(area1, area2))
	{
		return false; /* a door blocks sight */
	}

	return true;
}

/*
 * Also checks portalareas so that doors block sound
 */
static qboolean
PF_inPHS(vec3_t p1, vec3_t p2)
{
	int leafnum;
	int cluster;
	int area1, area2;
	byte *mask;

	leafnum = CM_PointLeafnum(p1);
	cluster = CM_LeafCluster(leafnum);
	area1 = CM_LeafArea(leafnum);
	mask = CM_ClusterPHS(cluster);

	leafnum = CM_PointLeafnum(p2);
	cluster = CM_LeafCluster(leafnum);
	area2 = CM_LeafArea(leafnum);

	// cluster -1 means "not in a visible leaf" or something like that (void?)
	// so p1 and p2 probably don't "hear" each other.
	// either way, we must avoid using a negative index into mask[]!
	if (cluster < 0 || (!(mask[cluster >> 3] & (1 << (cluster & 7)))))
	{
		return false; /* more than one bounce away */
	}

	if (!CM_AreasConnected(area1, area2))
	{
		return false; /* a door blocks hearing */
	}

	return true;
}

static void
PF_StartSound(edict_t *entity, int channel, int sound_num,
		float volume, float attenuation, float timeofs)
{
	if (!entity)
	{
		return;
	}

	SV_StartSound(NULL, entity, channel, sound_num,
			volume, attenuation, timeofs);
}

/*
 * Called when either the entire server is being killed, or
 * it is changing to a different game directory.
 */
void
SV_ShutdownGameProgs(void)
{
	if (!ge)
	{
		return;
	}

	ge->Shutdown();
	Sys_UnloadGame();
	ge = NULL;
}

#include "../game/common/fx.h"
int sv_numeffects = 0;
PerEffectsBuffer_t SV_Persistant_Effects[MAX_PERSISTANT_EFFECTS];

game_export_t* GetGameAPI(game_import_t* import);

void MSG_WriteData(sizebuf_t* sb, byte* data, int len);


static void
SV_CreateEffectEvent(byte EventId, edict_t* ent, int type, int flags, vec3_t origin, char* format, ...)
{
	if (developer && developer->value)
	{
		Com_Printf("%s: TODO: Unimplemented\n", __func__);
	}
}

static void
SV_RemoveEffectsEvent(byte EventId, edict_t* ent, int type)
{
	if (developer && developer->value)
	{
		Com_Printf("%s: TODO: Unimplemented\n", __func__);
	}
}

qboolean SV_RemovePersistantEffect(int toRemove, int call_from)
{
	if (developer && developer->value)
	{
		Com_Printf("%s: TODO: Unimplemented\n", __func__);
	}
	return false;
}

static void
SV_RemoveEffects(edict_t* ent, int type)
{
	ent->s.clientEffects.numEffects = 0;
}

void
SV_WriteEffectToBuffer(sizebuf_t* msg, char* format, va_list args)
{
	int len = strlen(format);
	for (int i = 0; i < len; i++)
	{
		switch (format[i])
		{
		case 'b':
			MSG_WriteByte(msg, va_arg(args, int));
			break;
		case 'd':
			MSG_WriteDir(msg, va_arg(args, float *));
			break;
		case 'f':
			MSG_WriteFloat(msg, va_arg(args, double));
			break;
		case 'i':
			MSG_WriteLong(msg, va_arg(args, long));
			break;
		case 'p':
		case 'v':
			MSG_WritePos(msg, va_arg(args, float*), SV_GetRecomendedProtocol());
			break;
		case 's':
			MSG_WriteShort(msg, va_arg(args, int));
			break;
		case 't':
		case 'u':
		case 'x':
			MSG_WritePos(msg, va_arg(args, float*), SV_GetRecomendedProtocol());
			break;
		default:
			break;
		}
	}
}

void
SV_WriteClientEffectsToClient(client_frame_t* from, client_frame_t* to, sizebuf_t* msg)
{
	int numUpdatedEffects = 0;

	MSG_WriteByte(msg, svc_client_effect);

	for (int i = 0; i < MAX_PERSISTANT_EFFECTS; i++)
	{
		if (SV_Persistant_Effects[i].inUse && SV_Persistant_Effects[i].needsUpdate && SV_Persistant_Effects[i].entity == NULL)
		{
			numUpdatedEffects++;
		}
	}

	MSG_WriteByte(msg, numUpdatedEffects);

	for (int i = 0; i < MAX_PERSISTANT_EFFECTS; i++)
	{
		if (SV_Persistant_Effects[i].inUse && SV_Persistant_Effects[i].needsUpdate && SV_Persistant_Effects[i].entity == NULL)
		{
			MSG_WriteData(msg, SV_Persistant_Effects[i].buf, SV_Persistant_Effects[i].data_size);
			SV_Persistant_Effects[i].needsUpdate = false;

			if (SV_Persistant_Effects[i].nonPersistant)
			{
				SV_Persistant_Effects[i].nonPersistant = false;
				SV_Persistant_Effects[i].inUse = false;
			}
		}
	}
}

int
SV_CreatePersistantEffect(edict_t* ent, int type, int flags, vec3_t origin, char* format, ...)
{
	int enta;
	int effectID = -1;
	va_list args;
	sizebuf_t msg;
	int testflags;

	va_start(args, format);

	if (ent)
	{
		enta = ent->s.number;
	}
	else {
		enta = -1;
	}

	if (sv_numeffects >= MAX_PERSISTANT_EFFECTS)
	{
		Com_Printf("Warning : Unable to create persistant effect\n");
		va_end(args);
		return -1;
	}

	for (int i = 0; i < MAX_PERSISTANT_EFFECTS; i++)
	{
		if (SV_Persistant_Effects[i].inUse == false)
		{
			effectID = i;
			break;
		}
	}

	if (effectID == -1)
	{
		va_end(args);
		return -1;
	}

	if (type == FX_FIRE)
	{
		ent = NULL;
		flags &= CEF_OWNERS_ORIGIN;
	}

	PerEffectsBuffer_t* effect = &SV_Persistant_Effects[effectID];
	effect->freeBlock = 0;
	effect->bufSize = 192;
	effect->numEffects = 1;
	effect->fx_num = type;
	effect->demo_send_mask = -1;
	effect->send_mask = 0;
	effect->needsUpdate = true;
	effect->entity = ent;
	effect->inUse = true;
	effect->nonPersistant = false;

	SZ_Init(&msg, effect->buf, sizeof(effect->buf));
	MSG_WriteShort(&msg, type);

	if (ent != NULL)
	{
		ent->s.clientEffects.buf = &effect->buf[0];
		ent->s.clientEffects.bufSize = sizeof(effect->buf);
		ent->s.clientEffects.numEffects = 1;

		if (type == (int)FX_MAGIC_PORTAL)
		{
			ent->s.clientEffects.isPersistant = true;
		}
		else
		{
			ent->s.clientEffects.isPersistant = false;
		}
	}

	testflags = flags;

	MSG_WriteShort(&msg, testflags);

	if ((testflags & CEF_BROADCAST) != 0 && enta >= 0)
	{
		MSG_WriteShort(&msg, enta);
	}

	if ((testflags & CEF_OWNERS_ORIGIN) == 0)
	{
		MSG_WritePos(&msg, origin, SV_GetRecomendedProtocol());
	}

	MSG_WriteByte(&msg, 0x3a);

	if (format && format[0])
	{
		SV_WriteEffectToBuffer(&msg, format, args);
	}

	effect->data_size = msg.cursize;
	va_end(args);

	return effectID;
}

void
SV_CreateEffect(edict_t* ent, int type, int flags, vec3_t origin, char* format, ...)
{
	int enta;
	int effectID = -1;
	va_list args;
	sizebuf_t msg;
	int testflags;

	va_start(args, format);

	if (ent)
	{
		enta = ent->s.number;
	}
	else
	{
		enta = -1;
	}

	if (sv_numeffects >= MAX_PERSISTANT_EFFECTS)
	{
		Com_Printf("Warning : Unable to create persistant effect\n");
		va_end(args);
		return;
	}

	for (int i = 0; i < MAX_PERSISTANT_EFFECTS; i++)
	{
		if (SV_Persistant_Effects[i].inUse == false)
		{
			effectID = i;
			break;
		}
	}

	if (effectID == -1)
	{
		va_end(args);
		return;
	}

	PerEffectsBuffer_t* effect = &SV_Persistant_Effects[effectID];
	effect->freeBlock = 0;
	effect->bufSize = 192;
	effect->numEffects = 1;
	effect->fx_num = type;
	effect->demo_send_mask = -1;
	effect->send_mask = 0;
	effect->needsUpdate = true;
	effect->entity = ent;
	effect->inUse = true;
	effect->nonPersistant = true;

	SZ_Init(&msg, effect->buf, sizeof(effect->buf));
	MSG_WriteShort(&msg, type);

	if (ent != NULL)
	{
		ent->s.clientEffects.buf = &effect->buf[0];
		ent->s.clientEffects.bufSize = sizeof(effect->buf);
		ent->s.clientEffects.numEffects = 1;
	}

	testflags = flags;

	MSG_WriteShort(&msg, testflags);

	if ((testflags & CEF_BROADCAST) != 0 && enta >= 0)
	{
		MSG_WriteShort(&msg, enta);
	}

	if ((testflags & CEF_OWNERS_ORIGIN) == 0) {
		MSG_WritePos(&msg, origin, SV_GetRecomendedProtocol());
	}

	MSG_WriteByte(&msg, 0x3a);

	if (format && format[0]) {
		SV_WriteEffectToBuffer(&msg, format, args);
	}

	effect->data_size = msg.cursize;
	va_end(args);
}

void
SV_ClearPersistantEffects(void)
{
	sv_numeffects = 0;
	memset(&SV_Persistant_Effects, 0, sizeof(SV_Persistant_Effects));
}


/*
 * Init the game subsystem for a new map
 */
void
SV_InitGameProgs(void)
{
	game_import_t import;

	/* unload anything we have now */
	if (ge)
	{
		SV_ShutdownGameProgs();
	}

	Com_Printf("-------- game initialization -------\n");

	/* load a new game dll */
	import.multicast = SV_Multicast;
	import.unicast = PF_Unicast;
	import.bprintf = SV_BroadcastPrintf;
	import.dprintf = PF_dprintf;
	import.cprintf = PF_cprintf;
	import.centerprintf = PF_centerprintf;
	import.error = PF_error;

	import.linkentity = SV_LinkEdict;
	import.unlinkentity = SV_UnlinkEdict;
	import.BoxEdicts = SV_AreaEdicts;
	import.trace = SV_Trace;
	import.pointcontents = SV_PointContents;
	import.setmodel = PF_setmodel;
	import.inPVS = PF_inPVS;
	import.inPHS = PF_inPHS;
	import.Pmove = Pmove;

	import.modelindex = SV_ModelIndex;
	import.soundindex = SV_SoundIndex;
	import.imageindex = SV_ImageIndex;

	import.configstring = PF_Configstring;
	import.sound = PF_StartSound;
	import.positioned_sound = SV_StartSound;

	import.WriteChar = PF_WriteChar;
	import.WriteByte = PF_WriteByte;
	import.WriteShort = PF_WriteShort;
	import.WriteLong = PF_WriteLong;
	import.WriteFloat = PF_WriteFloat;
	import.WriteString = PF_WriteString;
	import.WritePosition = PF_WritePos;
	import.WriteDir = PF_WriteDir;
	import.WriteAngle = PF_WriteAngle;

	import.TagMalloc = Z_TagMalloc;
	import.TagFree = Z_Free;
	import.FreeTags = Z_FreeTags;

	import.cvar = Cvar_Get;
	import.cvar_set = Cvar_Set;
	import.cvar_forceset = Cvar_ForceSet;

	import.argc = Cmd_Argc;
	import.argv = Cmd_Argv;
	import.args = Cmd_Args;
	import.AddCommandString = Cbuf_AddText;

#ifndef DEDICATED_ONLY
	import.DebugGraph = SCR_DebugGraph;
#endif

	import.SetAreaPortalState = CM_SetAreaPortalState;
	import.AreasConnected = CM_AreasConnected;

	/* Extension to classic Quake2 API */
	import.LoadFile = FS_LoadFile;
	import.FreeFile = FS_FreeFile;
	import.Gamedir = FS_Gamedir;
	import.CreatePath = FS_CreatePath;
	import.GetConfigString = PF_ConfigStringGet;
	import.GetModelInfo = PF_GetModelInfo;
	import.GetModelFrameInfo = PF_GetModelFrameInfo;
	import.PmoveEx = PmoveEx;

	/* Heretic 2 specific */
	import.FS_NextPath = FS_NextPath;

	import.CreateEffect = SV_CreateEffect;
	import.RemoveEffects = SV_RemoveEffects;
	import.CreateEffectEvent = SV_CreateEffectEvent;
	import.RemoveEffectsEvent = SV_RemoveEffectsEvent;
	import.CreatePersistantEffect = SV_CreatePersistantEffect;
	import.RemovePersistantEffect = SV_RemovePersistantEffect;
	import.ClearPersistantEffects = SV_ClearPersistantEffects;

	import.Persistant_Effects_Array = &SV_Persistant_Effects;

	ge = (game_export_t *)Sys_GetGameAPI(&import);

	SV_ClearPersistantEffects();

	if (!ge)
	{
		Com_Error(ERR_DROP, "failed to load game DLL");
	}

	if (ge->apiversion != GAME_API_VERSION &&
		ge->apiversion != GAME_API_R97_VERSION)
	{
		Com_Error(ERR_DROP, "game is version %i, not %i", ge->apiversion,
				GAME_API_VERSION);
	}

	ge->Init();

	Com_Printf("------------------------------------\n\n");
}

