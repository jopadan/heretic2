//
// Heretic II
// Copyright 1998 Raven Software
//
typedef enum AnimID_e
{
	CHICKEN_ANIM_STAND1,
	CHICKEN_ANIM_WALK,
	CHICKEN_ANIM_RUN,
	CHICKEN_ANIM_CLUCK,
	CHICKEN_ANIM_ATTACK,
	CHICKEN_ANIM_EAT,
	CHICKEN_ANIM_JUMP,
	NUM_ANIMS
} AnimID_t;

typedef enum SoundID_e
{
	//for the cluck animation
	SND_CLUCK1,
	SND_CLUCK2,

	//for the foot falls
	SND_CLAW,

	//for getting hit - even though right now, it dies immediately - they want this changed
	SND_PAIN1,
	SND_PAIN2,

	//for dying - we only ever get gibbed, so no other sound is required
	SND_DIE,

	//for biting the player
	SND_BITE1,
	SND_BITE2,
	SND_BITE3,

	//for pecking the ground
	SND_PECK1,
	SND_PECK2,

	//and lastly, I thought it might be cool to have some cries for when the chicken jumps
	SND_JUMP1,
	SND_JUMP2,
	SND_JUMP3,

	NUM_SOUNDS
} SoundID_t;

extern mmove_t chicken_move_stand1;
extern mmove_t chicken_move_walk;
extern mmove_t chicken_move_run;
extern mmove_t chicken_move_cluck;
extern mmove_t chicken_move_attack;
extern mmove_t chicken_move_eat;
extern mmove_t chicken_move_jump;

//Dummy anim to catch sequence leaks
extern mmove_t chickenp_move_dummy;

extern mmove_t chickenp_move_stand;
extern mmove_t chickenp_move_stand1;
extern mmove_t chickenp_move_stand2;
extern mmove_t chickenp_move_walk;
extern mmove_t chickenp_move_run;
extern mmove_t chickenp_move_back;
extern mmove_t chickenp_move_runb;
extern mmove_t chickenp_move_bite;
extern mmove_t chickenp_move_strafel;
extern mmove_t chickenp_move_strafer;
extern mmove_t chickenp_move_jump;
extern mmove_t chickenp_move_wjump;
extern mmove_t chickenp_move_wjumpb;
extern mmove_t chickenp_move_rjump;
extern mmove_t chickenp_move_rjumpb;
extern mmove_t chickenp_move_jump_loop;
extern mmove_t chickenp_move_attack;

void chicken_stand(edict_t *self, G_Message_t *msg);
void chicken_walk(edict_t *self, G_Message_t *msg);
void chicken_run(edict_t *self, G_Message_t *msg);
void chicken_attack(edict_t *self, G_Message_t *msg);
void chicken_death(edict_t *self, G_Message_t *msg);
void player_chicken_death(edict_t *self, G_Message_t *msg);
void chicken_eat(edict_t *self, G_Message_t *msg);
void chicken_cluck(edict_t *self, G_Message_t *msg);
void chicken_jump(edict_t *self, G_Message_t *msg);

void chicken_pause(edict_t *self);
void chicken_check(edict_t *self);
void chicken_eat_again(edict_t *self);
void chicken_bite (edict_t *self);

int make_chicken_jump(edict_t *self);
void chickenSound (edict_t *self, float channel, float sndindex, float atten);
