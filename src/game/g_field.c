//
// Heretic II
// Copyright 1998 Raven Software
//
#include "header/local.h"
#include "player/library/player.h"
#include "player/library/p_actions.h"
#include "header/g_defaultmessagehandler.h"
#include "player/library/p_main.h"
#include "header/buoy.h"
#include "monster/stats/stats.h"
#include "header/g_teleport.h"

extern void SP_misc_teleporter (edict_t *self);


void InitTrigger(edict_t *self);

void
InitField(edict_t *self)
{
	if(!Vec3IsZero(self->s.angles))
	{
		G_SetMovedir(self->s.angles, self->movedir);
	}

	self->classID = CID_TRIGGER;	// fields are basically triggers
	self->solid = SOLID_TRIGGER;
	self->movetype = MOVETYPE_NONE;
	gi.setmodel (self, self->model);
	self->svflags = SVF_NOCLIENT;

	gi.linkentity(self);
}



void FogDensity_touch(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	player_state_t		*ps;

	// Only players can know about fog density changes
	if(other->client)
	{
		ps = &other->client->ps;

		if (!self->target)
		{
			ps->fog_density = 0.0;
			return;
		}
		ps->fog_density = strtod(self->target, NULL);
	}
}

/*QUAKED trigger_fogdensity (.5 .5 .5) ?
Sets the value of r_fog_density
and the fog color
---------KEY----------------
target - fog density (.01 - .0001)
color - red green blue values (0 0 0)
        range of 1.0 - 0
----------------------------
*/
void SP_trigger_fogdensity(edict_t *self)
{
	InitField(self);

	self->touch = FogDensity_touch;
	self->solid = SOLID_TRIGGER;
}


//----------------------------------------------------------------------
// Damage Field
//----------------------------------------------------------------------

void DamageField_Use(edict_t *self, edict_t *other, edict_t *activator);
void DamageField_Touch(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf);

void TrigDamage_Deactivate(edict_t *self, G_Message_t *msg)
{
	self->solid = SOLID_NOT;
	self->use = NULL;
}


void TrigDamage_Activate(edict_t *self, G_Message_t *msg)
{
	self->solid = SOLID_TRIGGER;
	self->use = DamageField_Use;
	gi.linkentity(self);
}


void TrigDamageStaticsInit()
{
	classStatics[CID_TRIG_DAMAGE].msgReceivers[G_MSG_SUSPEND] = TrigDamage_Deactivate;
	classStatics[CID_TRIG_DAMAGE].msgReceivers[G_MSG_UNSUSPEND] = TrigDamage_Activate;
}


/*QUAKED trigger_Damage (.5 .5 .5) ? START_OFF TOGGLE SILENT NO_PROTECTION SLOW
Any entity that Touches this will be Damage.

It does dmg points of Damage each server frame

SILENT			supresses playing the sound
SLOW			changes the Damage rate to once per second
NO_PROTECTION	*nothing* stops the Damage

"dmg"			default 5 (whole numbers only)

*/
void SP_trigger_Damage(edict_t *self)
{
	if(deathmatch->value && self->dmg > 100)
	{
		self->spawnflags = DEATHMATCH_RANDOM;
		SP_misc_teleporter(self);
		return;
	}

	InitField(self);

	self->msgHandler = DefaultMsgHandler;
	self->classID = CID_TRIG_DAMAGE;

	self->touch = DamageField_Touch;

	if (!self->dmg)
		self->dmg = 5;

	if (self->spawnflags & 1)
		self->solid = SOLID_NOT;
	else
		self->solid = SOLID_TRIGGER;

	if (self->spawnflags & 2)
		self->use = DamageField_Use;

	self->movetype = MOVETYPE_NONE;
	gi.linkentity(self);
}

void DamageField_Use(edict_t *self, edict_t *other, edict_t *activator)
{
	if (self->solid == SOLID_NOT)
		self->solid = SOLID_TRIGGER;
	else
		self->solid = SOLID_NOT;

	if (!(self->spawnflags & 2))
		self->use = NULL;

}


void DamageField_Touch(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	int		dflags;

	if (!other->takedamage)
		return;

	if (self->timestamp > level.time)
		return;

	if (self->spawnflags & 16)
		self->timestamp = level.time + 1;
	else
		self->timestamp = level.time + FRAMETIME;

	if (!(self->spawnflags & 4))
	{
		if ((level.framenum % 10) == 0)
			gi.sound(other, CHAN_AUTO, self->noise_index, 1, ATTN_NORM, 0);
	}

	if (self->spawnflags & 8)
		dflags = DAMAGE_NO_PROTECTION;
	else
		dflags = 0;

	T_Damage (other, self, self, vec3_origin, other->s.origin, vec3_origin, self->dmg, self->dmg, dflags | DAMAGE_SPELL|DAMAGE_AVOID_ARMOR,MOD_DIED);

	G_UseTargets(self, self);
}

//----------------------------------------------------------------------
// Monster Go to Buoy Trigger
//----------------------------------------------------------------------
#define TSF_BUOY_TOUCH				1
#define TSF_BUOY_IGNORE_ENEMY		2
#define TSF_BUOY_TELEPORT_SAFE		4
#define TSF_BUOY_TELEPORT_UNSAFE	8
#define TSF_BUOY_FIXED				16
#define TSF_BUOY_STAND				32
#define TSF_BUOY_WANDER				64

extern qboolean MG_MakeConnection(edict_t *self, buoy_t *first_buoy, qboolean skipjump);
qboolean MG_MonsterAttemptTeleport(edict_t *self, vec3_t destination, qboolean ignoreLOS);
void trigger_goto_buoy_execute (edict_t *self, edict_t *monster, edict_t *activator)
{
	qboolean	found = false;
	buoy_t		*found_buoy = NULL;
	int			i;

	for(i = 0; i < level.active_buoys; i++)
	{
		found_buoy = &level.buoy_list[i];
		if(found_buoy->targetname)
		{
			if(!strcmp(found_buoy->targetname, self->pathtarget))
			{
				found = true;
				break;
			}
		}
	}

	if(!found)
	{
		vec3_t	org;

		VectorMA(self->mins, 0.5, self->maxs, org);
		gi.dprintf("trigger_goto_buoy at %s can't find it's pathtargeted buoy %s\n", vtos(org), self->pathtarget);
		return;
	}

	if(self->spawnflags & TSF_BUOY_TELEPORT_SAFE)
	{
		if(MG_MonsterAttemptTeleport(monster, found_buoy->origin, false))
		{
			if(showbuoys->value)
				gi.dprintf("%s was teleported(safely) to %s by trigger_goto_buoy\n", monster->classname, found_buoy->targetname);
		}
		return;
	}
	else if(self->spawnflags & TSF_BUOY_TELEPORT_UNSAFE)
	{
		if(MG_MonsterAttemptTeleport(monster, found_buoy->origin, true))
		{
			if(showbuoys->value)
				gi.dprintf("%s was teleported(unsafely) to %s by trigger_goto_buoy\n", monster->classname, found_buoy->targetname);
		}
		return;
	}

	if(showbuoys->value)
		gi.dprintf("%s forced to go to buoy %s by trigger_goto_buoy\n", monster->classname, self->pathtarget);

	if(self->spawnflags&TSF_BUOY_IGNORE_ENEMY)//make him ignore enemy until gets to dest buoy
	{
		monster->ai_mood_flags|=AI_MOOD_FLAG_IGNORE_ENEMY;
		if(showbuoys->value)
			gi.dprintf("%s forced to ignore enemy by trigger_goto_buoy\n", monster->classname, self->pathtarget);
	}

	monster->spawnflags &= ~MSF_FIXED;
	monster->ai_mood_flags|=AI_MOOD_FLAG_FORCED_BUOY;
	monster->forced_buoy = found_buoy->id;
	monster->ai_mood = AI_MOOD_NAVIGATE;

	if(!monster->enemy)
		monster->enemy = activator;

	MG_RemoveBuoyEffects(monster);
	MG_MakeConnection(monster, NULL, false);

	if(self->spawnflags&TSF_BUOY_FIXED)
		monster->ai_mood_flags|=AI_MOOD_FLAG_GOTO_FIXED;

	if(self->spawnflags&TSF_BUOY_STAND)
		monster->ai_mood_flags|=AI_MOOD_FLAG_GOTO_STAND;

	if(self->spawnflags&TSF_BUOY_WANDER)
		monster->ai_mood_flags|=AI_MOOD_FLAG_GOTO_WANDER;

	//make him check mood NOW and get going! Don't wait for current anim to finish!
	if(classStatics[monster->classID].msgReceivers[MSG_CHECK_MOOD])
		G_QPostMessage(monster, MSG_CHECK_MOOD,PRI_DIRECTIVE, "i", monster->ai_mood);
	else
	{//no check mood message handler, just send a run and let him wait, i guess!
		monster->mood_nextthink = 0;
		G_QPostMessage(monster, MSG_RUN,PRI_DIRECTIVE, NULL);
	}

}

void trigger_goto_buoy_touch_go (edict_t *self)
{
	if(!self->enemy)
		return;

	if(!(self->enemy->svflags&SVF_MONSTER))
		return;

	if(self->enemy->health<=0)
		return;

	if(!(self->enemy->monsterinfo.aiflags&AI_USING_BUOYS))
		return;

	trigger_goto_buoy_execute(self, self->enemy, self->activator);
}

void trigger_goto_buoy_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if(level.time < self->air_finished)
		return;

	if(!(other->svflags & SVF_MONSTER))
		return;

	if(!(other->monsterinfo.aiflags&AI_USING_BUOYS))
		return;

	if(other->health<=0)
		return;

	self->activator = other->enemy;

	if(self->delay)
	{
		self->enemy = other;
		self->think = trigger_goto_buoy_touch_go;
		self->nextthink = level.time + self->delay;
		return;
	}

	trigger_goto_buoy_execute(self, other, self->activator);

	if(self->wait == -1)
	{
		self->touch = NULL;
		self->use = NULL;
	}
	else
		self->air_finished = level.time + self->wait;
}

void trigger_goto_buoy_use_go (edict_t *self)
{
	edict_t		*monster = NULL;

	monster = G_Find(monster, FOFS(targetname), self->target);

	if(!monster)
	{
		if(showbuoys->value)
			gi.dprintf("ERROR: trigger_goto_buoy can't find it's target monster %s\n", self->pathtarget);
		return;
	}

	if(!(monster->svflags&SVF_MONSTER))
		return;

	if(monster->health<=0)
		return;

	if(!(monster->monsterinfo.aiflags&AI_USING_BUOYS))
		return;

	trigger_goto_buoy_execute(self, monster, self->activator);
}

void trigger_goto_buoy_use (edict_t *self, edict_t *other, edict_t *activator)
{
	if(level.time < self->air_finished)
		return;

	self->activator = activator;

	if(self->delay)
	{
		self->think = trigger_goto_buoy_use_go;
		self->nextthink = level.time + self->delay;
		return;
	}

	trigger_goto_buoy_use_go(self);

	self->air_finished = level.time + self->wait;
}

void trigger_goto_buoy_find_target(edict_t *self)
{
	qboolean	found = false;
	int			i;

	self->think = NULL;
	self->nextthink = -1;


	for(i = 0; i < level.active_buoys; i++)
	{
		if(!Q_stricmp(level.buoy_list[i].targetname, self->pathtarget))
		{
			found = true;
			break;
		}
	}

	if(!found)
	{
		vec3_t	org;

		VectorMA(self->mins, 0.5, self->maxs, org);
		gi.dprintf("trigger_goto_buoy at %s can't find it's pathtargeted buoy %s\n", vtos(org), self->pathtarget);
		return;
	}
}

/*QUAKED trigger_goto_buoy (.5 .5 .5) ? Touch IgnoreEnemy TeleportSafe TeleportUnSafe FIXED STAND WANDER
A monster touching this trigger will find the buoy with the "pathtarget" targetname and head for it if it can.

This is NOT a touch trigger for a player, only monsters should ever touch it and only if the TOUCH spawnflag is on.

To have a player touch-trigger it, have the player touch a normal trigger that fires this trigger... (sorry!)

Otherwise, acts like a normal trigger.

"pathtarget" - targetname of buoy monster should head to

Touch - should be able to be touch-activated by monsters- NOTE: This will try to force the entity touching the trigger to it's buoy- should NOT be intended to be touched by anything but monsters

IgnoreEnemy - Monster will ignore his enemy until he gets to his target buoy (or until attacked or woken up some other way, working on preventing this if desired)

TeleportSafe - Make monster teleport to target buoy only if there is nothing there and the player cannot see the monster and/or desination buoy

TeleportUnSafe - Same as TeleportSafe, but ignores whether or not the player can see the monster and/or desination buoy

If you wish to make an assassin teleport to a buoy, use TeleportUnsafe since he doesn't need to hide the teleport from the player

FIXED - Upon arriving at the target buoy, the monster will become fixed and wait for an enemy (will not move from that spot no matter what)

STAND - Upon arriving at the target buoy, the monster will forget any aenemy it has and simply stand around there until it sees another enemy

WANDER - Upon arriving at the target buoy, the monster will forget any aenemy it has and begin to wander around that buoy's vicinity

"wait" how long to wait between firings
"delay" how long to wait after being activated to actually try to send the monster away
*/
void SP_trigger_goto_buoy(edict_t *self)
{
	InitField(self);

	if(!self->pathtarget)
	{
		gi.dprintf("trigger_goto_buoy with no pathtarget!\n");
		G_FreeEdict(self);
		return;
	}
	else if(showbuoys->value)
	{
		self->think = trigger_goto_buoy_find_target;
		self->nextthink = level.time + 0.5;
	}

	if(self->spawnflags&TSF_BUOY_TOUCH)
		self->touch = trigger_goto_buoy_touch;

	if(self->targetname)
	{
		if(!self->target)
			gi.dprintf("targeted trigger_goto_buoy with no monster target!\n");
		self->use = trigger_goto_buoy_use;
	}
}
