//
// Heretic II
// Copyright 1998 Raven Software
//
typedef enum AnimID_e
{
	RAT_ANIM_EATING1,
	RAT_ANIM_EATING2,
	RAT_ANIM_EATING3,
	RAT_ANIM_STAND1,
	RAT_ANIM_STAND2,
	RAT_ANIM_STAND3,
	RAT_ANIM_STAND4,
	RAT_ANIM_STAND5,
	RAT_ANIM_STAND6,
	RAT_ANIM_STAND7,
	RAT_ANIM_STAND8,
	RAT_ANIM_WATCH1,
	RAT_ANIM_WATCH2,
	RAT_ANIM_WALK1,
	RAT_ANIM_RUN1,
	RAT_ANIM_RUN2,
	RAT_ANIM_RUN3,
	RAT_ANIM_MELEE1,
	RAT_ANIM_MELEE2,
	RAT_ANIM_MELEE3,
	RAT_ANIM_PAIN1,
	RAT_ANIM_DIE1,
	RAT_ANIM_DIE2,
	RAT_NUM_ANIMS
} AnimID_t;

typedef enum SoundID_e
{
	SND_BITEHIT1,
	SND_BITEMISS1,
	SND_BITEMISS2,

	SND_SCRATCH,

	SND_HISS,

	SND_PAIN1,
	SND_PAIN2,

	SND_CHATTER1,
	SND_CHATTER2,
	SND_CHATTER3,

	SND_CHEW1,
	SND_CHEW2,
	SND_CHEW3,

	SND_SWALLOW,

	SND_DIE,
	SND_GIB,
	NUM_SOUNDS
} SoundID_t;

extern mmove_t rat_move_eat1;
extern mmove_t rat_move_eat2;
extern mmove_t rat_move_eat3;
extern mmove_t rat_move_stand1;
extern mmove_t rat_move_stand2;
extern mmove_t rat_move_stand3;
extern mmove_t rat_move_stand4;
extern mmove_t rat_move_stand5;
extern mmove_t rat_move_stand6;
extern mmove_t rat_move_stand7;
extern mmove_t rat_move_stand8;
extern mmove_t rat_move_watch1;
extern mmove_t rat_move_watch2;
extern mmove_t rat_move_walk1;
extern mmove_t rat_move_run1;
extern mmove_t rat_move_run2;
extern mmove_t rat_move_run3;
extern mmove_t rat_move_melee1;
extern mmove_t rat_move_melee2;
extern mmove_t rat_move_melee3;
extern mmove_t rat_move_pain1;
extern mmove_t rat_move_death1;
extern mmove_t rat_move_death2;

void rat_use(edict_t *self, edict_t *other, edict_t *activator);
void rat_touch(edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf);

void ratdeathsqueal (edict_t *self);
void ratsqueal (edict_t *self);
void ratbite (edict_t *self);
void rat_pain_init(edict_t *self);
void rat_runorder(edict_t *self);
void rat_standorder(edict_t *self);
void rat_pause (edict_t *self);

int rat_standdecision (edict_t *self);
int rat_eatdecision (edict_t *self);
void rat_eatorder(edict_t *self);
void rat_runorder(edict_t *self);
void rathiss (edict_t *self);
void ratscratch (edict_t *self);
void ratchatter (edict_t *self);
void ratchew (edict_t *self);
void ratswallow (edict_t *self);
void ratjump(edict_t *self);

void rat_init (void);

void rat_pain(edict_t *self, G_Message_t *msg);
void rat_death(edict_t *self, G_Message_t *msg);
void rat_run(edict_t *self, G_Message_t *msg);
void rat_walk(edict_t *self, G_Message_t *msg);
void rat_melee(edict_t *self, G_Message_t *msg);
void rat_watch(edict_t *self, G_Message_t *msg);
void rat_stand(edict_t *self, G_Message_t *msg);
void rat_eat(edict_t *self, G_Message_t *msg);

void rat_ai_eat(edict_t *self, float dist);
void rat_ai_stand(edict_t *self, float dist);
