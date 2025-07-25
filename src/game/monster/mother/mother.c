//==============================================================================
//
// m_mother.c
//
// Heretic II
// Copyright 1998 Raven Software
//
//
//	AI :
//
//	STAND1		: Looking straight ahead
//
//
//==============================================================================

#include "../../header/local.h"
#include "../../header/utilities.h"
#include "../../header/g_defaultmessagehandler.h"
#include "../../header/g_monster.h"
#include "../../common/fx.h"
#include "../../common/h2rand.h"
#include "../../header/buoy.h"

#include "mother.h"
#include "mother_anim.h"
#include "../../header/g_misc.h"
#include "../../character/ai.h"
#include "../stats/stats.h"


/*----------------------------------------------------------------------
-----------------------------------------------------------------------*/
static mmove_t *animations[MOTHER_NUM_ANIMS] =
{
	&mother_move_pain,
	&mother_move_stand,
};

static int sounds[NUM_SOUNDS];
static ClassResourceInfo_t resInfo;


void mother_growl(edict_t *self)
{
	int chance;

	chance = irand(0,2);

	if (chance == 1)
		gi.sound(self, CHAN_BODY, sounds[SND_GROWL1], 1, ATTN_NORM, 0);
	else if (chance == 2)
		gi.sound(self, CHAN_BODY, sounds[SND_GROWL2], 1, ATTN_NORM, 0);
}

void mother_pain(edict_t *self, G_Message_t *msg)
{
	int				temp, damage;
	qboolean		force_pain;

	G_ParseMsgParms(msg, "eeiii", &temp, &temp, &force_pain, &damage, &temp);

	if (self->pain_debounce_time < level.time)
	{
		self->pain_debounce_time = level.time + 1;
		SetAnim(self, MOTHER_ANIM_PAIN);
	}

	gi.sound(self, CHAN_BODY, sounds[SND_PAIN], 1, ATTN_NORM, 0);
}



/*-------------------------------------------------------------------------
-------------------------------------------------------------------------*/
void mother_stand(edict_t *self, G_Message_t *msg)
{
	SetAnim(self, MOTHER_ANIM_STAND);

	return;
}

void mother_pause (edict_t *self)
{
	G_QPostMessage(self, MSG_STAND, PRI_DIRECTIVE, NULL);
}


void mother_gib(edict_t *self, G_Message_t *msg)
{
	gi.sound(self, CHAN_BODY, sounds[SND_GIB], 1, ATTN_NORM, 0);
	self->think = BecomeDebris;
	self->nextthink = level.time + 0.1;
}

/*-------------------------------------------------------------------------
	MotherStaticsInit
-------------------------------------------------------------------------*/
void MotherStaticsInit()
{

	classStatics[CID_MOTHER].msgReceivers[MSG_STAND] = mother_stand;
	classStatics[CID_MOTHER].msgReceivers[MSG_PAIN] = mother_pain;
	classStatics[CID_MOTHER].msgReceivers[MSG_DEATH] = mother_gib;

	sounds[SND_GROWL1] = gi.soundindex("monsters/insect/growlf1.wav");
	sounds[SND_GROWL2] = gi.soundindex("monsters/insect/growlf2.wav");
	sounds[SND_PAIN] = gi.soundindex("monsters/insect/painf.wav");
	sounds[SND_GIB] = gi.soundindex("monsters/insect/gib.wav");

	resInfo.numAnims = MOTHER_NUM_ANIMS;
	resInfo.animations = animations;

	resInfo.modelIndex = gi.modelindex("models/monsters/mother/tris.fm");

	resInfo.numSounds = NUM_SOUNDS;
	resInfo.sounds = sounds;

	classStatics[CID_MOTHER].resInfo = &resInfo;
}

/*QUAKED monster_tcheckrik_mothers (1 .5 0) (-40 -40 -75) (40 40 75)
Momma egg layer

"pain_target" - monsters will fire this target the first time it gets hurt (only once)
*/
void SP_monster_tcheckrik_mothers (edict_t *self)
{

	self->classID = CID_MOTHER;

	if (!monster_start(self))
		return;					// Failed initialization

	self->msgHandler = DefaultMsgHandler;
	self->think = walkmonster_start_go;

	if (!self->health)
	{
		self->health = PLAGUEELF_HEALTH;
	}

	self->mass = 2000;
	self->yaw_speed = 20;

	self->movetype = MOVETYPE_STATIC;
	VectorClear(self->knockbackvel);

	self->solid=SOLID_BBOX;

	VectorSet(self->mins, -40, -40, -75);
	VectorSet(self->maxs,  40,  40,  75);
	self->viewheight = self->maxs[2]*0.8;

	self->s.modelindex = classStatics[CID_MOTHER].resInfo->modelIndex;

	if (!self->rrs.scale[0] ||
		!self->rrs.scale[1] ||
		!self->rrs.scale[2])
	{
		self->monsterinfo.scale = MODEL_SCALE;
		VectorSet(self->rrs.scale,
			self->monsterinfo.scale,
			self->monsterinfo.scale,
			self->monsterinfo.scale);
	}

	self->materialtype = MAT_INSECT;

	self->svflags |= SVF_WAIT_NOTSOLID;

}
