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
 * This file implements all static entities at client site.
 *
 * =======================================================================
 */

#include <math.h>
#include "header/client.h"

#include "../client_effects/client_effects.h"
#include "../../h2common/resourcemanager.h"

extern struct model_s *cl_mod_powerscreen;

//PGM
int	vidref_val;
//PGM

/*
=========================================================================

FRAME PARSING

=========================================================================
*/

/*
=================
CL_ParseEntityBits

Returns the entity number and the header bits
=================
*/
int	bitcounts[32];	/// just for protocol profiling
int
CL_ParseEntityBits(unsigned int *bits)
{
	unsigned	total;
	int			number;

	total = MSG_ReadLong (&net_message);
	number = MSG_ReadShort(&net_message);

	*bits = total;

	return number;
}

extern ResourceManager_t FXBufMgnr;

/*
==================
CL_ParseDelta

Can go from either a baseline or a previous packet_entity
==================
*/
void
CL_ParseDelta(entity_state_t *from, entity_state_t *to, int number, int bits)
{
	// set everything to the state we are delta'ing from
	*to = *from;

	VectorCopy (from->origin, to->old_origin);
	to->number = number;

	if (bits & U_CLIENT_EFFECTS)
	{
		int numEffects = MSG_ReadShort(&net_message);

		if (numEffects == -1)
		{
			if (to->clientEffects.buf != NULL)
			{
				ResMngr_DeallocateResource(&FXBufMgnr, to->clientEffects.buf, 0);
				to->clientEffects.buf = NULL;
				to->clientEffects.numEffects = 0;
			}
		}
		else
		{
			to->clientEffects.numEffects = numEffects;
			to->clientEffects.buf = (byte*)ResMngr_AllocateResource(&FXBufMgnr, ENTITY_FX_BUF_SIZE);
			to->clientEffects.bufSize = MSG_ReadShort(&net_message);;
			MSG_ReadData(&net_message, to->clientEffects.buf, to->clientEffects.bufSize);
		}
	}
	else
	{
		to->clientEffects.numEffects = 0;
	}

	if (bits & U_FM_FRAME)
	{
		for (int i = 0; i < MAX_FM_MESH_NODES; i++)
		{
			to->fmnodeinfo[i].frame = MSG_ReadShort(&net_message);
		}
	}

	if (bits & U_FM_FLAGS)
	{
		for (int i = 0; i < MAX_FM_MESH_NODES; i++)
		{
			to->fmnodeinfo[i].flags = MSG_ReadShort(&net_message);
		}
	}

	if (bits & U_MODEL)
		to->modelindex = MSG_ReadByte(&net_message);

	if (bits & U_FRAME8)
		to->frame = MSG_ReadByte(&net_message);

	if (bits & U_FRAME16)
		to->frame = MSG_ReadShort(&net_message);

	if ((bits & U_SKIN8) && (bits & U_SKIN16))		//used for laser colors
		to->skinnum = MSG_ReadLong(&net_message);
	else if (bits & U_SKIN8)
		to->skinnum = MSG_ReadByte(&net_message);
	else if (bits & U_SKIN16)
		to->skinnum = MSG_ReadShort(&net_message);

	if ( (bits & (U_EFFECTS8|U_EFFECTS16)) == (U_EFFECTS8|U_EFFECTS16) )
		to->effects = MSG_ReadLong(&net_message);
	else if (bits & U_EFFECTS8)
		to->effects = MSG_ReadByte(&net_message);
	else if (bits & U_EFFECTS16)
		to->effects = MSG_ReadShort(&net_message);

	if ( (bits & (U_RENDERFX8|U_RENDERFX16)) == (U_RENDERFX8|U_RENDERFX16) )
		to->renderfx = MSG_ReadLong(&net_message);
	else if (bits & U_RENDERFX8)
		to->renderfx = MSG_ReadByte(&net_message);
	else if (bits & U_RENDERFX16)
		to->renderfx = MSG_ReadShort(&net_message);

	if (bits & U_ORIGIN12) {
		MSG_ReadCoord(&net_message); // jmarshall: hack! padding, for some reason without this origin[0] can be nan
		to->origin[0] = MSG_ReadCoord(&net_message);
	}
	if (bits & U_ORIGIN12)
		to->origin[1] = MSG_ReadCoord (&net_message);
	if (bits & U_ORIGIN3)
		to->origin[2] = MSG_ReadCoord (&net_message);

	if (bits & U_ANGLE1) {
		MSG_ReadCoord(&net_message); // jmarshall: hack! padding, for some reason without this origin[0] can be nan
		to->angles[0] = MSG_ReadAngle(&net_message);
	}

	if (bits & U_ANGLE2) {
		MSG_ReadCoord(&net_message); // jmarshall: hack! padding, for some reason without this origin[0] can be nan
		to->angles[1] = MSG_ReadAngle(&net_message);
	}

	if (bits & U_ANGLE3)
		to->angles[2] = MSG_ReadAngle(&net_message);

	if (bits & U_OLDORIGIN)
		MSG_ReadPos (&net_message, to->old_origin);

	if (bits & U_SOUND)
		to->sound = MSG_ReadByte(&net_message);

	if (bits & U_SOLID)
		to->solid = MSG_ReadShort(&net_message);
}

/*
==================
CL_DeltaEntity

Parses deltas from the given base and adds the resulting entity
to the current frame
==================
*/
void
CL_DeltaEntity(frame_t *frame, int newnum, entity_state_t *old, int bits)
{
	centity_t	*ent;
	entity_state_t	*state;

	ent = &cl_entities[newnum];

	state = &cl_parse_entities[cl.parse_entities & (MAX_PARSE_ENTITIES-1)];
	cl.parse_entities++;
	frame->num_entities++;

	CL_ParseDelta (old, state, newnum, bits);

	// some data changes will force no lerping
	if (state->modelindex != ent->current.modelindex
		|| fabs(state->origin[0] - ent->current.origin[0]) > 512
		|| fabs(state->origin[1] - ent->current.origin[1]) > 512
		|| fabs(state->origin[2] - ent->current.origin[2]) > 512
		)
	{
		ent->serverframe = -99;
	}

	if (ent->serverframe != cl.frame.serverframe - 1)
	{	// wasn't in last update, so initialize some things
		//ent->trailcount = 1024;		// for diminishing rocket / grenade trails
		// duplicate the current state so lerping doesn't hurt anything
		ent->prev = *state;
		VectorCopy(state->old_origin, ent->prev.origin);
		VectorCopy(state->old_origin, ent->lerp_origin);
	}
	else
	{	// shuffle the last state to previous
		ent->prev = ent->current;
	}

	ent->serverframe = cl.frame.serverframe;
	ent->current = *state;
}

/*
==================
CL_ParsePacketEntities

An svc_packetentities has just been parsed, deal with the
rest of the data stream.
==================
*/
void
CL_ParsePacketEntities(frame_t *oldframe, frame_t *newframe)
{
	int			newnum;
	unsigned int			bits;
	entity_state_t	*oldstate = NULL;
	int			oldindex, oldnum;


	newframe->parse_entities = cl.parse_entities;
	newframe->num_entities = 0;

	// delta from the entities present in oldframe
	oldindex = 0;
	if (!oldframe)
	{
		oldnum = 99999;
	}
	else
	{
		if (oldindex >= oldframe->num_entities)
			oldnum = 99999;
		else
		{
			oldstate = &cl_parse_entities[(oldframe->parse_entities+oldindex) & (MAX_PARSE_ENTITIES-1)];
			oldnum = oldstate->number;
		}
	}

	while (1)
	{
		newnum = CL_ParseEntityBits (&bits);
		if (newnum >= MAX_EDICTS)
			Com_Error(ERR_DROP,"CL_ParsePacketEntities: bad number:%i", newnum);

		if (net_message.readcount > net_message.cursize)
			Com_Error(ERR_DROP,"CL_ParsePacketEntities: end of message");

		if (!newnum)
			break;

		while (oldnum < newnum)
		{
			/* one or more entities from the old packet are unchanged */
			if (cl_shownet->value == 3)
			{
				Com_Printf("   unchanged: %i\n", oldnum);
			}

			CL_DeltaEntity(newframe, oldnum, oldstate, 0);

			oldindex++;

			if (oldindex >= oldframe->num_entities)
			{
				oldnum = 99999;
			}
			else
			{
				oldstate = &cl_parse_entities[(oldframe->parse_entities+oldindex) & (MAX_PARSE_ENTITIES-1)];
				oldnum = oldstate->number;
			}
		}

		if (bits & U_REMOVE)
		{
			// the entity present in oldframe is not in the current frame
			if (cl_shownet->value == 3)
			{
				Com_Printf("   remove: %i\n", newnum);
			}

			if (oldnum != newnum)
			{
				Com_Printf("U_REMOVE: oldnum != newnum\n");
			}

			oldindex++;

			fxe.RemoveClientEffects(&cl_entities[newnum]);

			if (oldframe)
			{
				if (oldindex >= oldframe->num_entities)
				{
					oldnum = 99999;
				}
				else
				{
					oldstate = &cl_parse_entities[(oldframe->parse_entities + oldindex) & (MAX_PARSE_ENTITIES - 1)];
					oldnum = oldstate->number;
				}
			}
			continue;
		}

		if (oldnum == newnum)
		{	// delta from previous state
			if (cl_shownet->value == 3)
			{
				Com_Printf("   delta: %i\n", newnum);
			}

			CL_DeltaEntity(newframe, newnum, oldstate, bits);

			oldindex++;

			if (oldindex >= oldframe->num_entities)
			{
				oldnum = 99999;
			}
			else
			{
				oldstate = &cl_parse_entities[(oldframe->parse_entities+oldindex) & (MAX_PARSE_ENTITIES-1)];
				oldnum = oldstate->number;
			}
			continue;
		}

		if (oldnum > newnum)
		{
			/* delta from baseline */
			if (cl_shownet->value == 3)
			{
				Com_Printf("   baseline: %i\n", newnum);
			}

			CL_DeltaEntity(newframe, newnum, &cl_entities[newnum].baseline, bits);
			continue;
		}

	}

	// any remaining entities in the old frame are copied over
	while (oldnum != 99999)
	{
		/* one or more entities from the old packet are unchanged */
		if (cl_shownet->value == 3)
		{
			Com_Printf("   unchanged: %i\n", oldnum);
		}

		CL_DeltaEntity(newframe, oldnum, oldstate, 0);

		oldindex++;

		if (oldindex >= oldframe->num_entities)
		{
			oldnum = 99999;
		}
		else
		{
			oldstate = &cl_parse_entities[(oldframe->parse_entities+oldindex) & (MAX_PARSE_ENTITIES-1)];
			oldnum = oldstate->number;
		}
	}
}

/*
===================
CL_ParsePlayerstate
===================
*/
void
CL_ParsePlayerstate(frame_t *oldframe, frame_t *newframe)
{
	// TODO: Rewrite protocol
	int flags;
	player_state_t *state;

	state = &newframe->playerstate;

	// clear to old value before delta parsing
	if (oldframe)
	{
		*state = oldframe->playerstate;
	}
	else
	{
		memset (state, 0, sizeof(*state));
	}

	flags = MSG_ReadLong (&net_message);
	MSG_ReadData(&net_message, (byte*)&state->stats[0], sizeof(state->stats));
#if 0
	// TODO: Rewrite protocol
	if (flags & PS_MINSMAXS) {
		state->mins[0] = MSG_ReadFloat(&net_message);
		state->mins[1] = MSG_ReadFloat(&net_message);
		state->mins[2] = MSG_ReadFloat(&net_message);

		state->maxs[0] = MSG_ReadFloat(&net_message);
		state->maxs[1] = MSG_ReadFloat(&net_message);
		state->maxs[2] = MSG_ReadFloat(&net_message);
	}
#endif

	//
	// parse the pmove_state_t
	//
	if (flags & PS_M_TYPE)
	{
		state->pmove.pm_type = (pmtype_t)MSG_ReadByte(&net_message);
	}

	if (flags & PS_REMOTE_ID)
	{
		state->remote_id = MSG_ReadShort(&net_message);
	}

	if (flags & PS_REMOTE_VIEWORIGIN)
	{
		state->remote_vieworigin[0] = MSG_ReadShort(&net_message);
		state->remote_vieworigin[1] = MSG_ReadShort(&net_message);
		state->remote_vieworigin[2] = MSG_ReadShort(&net_message);
	}

	if (flags & PS_REMOTE_VIEWANGLES)
	{
		state->remote_viewangles[0] = MSG_ReadShort(&net_message);
		state->remote_viewangles[1] = MSG_ReadShort(&net_message);
		state->remote_viewangles[2] = MSG_ReadShort(&net_message);
	}

	if (flags & PS_M_ORIGIN_XY)
	{
		state->pmove.origin[0] = MSG_ReadShort(&net_message);
		state->pmove.origin[1] = MSG_ReadShort(&net_message);
	}

	if (flags & PS_VIEWHEIGHT)
	{
		state->viewheight = MSG_ReadShort(&net_message);
	}

	if (flags & PS_M_ORIGIN_Z)
	{
		state->pmove.origin[2] = MSG_ReadShort(&net_message);
	}

	if (flags & PS_M_VELOCITY_XY)
	{
		state->pmove.velocity[0] = MSG_ReadShort(&net_message);
		state->pmove.velocity[1] = MSG_ReadShort(&net_message);
	}

	if (flags & PS_M_VELOCITY_Z)
	{
		state->pmove.velocity[2] = MSG_ReadShort(&net_message);
	}

	if (flags & PS_M_TIME)
	{
		state->pmove.pm_time = MSG_ReadByte(&net_message);
	}

	if (flags & PS_M_FLAGS)
	{
		state->pmove.pm_flags = MSG_ReadByte(&net_message);
	}

	if (flags & PS_M_GRAVITY)
	{
		state->pmove.gravity = MSG_ReadShort(&net_message);
	}

	if (flags & PS_M_DELTA_ANGLES)
	{
		state->pmove.delta_angles[0] = MSG_ReadShort(&net_message);
		state->pmove.delta_angles[1] = MSG_ReadShort(&net_message);
		state->pmove.delta_angles[2] = MSG_ReadShort(&net_message);
	}

	if (cl.attractloop)
	{
		state->pmove.pm_type = PM_FREEZE;		// demo playback
	}

	if (flags & PS_VIEWANGLES)
	{
		state->viewangles[0] = MSG_ReadAngle16(&net_message);
		state->viewangles[1] = MSG_ReadAngle16(&net_message);
		state->viewangles[2] = MSG_ReadAngle16(&net_message);
	}

	if (flags & PS_FOV)
	{
		state->fov = MSG_ReadByte(&net_message);
	}

	if (flags & PS_RDFLAGS)
	{
		state->rdflags = MSG_ReadByte(&net_message);
	}
}


/*
==================
CL_FireEntityEvents

==================
*/
void
CL_FireEntityEvents(frame_t *frame)
{
}


/*
================
CL_ParseFrame
================
*/
void
CL_ParseFrame(void)
{
	int			cmd;
	int			len;
	frame_t		*old;

	memset (&cl.frame, 0, sizeof(cl.frame));

	cl.frame.serverframe = MSG_ReadLong (&net_message);
	cl.frame.deltaframe = MSG_ReadLong (&net_message);
	cl.frame.servertime = cl.frame.serverframe*100;

	if (cl_shownet->value == 3)
		Com_Printf("   frame:%i  delta:%i\n", cl.frame.serverframe,
			cl.frame.deltaframe);

	// If the frame is delta compressed from data that we
	// no longer have available, we must suck up the rest of
	// the frame, but not use it, then ask for a non-compressed
	// message
	if (cl.frame.deltaframe <= 0)
	{
		cl.frame.valid = true;		// uncompressed frame
		old = NULL;
		cls.demowaiting = false;	// we can start recording now
	}
	else
	{
		old = &cl.frames[cl.frame.deltaframe & UPDATE_MASK];
		if (!old->valid)
		{	// should never happen
			Com_Printf("Delta from invalid frame (not supposed to happen!).\n");
		}
		if (old->serverframe != cl.frame.deltaframe)
		{	// The frame that the server did the delta from
			// is too old, so we can't reconstruct it properly.
			Com_Printf("Delta frame too old.\n");
		}
		else if (cl.parse_entities - old->parse_entities > MAX_PARSE_ENTITIES-128)
		{
			Com_Printf("Delta parse_entities too old.\n");
		}
		else
			cl.frame.valid = true;	// valid delta parse
	}

	// clamp time
	if (cl.time > cl.frame.servertime)
	{
		cl.time = cl.frame.servertime;
	}
	else if (cl.time < cl.frame.servertime - 100)
	{
		cl.time = cl.frame.servertime - 100;
	}

	// read areabits
	len = MSG_ReadByte(&net_message);
	MSG_ReadData(&net_message, &cl.frame.areabits, len);

	// read playerinfo
	cmd = MSG_ReadByte(&net_message);
	SHOWNET(svc_strings[cmd]);
	if (cmd != svc_playerinfo)
	{
		Com_Error(ERR_DROP, "CL_ParseFrame: not playerinfo");
	}
	CL_ParsePlayerstate (old, &cl.frame);

	// read packet entities
	cmd = MSG_ReadByte(&net_message);
	SHOWNET(svc_strings[cmd]);
	if (cmd != svc_packetentities)
		Com_Error(ERR_DROP, "CL_ParseFrame: not packetentities");
	CL_ParsePacketEntities (old, &cl.frame);

	cmd = MSG_ReadByte(&net_message);
	if (cmd != svc_client_effect) {
		Com_Error(ERR_DROP, "CL_ParseFrame: not client effects");
	}
	fxe.ParseClientEffects(NULL);
#if 0
	if (cmd == svc_packetentities2)
		CL_ParseProjectiles();
#endif

	// save the frame off in the backup array for later delta comparisons
	cl.frames[cl.frame.serverframe & UPDATE_MASK] = cl.frame;

	if (cl.frame.valid)
	{
		// getting a valid frame message ends the connection process
		if (cls.state != ca_active)
		{
			cls.state = ca_active;
			cl.force_refdef = true;
			cl.predicted_origin[0] = cl.frame.playerstate.pmove.origin[0]*0.125;
			cl.predicted_origin[1] = cl.frame.playerstate.pmove.origin[1]*0.125;
			cl.predicted_origin[2] = cl.frame.playerstate.pmove.origin[2]*0.125;
			VectorCopy (cl.frame.playerstate.viewangles, cl.predicted_angles);
			if (cls.disable_servercount != cl.servercount
				&& cl.refresh_prepped)
				SCR_EndLoadingPlaque();	// get rid of loading plaque
		}
		cl.sound_prepped = true;	// can start mixing ambient sounds

		// fire entity events
		CL_FireEntityEvents (&cl.frame);
		CL_CheckPredictionError ();
	}
}

/*
==========================================================================

INTERPOLATE BETWEEN FRAMES TO GET RENDERING PARMS

==========================================================================
*/

struct model_s *
S_RegisterSexedModel(entity_state_t *ent, char *base)
{
	int				n;
	char			*p;
	struct model_s	*mdl;
	char			model[MAX_QPATH];
	char			buffer[MAX_QPATH];

	// determine what model the client is using
	model[0] = 0;
	n = CS_PLAYERSKINS + ent->number - 1;
	if (cl.configstrings[n][0])
	{
		p = strchr(cl.configstrings[n], '\\');
		if (p)
		{
			p += 1;
			strcpy(model, p);
			p = strchr(model, '/');
			if (p)
				*p = 0;
		}
	}
	// if we can't figure it out, they're male
	if (!model[0])
		strcpy(model, "male");

	Com_sprintf (buffer, sizeof(buffer), "players/%s/%s", model, base+1);
	mdl = re.RegisterModel(buffer);
	if (!mdl) {
		// not found, try default weapon model
		Com_sprintf (buffer, sizeof(buffer), "players/%s/weapon.md2", model);
		mdl = re.RegisterModel(buffer);
		if (!mdl) {
			// no, revert to the male model
			Com_sprintf (buffer, sizeof(buffer), "players/%s/%s", "male", base+1);
			mdl = re.RegisterModel(buffer);
			if (!mdl) {
				// last try, default male weapon.md2
				Com_sprintf (buffer, sizeof(buffer), "players/male/weapon.md2");
				mdl = re.RegisterModel(buffer);
			}
		}
	}

	return mdl;
}

/*
===============
CL_OffsetThirdPersonView

===============
*/
#define FOCUS_DISTANCE  512
#define VectorMA2(v, s, b, o) \
	{	\
		(o)[0] = (v)[0] + (b)[0] * (s);	\
		(o)[1] = (v)[1] + (b)[1] * (s);	\
		(o)[2] = (v)[2] + (b)[2] * (s);	\
	}

void
CL_OffsetThirdPersonView(void)
{
	vec3_t forward, right, up;
	vec3_t view;
	vec3_t focusAngles;
	trace_t trace;
	static vec3_t mins = { -4, -4, -4 };
	static vec3_t maxs = { 4, 4, 4 };
	vec3_t focusPoint;
	float focusDist;
	float forwardScale, sideScale;

	//cl.refdef.vieworg[2] += cg.predictedPlayerState.viewheight;

	VectorCopy(cl.refdef.viewangles, focusAngles);

	// if dead, look at killer
	//if (cg.predictedPlayerState.stats[STAT_HEALTH] <= 0) {
	//	focusAngles[YAW] = cg.predictedPlayerState.stats[STAT_DEAD_YAW];
	//	cg.refdefViewAngles[YAW] = cg.predictedPlayerState.stats[STAT_DEAD_YAW];
	//}

	if (focusAngles[PITCH] > 45) {
		focusAngles[PITCH] = 45;        // don't go too far overhead
	}
	AngleVectors(focusAngles, forward, NULL, NULL);

	VectorMA2(cl.refdef.vieworg, FOCUS_DISTANCE, forward, focusPoint);

	VectorCopy(cl.refdef.vieworg, view);

	view[2] += 24; // TODO: view height

	cl.refdef.viewangles[PITCH] *= 0.5;

	AngleVectors(cl.refdef.viewangles, forward, right, up);

	float cg_thirdPersonAngle = 0.0f;
	float cg_thirdPersonRange = 64.0f; // TODO: view range

	forwardScale = cos(cg_thirdPersonAngle / 180 * M_PI);
	sideScale = sin(cg_thirdPersonAngle / 180 * M_PI);

	VectorMA2(view, -cg_thirdPersonRange * forwardScale, forward, view);
	VectorMA2(view, -cg_thirdPersonRange * sideScale, right, view);

	// trace a ray from the origin to the viewpoint to make sure the view isn't
	// in a solid block.  Use an 8 by 8 block to prevent the view from near clipping anything

	trace = CM_BoxTrace( cl.refdef.vieworg, view, mins, maxs,  0, MASK_PLAYERSOLID);

	if (trace.fraction != 1.0) {
		VectorCopy(trace.endpos, view);
		view[2] += (1.0 - trace.fraction) * 32;
		// try another trace to this position, because a tunnel may have the ceiling
		// close enogh that this is poking out

		trace = CM_BoxTrace(cl.refdef.vieworg, view, mins, maxs, 0, MASK_PLAYERSOLID);
		VectorCopy(trace.endpos, view);
	}


	VectorCopy(view, cl.refdef.vieworg);

	// select pitch to look at focus point from vieword
	VectorSubtract(focusPoint, cl.refdef.vieworg, focusPoint);
	focusDist = sqrt(focusPoint[0] * focusPoint[0] + focusPoint[1] * focusPoint[1]);
	if (focusDist < 1) {
		focusDist = 1;  // should never happen
	}
	cl.refdef.viewangles[PITCH] = -180 / M_PI * atan2(focusPoint[2], focusDist);
	cl.refdef.viewangles[YAW] -= cg_thirdPersonAngle;
}


/*
===============
CL_CalcViewValues

Sets cl.refdef view values
===============
*/
void
CL_CalcViewValues(void)
{
	int			i;
	float		lerp;
	frame_t* oldframe;
	player_state_t* ps, * ops;

	// find the previous frame to interpolate from
	ps = &cl.frame.playerstate;
	i = (cl.frame.serverframe - 1) & UPDATE_MASK;
	oldframe = &cl.frames[i];
	if (oldframe->serverframe != cl.frame.serverframe - 1 || !oldframe->valid)
		oldframe = &cl.frame;		// previous frame was dropped or involid
	ops = &oldframe->playerstate;

	// see if the player entity was teleported this frame
	if (fabs(ops->pmove.origin[0] - ps->pmove.origin[0]) > 256 * 8
		|| abs(ops->pmove.origin[1] - ps->pmove.origin[1]) > 256 * 8
		|| abs(ops->pmove.origin[2] - ps->pmove.origin[2]) > 256 * 8)
		ops = ps;		// don't interpolate

	lerp = cl.lerpfrac;

	// calculate the origin
	//if ((cl_predict->value) && !(cl.frame.playerstate.pmove.pm_flags & PMF_NO_PREDICTION))
	//{	// use predicted values
	//	unsigned	delta;
	//
	//	backlerp = 1.0 - lerp;
	//	for (i=0 ; i<3 ; i++)
	//	{
	//		cl.refdef.vieworg[i] = cl.predicted_origin[i]
	//			- backlerp * cl.prediction_error[i];
	//	}
	//
	//	// smooth out stair climbing
	//	delta = cls.realtime - cl.predicted_step_time;
	//	if (delta < 100)
	//		cl.refdef.vieworg[2] -= cl.predicted_step * (100 - delta) * 0.01;
	//}
	//else
	//{
	//}
// jmarshall
	if (ps->remote_id != -1)
	{
		// just use interpolated values
		for (i = 0; i < 3; i++)
			cl.refdef.vieworg[i] = ops->remote_vieworigin[i];

		// just use interpolated values
		for (i = 0; i < 3; i++)
			cl.refdef.viewangles[i] = LerpAngle(ops->remote_viewangles[i], ps->remote_viewangles[i], lerp);

		//for (i=0 ; i<3 ; i++)
		//	cl.refdef.viewangles[i] += LerpAngle (ops->kick_angles[i], ps->kick_angles[i], lerp);


	}
	else if(cl.refdef.entities)
	{
		// just use interpolated values
		for (i = 0; i < 3; i++)
			cl.refdef.entities[0]->origin[i] = cl.refdef.vieworg[i] = ops->pmove.origin[i] * 0.125
			+ lerp * (ps->pmove.origin[i] * 0.125
				- (ops->pmove.origin[i] * 0.125));

			// if not running a demo or on a locked frame, add the local angle movement
		if (cl.frame.playerstate.pmove.pm_type < PM_DEAD)
		{	// use predicted values
			for (i = 0; i < 3; i++)
				cl.refdef.viewangles[i] = cl.predicted_angles[i];
		}
		else
		{	// just use interpolated values
			for (i = 0; i < 3; i++)
				cl.refdef.entities[0]->angles[i] = cl.refdef.viewangles[i] = LerpAngle(ops->viewangles[i], ps->viewangles[i], lerp);
		}

		CL_OffsetThirdPersonView();
	}

	AngleVectors(cl.refdef.viewangles, cl.v_forward, cl.v_right, cl.v_up);

	// interpolate field of view
	cl.refdef.fov_x = ops->fov + lerp * (ps->fov - ops->fov);



	// don't interpolate blend color
	//for (i=0 ; i<4 ; i++)
	//	cl.refdef.blend[i] = ps->blend[i];
}

/*
 * Emits all entities, particles, and lights to the refresh
 */
void
CL_AddEntities(void)
{
	if (cls.state != ca_active)
	{
		return;
	}

	if (cl.time > cl.frame.servertime)
	{
		if (cl_showclamp->value)
		{
			Com_Printf("high clamp %i\n", cl.time - cl.frame.servertime);
		}

		cl.time = cl.frame.servertime;
		cl.lerpfrac = 1.0;
	}
	else if (cl.time < cl.frame.servertime - 100)
	{
		if (cl_showclamp->value)
		{
			Com_Printf("low clamp %i\n", cl.frame.servertime - 100 - cl.time);
		}

		cl.time = cl.frame.servertime - 100;
		cl.lerpfrac = 0;
	}
	else
	{
		cl.lerpfrac = 1.0 - (cl.frame.servertime - cl.time) * 0.01f;
	}

	if (cl_timedemo->value)
	{
		cl.lerpfrac = 1.0;
	}

//	CL_AddPacketEntities (&cl.frame);
//	CL_AddTEnts ();
//	CL_AddParticles ();
//	CL_AddDLights ();
//	CL_AddLightStyles ();

// jmarshall - this is in client effects.dll
	fxe.AddPacketEntities(&cl.frame); // CL_AddPacketEntities (&cl.frame);
	fxe.AddEffects(false);

	CL_CalcViewValues();
// jmarshall end
	//CL_AddParticles ();
	//CL_AddDLights ();
	//CL_AddLightStyles ();
}

/*
 * Called to get the sound spatialization origin
 */
void
CL_GetEntitySoundOrigin(int ent, vec3_t org)
{
	centity_t *old;

	if ((ent < 0) || (ent >= MAX_EDICTS))
	{
		Com_Error(ERR_DROP, "CL_GetEntitySoundOrigin: bad ent");
	}

	old = &cl_entities[ent];
	VectorCopy(old->lerp_origin, org);
}
