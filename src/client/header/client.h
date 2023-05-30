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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307,
 * USA.
 *
 * =======================================================================
 *
 * Main header for the client
 *
 * =======================================================================
 */

#ifndef CL_CLIENT_H
#define CL_CLIENT_H

#define MAX_CLIENTWEAPONMODELS 20
#define	CMD_BACKUP 256 /* allow a lot of command backups for very fast systems */

/* the cl_parse_entities must be large enough to hold UPDATE_BACKUP frames of
   entities, so that when a delta compressed message arives from the server
   it can be un-deltad from the original */
#define	MAX_PARSE_ENTITIES	1024

#define MAX_SUSTAINS		32
#define	PARTICLE_GRAVITY 40
#define BLASTER_PARTICLE_COLOR 0xe0
#define INSTANT_PARTICLE -10000.0

#include <math.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "../../common/header/common.h"

#include "../curl/header/download.h"
#include "../sound/header/sound.h"
#include "../sound/header/vorbis.h"
#include "../vid/header/ref.h"
#include "../vid/header/vid.h"

#include "screen.h"
#include "keyboard.h"
#include "console.h"

#include "../../../h2common/angles.h"
#include "../../../h2common/vector.h"
#include "../../player/player.h"
#include "../../../h2common/levelmaps.h"

typedef struct
{
	qboolean		valid; /* cleared if delta parsing was invalid */
	int				serverframe;
	int				servertime; /* server time the message is valid for (in msec) */
	int				deltaframe;
	byte			areabits[MAX_MAP_AREAS/8]; /* portalarea visibility bits */
	player_state_t	playerstate;
	int				num_entities;
	int				parse_entities; /* non-masked index into cl_parse_entities array */
} frame_t;

typedef struct
{
	entity_state_t	baseline; /* delta from this if not from a previous frame */
	entity_state_t	current;
	entity_state_t	prev; /* will always be valid, but might just be a copy of current */

	int			serverframe; /* if not current, this ent isn't in the frame */

	int			trailcount;	 /* for diminishing grenade trails */
	vec3_t		lerp_origin; /* for trails (variable hz) */

	int			fly_stoptime;

	int			flags;			// What freaking flags go in here??!?!

	vec3_t			origin;			// current interpolated origin

	vec3_t			lerp_angles;	// current interpolated angles

	struct entity_s		*entity;		// so client fx can play with its owners entity

	struct client_entity_s		*effects;		// client effects, only has meaning within the
												// Client Effects DLL

	struct LERPedReferences_s	*referenceInfo;

	entity_state_t		*s1;			// pointer to the corresponding entity_state_t in												// cl_parse_entities.
} centity_t;

// ********************************************************************************************
// CF_XXX
// ------
// Flags for the client side entity to know if its been deleted on the server, or is server
// culled.
// ********************************************************************************************

#define CF_INUSE			0x00000001
#define CF_SERVER_CULLED	0x00000002

// ********************************************************************************************
// predictinfo_t
// -------------
// Repositiory for all elements of player rendering that need to be predicted. When prediction
// is active, the values below are written by CL_DoPrediction() and read by AddServerEntities()
// instead of using values derived from server sent data.
// ********************************************************************************************

typedef struct
{
	int				prevFrame,currFrame,
					prevSwapFrame,currSwapFrame;
	vec3_t			prevAngles,currAngles;
	float			playerLerp;
	int				effects,
					renderfx,
					skinnum,
					clientnum;
	fmnodeinfo_t	fmnodeinfo[MAX_FM_MESH_NODES];
} predictinfo_t;

// jmarshall: clean up some memory issues
#define CL_MAXMODELS  16
// jmarshall: clean up some memory issues
typedef struct
{
	char	name[MAX_QPATH];

	struct image_s	*skin[SKIN_MAX];

	char	iconname[MAX_QPATH];
	struct model_s	*model[CL_MAXMODELS]; // jmarshall: clean up some memory issues
	char	skin_name[MAX_QPATH];
	char	model_name[MAX_QPATH];
	vec3_t	origin;
} clientinfo_t;

/* the client_state_t structure is wiped
   completely at every server map change */
typedef struct
{
	int			timeoutcount;

	int			timedemo_frames;
	int			timedemo_start;

	qboolean	refresh_prepped; /* false if on new level or new ref dll */
	qboolean	sound_prepped; /* ambient sounds can start */
	qboolean	force_refdef; /* vid has changed, so we can't use a paused refdef */

	int			parse_entities; /* index (not anded off) into cl_parse_entities[] */

	usercmd_t	cmd;
	usercmd_t	cmds[CMD_BACKUP]; /* each mesage will send several old cmds */
	int			cmd_time[CMD_BACKUP]; /* time sent, for calculating pings */
	short		predicted_origins[CMD_BACKUP][3]; /* for debug comparing against server */

	float		predicted_step; /* for stair up smoothing */
	unsigned	predicted_step_time;

	vec3_t		predicted_origin; /* generated by CL_PredictMovement */
	vec3_t		predicted_angles;
	vec3_t		prediction_error;

	frame_t		frame; /* received from server */
	int			surpressCount; /* number of messages rate supressed */
	frame_t		frames[UPDATE_BACKUP];

	/* the client maintains its own idea of view angles, which are
	   sent to the server each frame.  It is cleared to 0 upon entering each level.
	   the server sends a delta each frame which is added to the locally
	   tracked view angles to account for standing on rotating objects,
	   and teleport direction changes */
	vec3_t		viewangles;

	int			time; /* this is the time value that the client is rendering at. always <= cls.realtime */
	float		lerpfrac; /* between oldframe and frame */

	refdef_t	refdef;

	vec3_t		v_forward, v_right, v_up; /* set when refdef.angles is set */

	/* transient data from server */
	char		layout[1024]; /* general 2D overlay */
	int			inventory[MAX_ITEMS];

	/* non-gameserver infornamtion */
	fileHandle_t cinematic_file;
	int			cinematictime; /* cls.realtime for first cinematic frame */
	int			cinematicframe;
	unsigned char	cinematicpalette[768];
	qboolean	cinematicpalette_active;

	/* server state information */
	qboolean	attractloop; /* running the attract loop, any key will menu */
	int			servercount; /* server identification for prespawns */
	char		gamedir[MAX_QPATH];
	int			playernum;

	char		configstrings[MAX_CONFIGSTRINGS][MAX_QPATH];

	/* locally derived information from server state */

	struct model_s	*model_draw[MAX_MODELS];

	struct cmodel_s	*model_clip[MAX_MODELS];

	struct sfx_s	*sound_precache[MAX_SOUNDS];

	struct image_s	*image_precache[MAX_IMAGES];

	clientinfo_t	clientinfo[MAX_CLIENTS];
	clientinfo_t	baseclientinfo;

	// Client camera vieworigin and viewangles sent to server so it can do accurate(ish) culling.

	vec3_t		camera_vieworigin,camera_viewangles;

	// this is calculated on the client, as the distance between the client and the roof, and walls. - Used for EAX environment mapping.

	float		wall_dist[5];
	int			wall_check;					// used to determing which wall/ceiling we are checking on any given frame.

	// The time value that the client is rendering at. This is always <= cls.realtime.

	int				lastanimtime;
	int				PIV;

	playerinfo_t	playerinfo;

	predictinfo_t	predictinfo;
} client_state_t;

extern	client_state_t	cl;

/* the client_static_t structure is persistant through
   an arbitrary number of server connections */
typedef enum
{
	ca_uninitialized,
	ca_disconnected,  /* not talking to a server */
	ca_connecting, /* sending request packets to the server */
	ca_connected, /* netchan_t established, waiting for svc_serverdata */
	ca_active /* game views should be displayed */
} connstate_t;

typedef enum
{
	dl_none,
	dl_model,
	dl_sound,
	dl_skin,
	dl_single
} dltype_t;

typedef enum {key_game, key_console, key_message, key_menu} keydest_t;

typedef struct
{
	connstate_t	state;
	keydest_t	key_dest;

	int			framecount;
	int			realtime; /* always increasing, no clamping, etc, in MS */
	float		rframetime; /* seconds since last render frame */
	float		nframetime; /* network frame time */
	float		framemodifier;		// variable to mod cfx by

	/* screen rendering information */
	float		disable_screen; /* showing loading plaque between levels */
								/* or changing rendering dlls */

	/* if time gets > 30 seconds ahead, break it */
	int			disable_servercount; /* when we receive a frame and cl.servercount */
									 /* > cls.disable_servercount, clear disable_screen */

	/* connection information */
	char		servername[256]; /* name of server from original connect */
	float		connect_time; /* for connection retransmits */

	int			quakePort; /* a 16 bit value that allows quake servers */
						   /* to work around address translating routers */
	netchan_t	netchan;
	int			serverProtocol; /* in case we are doing some kind of version hack */

	int			challenge; /* from the server to use for connecting */

	qboolean	forcePacket; /* Forces a package to be send at the next frame. */

	FILE		*download; /* file transfer from server */
	char		downloadtempname[MAX_OSPATH];
	char		downloadname[MAX_OSPATH];
	int			downloadnumber;
	dltype_t	downloadtype;
	size_t		downloadposition;
	int			downloadpercent;

	/* demo recording info must be here, so it isn't cleared on level change */
	qboolean	demorecording;
	qboolean	demowaiting; /* don't record until a non-delta message is received */
	FILE		*demofile;

#ifdef USE_CURL
	/* http downloading */
	dlqueue_t  downloadQueue; /* queues with files to download. */
	dlhandle_t HTTPHandles[MAX_HTTP_HANDLES]; /* download handles. */
	char	   downloadServer[512]; /* URL prefix to dowload from .*/
	char	   downloadServerRetry[512]; /* retry count. */
	char	   downloadReferer[32]; /* referer string. */
#endif
} client_static_t;

extern client_static_t	cls;

/* Evil hack against too many power screen and power
   shield impact sounds. For example if the player
   fires his shotgun onto a Brain. */
extern int num_power_sounds;

/* Even more evil hack against spurious cinematic
   aborts caused by an unspeakable evil hack right
   out of the deeps of hell... Aeh... KeyEvent(). */
extern int abort_cinematic;

/* cvars */
extern	cvar_t	*gl1_stereo_separation;
extern	cvar_t	*gl1_stereo_convergence;
extern	cvar_t	*gl1_stereo;
extern	cvar_t	*cl_gun;
extern	cvar_t	*cl_add_blend;
extern	cvar_t	*cl_add_lights;
extern	cvar_t	*cl_add_particles;
extern	cvar_t	*cl_add_entities;
extern	cvar_t	*cl_predict;
extern	cvar_t	*cl_footsteps;
extern	cvar_t	*cl_noskins;
extern	cvar_t	*cl_upspeed;
extern	cvar_t	*cl_forwardspeed;
extern	cvar_t	*cl_sidespeed;
extern	cvar_t	*cl_yawspeed;
extern	cvar_t	*cl_pitchspeed;
extern	cvar_t	*cl_run;
extern	cvar_t	*cl_anglespeedkey;
extern	cvar_t	*cl_shownet;
extern	cvar_t	*cl_showmiss;
extern	cvar_t	*cl_showclamp;
extern	cvar_t	*lookstrafe;
extern	cvar_t	*joy_layout;
extern	cvar_t	*gyro_mode;
extern	cvar_t	*gyro_turning_axis;
extern	cvar_t	*m_pitch;
extern	cvar_t	*m_yaw;
extern	cvar_t	*m_forward;
extern	cvar_t	*m_side;
extern	cvar_t	*freelook;
extern	cvar_t	*cl_lightlevel;
extern	cvar_t	*cl_paused;
extern	cvar_t	*cl_loadpaused;
extern  cvar_t  *cl_audiopaused;
extern  cvar_t  *cl_unpaused_scvis;
extern	cvar_t	*cl_timedemo;
extern	cvar_t	*cl_vwep;
extern	cvar_t  *horplus;
extern	cvar_t	*cin_force43;
extern	cvar_t	*vid_fullscreen;
extern  cvar_t  *vid_renderer;
extern	cvar_t	*cl_kickangles;
extern  cvar_t  *cl_r1q2_lightstyle;
extern  cvar_t  *cl_limitsparksounds;
extern	cvar_t	*cl_autoskins;
extern	cvar_t	*cl_maxfps;
extern	cvar_t	*cl_frametime;
extern	cvar_t	*cl_predict_local;
extern	cvar_t	*cl_predict_remote;
extern	cvar_t *mouse_sensitivity_x;
extern	cvar_t *mouse_sensitivity_y;
extern	cvar_t *doubletap_speed;
extern	cvar_t *allow_download;
extern	cvar_t *allow_download_maps;
extern	cvar_t *allow_download_players;
extern	cvar_t *allow_download_models;
extern	cvar_t *allow_download_sounds;
extern	cvar_t *cl_freezeworld;
extern	cvar_t *lookspring;
extern cvar_t *cl_camera_clipdamp;
extern cvar_t *cl_camera_combat;
extern cvar_t *cl_camera_dampfactor;
extern cvar_t *cl_camera_fpoffs;
extern cvar_t *cl_camera_freeze;
extern cvar_t *cl_camera_under_surface;
extern cvar_t *cl_camera_viewdist;
extern cvar_t *cl_camera_viewmin;
extern cvar_t *cl_camera_viewmax;

extern cvar_t *cl_camera_fpmode;		// First person mode
extern cvar_t *cl_camera_fptrans;
extern cvar_t *cl_camera_fpdist;
extern cvar_t *cl_camera_fpheight;
extern cvar_t *cl_playertrans;

extern cvar_t *EAX_preset;
extern cvar_t *EAX_default;
extern cvar_t *cl_cinematicfreeze;
extern cvar_t *shownames;
extern cvar_t *autoweapon;
extern cvar_t *cl_showcaptions;

extern cvar_t *colour_obituary;
extern cvar_t *colour_chat;
extern cvar_t *colour_names;
extern cvar_t *colour_teamchat;
extern cvar_t *colour_level;
extern cvar_t *colour_game;
extern cvar_t *game_downloadable_type;
extern cvar_t *cl_no_middle_text;

typedef struct
{
	int		key; /* so entities can reuse same entry */
	vec3_t	color;
	vec3_t	origin;
	float	radius;
	float	die; /* stop lighting after this time */
	float	decay; /* drop this each second */
	float	minlight; /* don't add when contributing less */
} cdlight_t;

extern	centity_t	cl_entities[MAX_EDICTS];
extern	cdlight_t	cl_dlights[MAX_DLIGHTS];

extern	entity_state_t	cl_parse_entities[MAX_PARSE_ENTITIES];

extern	netadr_t	net_from;
extern	sizebuf_t	net_message;

extern qboolean paused_at_load;

void DrawString (int x, int y, char *s);
void DrawStringScaled(int x, int y, char *s, float factor);
void DrawAltString (int x, int y, char *s);	/* toggle high bit */
void DrawAltStringScaled(int x, int y, char *s, float factor);
qboolean	CL_CheckOrDownloadFile (char *filename);

void CL_AddNetgraph (void);
int CL_ParseEntityBits (unsigned *bits);
void CL_ParseDelta (entity_state_t *from, entity_state_t *to, int number, int bits);
void CL_ParseFrame (void);


//=============================================================================
#define	FX_API_VERSION		1

//
// these are the data and functions exported by the client fx module
//
typedef struct
{
	// if api_version is different, the dll cannot be used
	int		api_version;

	void (*Init)();
	void (*ShutDown)();

	void (*Clear)();

	void (*RegisterSounds)();
	void (*RegisterModels)();

	void (*ParseClientEffects)(centity_t *cent);
	void (*RemoveClientEffects)(centity_t *cent);

	void (*AddPacketEntities)(frame_t *frame);
	void (*AddEffects)(qboolean freeze);
	void (*UpdateEffects)();

	void (*SetLightstyle)(int i);
	level_map_info_t *(*GetLMI)();
	int (*GetLMIMax)();
	char *(client_string);

} client_fx_export_t;

extern client_fx_export_t fxe;

//
// these are the data and functions imported by the client fx module
//
typedef struct
{
	client_state_t	*cl;
	client_static_t *cls;

	int *r_numentities;
	entity_t **r_entities;

	int *r_num_alpha_entities;
	entity_t **r_alpha_entities;

	int *r_numdlights;
	dlight_t *r_dlights;

	lightstyle_t *r_lightstyles;

	int *r_numparticles;
	particle_t *r_particles;

	int *r_anumparticles;
	particle_t *r_aparticles;

	// Client versions of the game entities.

	centity_t		*server_entities;

	// Buffer into which net stuff is parsed.

	entity_state_t	*parse_entities;

	sizebuf_t		*net_message;
	float			*PlayerAlpha;
	struct			ResourceManager_s *FXBufMngr;
	entity_t		**PlayerEntPtr;

	// Client prediction stuff.

	cvar_t			*cl_predict;
	int				*cl_effectpredict;
	predictinfo_t	*predictinfo;
	float			*leveltime;
	float			*Highestleveltime;
	float			*EffectEventIdTimeArray;
	EffectsBuffer_t *clientPredEffects;

	//

	void	(*Sys_Error) (int err_level, char *str, ...);
	void	(*Com_Error) (int code, char *fmt, ...);
	void	(*Con_Printf) (int print_level, char *str, ...);

	//

	cvar_t *(*Cvar_Get) (char *name, char *value, int flags);
	cvar_t *(*Cvar_Set)( char *name, char *value );
	void	(*Cvar_SetValue)( char *name, float value );
	float	(*Cvar_VariableValue) (char *var_name);
	char	*(*Cvar_VariableString) (char *var_name);

	// allow the screen flash to be controlled from within the client effect DLL rather than going through the server.
	// this means we get 60 hz (hopefully) screen flashing, rather than 10 hz
	void	(*Activate_Screen_Flash)(int color);

	// allow the client to call a screen shake - done within the camera code, so we can shake the screen at 60hz
	void 	(*Activate_Screen_Shake)(float intensity, float duration, float current_time, int flags);

	qboolean	(*Get_Crosshair)(vec3_t origin, byte *type);

	void	(*S_StartSound)(vec3_t origin, int entnum, int entchannel, sfx_t *sfx, float fvol, float attenuation, float timeofs);
	struct sfx_s	*(*S_RegisterSound)(char *name);
	struct model_s *(*RegisterModel) (char *name);

	int		(*GetEffect)(centity_t *ent, int flags, char *format, ...);

	void	*(*TagMalloc)(int size, int tag);
	void	(*TagFree)(void *block);
	void	(*FreeTags)(int tag);

	void	(*Trace)(vec3_t start, vec3_t mins, vec3_t maxs, vec3_t end, int brushmask, int flags, trace_t *t);
	qboolean (*InCameraPVS)(vec3_t point);
} client_fx_import_t;

// this is the only function actually exported at the linker level
typedef	client_fx_export_t (*GetfxAPI_t) (client_fx_import_t);

#define ENTITY_FX_BUF_BLOCK_SIZE 256

extern struct ResourceManager_s cl_FXBufMngr;
//=================================================

void CL_PrepRefresh (void);
void CL_RegisterSounds (void);

void CL_Quit_f (void);

void IN_Accumulate (void);

void CL_ParseLayout (void);

//
// cl_main
//
extern refexport_t		re;				// interface to refresh DLL.
extern player_export_t	playerExport;	// interface to player DLL.

void CL_Init (void);
char *CL_GetGameString(int i);
char *CL_GetGameWav(int i);
char *CL_GetLevelString(int i);
char *CL_GetLevelWav(int i);
void CL_LoadStrings(void);
void CL_RequestNextDownload (void);

void CL_FixUpGender(void);
void CL_Disconnect (void);
void CL_Disconnect_f (void);
void CL_GetChallengePacket (void);
void CL_PingServers_f (void);
void CL_Snd_Restart_f (void);
void CL_Snd_Restart_f_nocfx (void);

typedef struct
{
	int			down[2]; /* key nums holding it down */
	unsigned	downtime; /* msec timestamp */
	unsigned	msec; /* msec down this frame */
	int			state;
} kbutton_t;

extern	kbutton_t	in_mlook, in_klook;
extern 	kbutton_t 	in_strafe;
extern 	kbutton_t 	in_speed;
extern	kbutton_t	in_lookaround;

void CL_InitInput (void);
void CL_RefreshCmd(void);
void CL_SendCmd (void);
void CL_RefreshMove(void);
void CL_SendMove (usercmd_t *cmd);

void CL_ClearState (void);

void CL_ReadPackets (void);

int  CL_ReadFromServer (void);
void CL_WriteToServer (usercmd_t *cmd);
void CL_BaseMove (usercmd_t *cmd);

void IN_CenterView (void);

float CL_KeyState (kbutton_t *key);
char *Key_KeynumToString (int keynum);

void CL_ParseDemoClientEffects (void);
void CL_WriteDemoMessage (void);
void CL_Stop_f (void);
void CL_Record_f (void);

extern	char *svc_strings[256];

void CL_ParseServerMessage (void);
void CL_LoadClientinfo (clientinfo_t *ci, char *s);
void SHOWNET(char *s);
void CL_ParseClientinfo (int player);

void V_Init (void);
void V_RenderView( float stereo_separation );

//
// cl_pred.c
//
void CL_InitPrediction (void);
void CL_PredictMove (void);
void CL_CheckPredictionError (void);

//
// menus
//
void M_Init (void);
void M_Keydown (int key);
void M_Draw (void);
void M_Menu_Main_f (void);
void M_ForceMenuOff (void);
void MenuUnsetMode(void);
void M_AddToServerList (netadr_t adr, char *info);

//
// cl_inv.c
//
void CL_ParseInventory (void);
void CL_KeyInventory (int key);
void CL_DrawInventory (void);

//
// cl_pred.c
//
void CL_PredictMovement(void);
void CL_ClipMoveToEntities(vec3_t start,vec3_t mins,vec3_t maxs,vec3_t end,trace_t *tr);

//
// cl_view.c
//

void Grab_EAX_Environment_type(void);

void SNDEAX_SetEnvironment(int id);
void S_Init(void);
struct sfx_s* S_FindName(char* name, qboolean create);
struct sfx_s* S_RegisterSound(char* name);
void S_Activate(qboolean active);
void S_BeginRegistration(void);
void S_EndRegistration(void);
void S_Shutdown(void);
void S_StartLocalSound(char* sound);
void S_StartSound(vec3_t origin, int entnum, int entchannel, sfx_t *sfx, float fvol, float attenuation, float timeofs);
void S_StopAllSounds(void);
void S_StopAllSounds_Sounding(void);
void S_Update(vec3_t quake_origin, vec3_t forward, vec3_t right, vec3_t up);

void CL_RunDLights(void);
void CL_RunLightStyles(void);
void CL_SetLightstyle(int i);

void DrawAltString(int x, int y, char* s);
void CL_AddEntities(void);

trace_t CL_PMTrace(vec3_t start, vec3_t mins, vec3_t maxs, vec3_t end);

#endif
