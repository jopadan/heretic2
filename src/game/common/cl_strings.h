//
// Heretic II
// Copyright 1998 Raven Software
//
#ifndef QCOMMON_CL_STRINGS_H
#define QCOMMON_CL_STRINGS_H

// Top 2 bits are used for level of message
#define MESSAGE_MASK	0x1fff

typedef enum
{
	GM_CS_HEALTH		= 3,
	GM_CS_MANA			,
	GM_CS_SILVER		,
	GM_CS_REFLECT		,
	GM_CS_POWERUP		,
	GM_CS_GOLD			,
	GM_CS_BLADE			,
	GM_CS_GHOST			,
	GM_CS_SPEED			,
	GM_S_MANA	 		,
	GM_S_HEALTH			,
	GM_S_SILVER			,
	GM_S_LIGHT			,
	GM_S_POWERUP 		,
	GM_S_BLADE			,
	GM_S_GHOST			,
	GM_S_REFLECT 		,
	GM_S_LUNGS			,
	GM_S_GOLD			,
	GM_S_SPEED			,

	GM_HELLSTAFF		= 27,
	GM_FORCEBLAST		,
	GM_STORMBOW			,
	GM_SPHERE			,
	GM_PHOENIX			,
	GM_IRONDOOM			,
	GM_FIREWALL			,
	GM_TOME				,
	GM_RING				,
	GM_SHIELD			,
	GM_TELEPORT			,
	GM_MORPH			,
	GM_METEOR			,
	GM_OFFMANAS			,
	GM_OFFMANAB			,
	GM_DEFMANAS			,
	GM_DEFMANAB			,
	GM_COMBMANAS		,
	GM_COMBMANAB		,
	GM_STORMARROWS		,
	GM_PHOENARROWS		,
	GM_HELLORB			,
	GM_HEALTHVIAL		,
	GM_HEALTHPOTION		,
	GM_F_TOWNKEY		,
	GM_F_COG			,
	GM_F_SHIELD			,
	GM_F_POTION			,
	GM_F_CONT			,
	GM_F_CONTFULL		,
	GM_F_CRYSTAL		,
	GM_F_CANYONKEY		,
	GM_F_AMULET			,
	GM_F_SPEAR			,
	GM_F_GEM			,
	GM_F_CARTWHEEL		,
	GM_F_UNREFORE		,
	GM_F_REFORE			,
	GM_F_DUNGEONKEY		,
	GM_F_CLOUDKEY		,
	GM_F_HIGHKEY		,
	GM_F_SYMBOL			,
	GM_F_TOME			,
	GM_F_TAVERNKEY		,

	GM_NOFLYINGFIST		= 74,
	GM_NOHELLORBS		,
	GM_NOFORCE			,
	GM_NOSTORMBOW		,
	GM_NOSPHERE			,
	GM_NOPHOENIX		,
	GM_NOIRONDOOM		,
	GM_NOFIREWALL		,
	GM_NOTOME			,
	GM_NORING			,
	GM_NOSHIELD			,
	GM_NOTELEPORT		,
	GM_NOMORPH			,
	GM_NOMETEOR			,
	GM_NEED_TOWNKEY		,
	GM_NEED_COG			,
	GM_NEED_SHIELD		,
	GM_NEED_POTION		,
	GM_NEED_CONT		,
	GM_NEED_CONTFULL	,
	GM_NEED_CRYSTAL		,
	GM_NEED_CANYONKEY	,
	GM_NEED_AMULET		,
	GM_NEED_SPEAR		,
	GM_NEED_GEM			,
	GM_NEED_CARTWHEEL	,
	GM_NEED_UNREFORE	,
	GM_NEED_REFORE		,
	GM_NEED_DUNGEONKEY	,
	GM_NEED_CLOUDKEY	,
	GM_NEED_HIGHKEY		,
	GM_NEED_SYMBOL		,
	GM_NEED_TOME		,
	GM_NEED_TAVERNKEY	,

	GM_NOTUSABLE		= 111,
	GM_NOCHEATS			,
	GM_NOITEM			,
	GM_NOMANA			,
	GM_NOAMMO			,
	GM_SEQCOMPLETE		,
	GM_TOGO_1			,
	GM_TOGO_2			,
	GM_TOGO_3			,
	GM_TOGO_4			,
	GM_TOGO_5			,
	GM_TOGO_6			,
	GM_TOGO_7			,
	GM_TOGO_8			,
	GM_TOGO_9			,
	GM_TOGO_10			,
	GM_NEWOBJ			,

	GM_KILLED_SELF		= 129,
	GM_DIED				,
	GM_WASKILLED		,
	GM_ENTERED			,
	GM_TIMELIMIT		,
	GM_FRAGLIMIT		,
	GM_EXIT				,
	GM_DISCON			,
	GM_WASKICKED		,
	GM_WASBANNED		,
	GM_KICKED			,
	GM_BANNED			,
	GM_TIMEDOUT			,
	GM_OVERFLOW 		,
	GM_COOPTIMEOUT		,
	GM_COOPWAITCIN		,

	GM_OBIT_STAFF			= 148,
	GM_OBIT_STAFF1			,
	GM_OBIT_STAFF2			,
	GM_OBIT_FIREBALL		,
	GM_OBIT_FIREBALL1		,
	GM_OBIT_FIREBALL2		,
	GM_OBIT_MMISSILE		,
	GM_OBIT_MMISSILE1		,
	GM_OBIT_MMISSILE2		,
	GM_OBIT_SPHERE			,
	GM_OBIT_SPHERE1			,
	GM_OBIT_SPHERE2			,
	GM_OBIT_SPHERE_SPL		,
	GM_OBIT_SPHERE_SPL1		,
	GM_OBIT_SPHERE_SPL2		,
	GM_OBIT_IRONDOOM		,
	GM_OBIT_IRONDOOM1		,
	GM_OBIT_IRONDOOM2		,
	GM_OBIT_FIREWALL		,
	GM_OBIT_FIREWALL1		,
	GM_OBIT_FIREWALL2		,
	GM_OBIT_STORM			,
	GM_OBIT_STORM1			,
	GM_OBIT_STORM2			,
	GM_OBIT_PHOENIX			,
	GM_OBIT_PHOENIX1 		,
	GM_OBIT_PHOENIX2 		,
	GM_OBIT_PHOENIX_SPL		,
	GM_OBIT_PHOENIX_SPL1	,
	GM_OBIT_PHOENIX_SPL2	,
	GM_OBIT_HELLSTAFF		,
	GM_OBIT_HELLSTAFF1		,
	GM_OBIT_HELLSTAFF2		,

	GM_OBIT_P_STAFF			= 184,
	GM_OBIT_P_STAFF1		,
	GM_OBIT_P_STAFF2	   	,
	GM_OBIT_P_FIREBALL		,
	GM_OBIT_P_FIREBALL1		,
	GM_OBIT_P_FIREBALL2		,
	GM_OBIT_P_MMISSILE		,
	GM_OBIT_P_MMISSILE1		,
	GM_OBIT_P_MMISSILE2		,
	GM_OBIT_P_SPHERE	   	,
	GM_OBIT_P_SPHERE1	   	,
	GM_OBIT_P_SPHERE2	   	,
	GM_OBIT_P_SPHERE_SPL   	,
	GM_OBIT_P_SPHERE_SPL1  	,
	GM_OBIT_P_SPHERE_SPL2  	,
	GM_OBIT_P_IRONDOOM		,
	GM_OBIT_P_IRONDOOM1		,
	GM_OBIT_P_IRONDOOM2		,
	GM_OBIT_P_FIREWALL		,
	GM_OBIT_P_FIREWALL1		,
	GM_OBIT_P_FIREWALL2		,
	GM_OBIT_P_STORM			,
	GM_OBIT_P_STORM1	   	,
	GM_OBIT_P_STORM2	   	,
	GM_OBIT_P_PHOENIX	   	,
	GM_OBIT_P_PHOENIX1 		,
	GM_OBIT_P_PHOENIX2 		,
	GM_OBIT_P_PHOENIX_SPL  	,
	GM_OBIT_P_PHOENIX_SPL1	,
	GM_OBIT_P_PHOENIX_SPL2	,
	GM_OBIT_P_HELLSTAFF		,
	GM_OBIT_P_HELLSTAFF1 	,
	GM_OBIT_P_HELLSTAFF2 	,

	GM_OBIT_UNKNOWN			= 220,
	GM_OBIT_UNKNOWN1		,
	GM_OBIT_UNKNOWN2		,
	GM_OBIT_KILLEDSELF		,
	GM_OBIT_KILLEDSELF1		,
	GM_OBIT_KILLEDSELF2		,
	GM_OBIT_KICKED			,
	GM_OBIT_KICKED1			,
	GM_OBIT_KICKED2			,
	GM_OBIT_METEORS			,
	GM_OBIT_METEORS1		,
	GM_OBIT_METEORS2		,
	GM_OBIT_ROR				,
	GM_OBIT_ROR1			,
	GM_OBIT_ROR2			,
	GM_OBIT_SHIELD			,
	GM_OBIT_SHIELD1			,
	GM_OBIT_SHIELD2			,
	GM_OBIT_CHICKEN			,
	GM_OBIT_CHICKEN1		,
	GM_OBIT_CHICKEN2		,
	GM_OBIT_TELEFRAG		,
	GM_OBIT_TELEFRAG1		,
	GM_OBIT_TELEFRAG2		,
	GM_OBIT_WATER			,
	GM_OBIT_WATER1			,
	GM_OBIT_WATER2			,
	GM_OBIT_SLIME			,
	GM_OBIT_SLIME1			,
	GM_OBIT_SLIME2			,
	GM_OBIT_LAVA			,
	GM_OBIT_LAVA1			,
	GM_OBIT_LAVA2			,
	GM_OBIT_CRUSH			,
	GM_OBIT_CRUSH1			,
	GM_OBIT_CRUSH2			,
	GM_OBIT_FALLING			,
	GM_OBIT_FALLING1		,
	GM_OBIT_FALLING2		,
	GM_OBIT_SUICIDE			,
	GM_OBIT_SUICIDE1		,
	GM_OBIT_SUICIDE2		,
	GM_OBIT_BARREL			,
	GM_OBIT_BARREL1			,
	GM_OBIT_BARREL2			,
	GM_OBIT_EXIT			,
	GM_OBIT_EXIT1			,
	GM_OBIT_EXIT2			,
	GM_OBIT_DIED			,
	GM_OBIT_DIED1			,
	GM_OBIT_DIED2			,
	GM_OBIT_BLEED			,
	GM_OBIT_BLEED1			,
	GM_OBIT_BLEED2			,
	GM_OBIT_SPEAR			,
	GM_OBIT_SPEAR1			,
	GM_OBIT_SPEAR2			,
	GM_OBIT_BURNT			,
	GM_OBIT_BURNT1			,
	GM_OBIT_BURNT2			,
	GM_OBIT_EXPL			,
	GM_OBIT_EXPL1			,
	GM_OBIT_EXPL2			,

	GM_SIR_NATE_HIT_AGAIN0		= 286,
	GM_SIR_NATE_HIT_AGAIN1,
	GM_SIR_NATE_HIT_AGAIN2,
	GM_SIR_NATE_HIT_AGAIN3,
	GM_SIR_NATE_HIT_AGAIN4,
	GM_SIR_NATE_HIT_AGAIN5,
	GM_SIR_NATE_HIT_AGAIN6,
	GM_SIR_NATE_HIT_AGAIN7,
	GM_SIR_NATE_HIT_AGAIN8,
	GM_SIR_NATE_HIT_AGAIN9,
	GM_SIR_NATE_FAILURE,
	GM_SIR_NATE_SUCCESS,
	GM_SIR_NATE_FINISH,
	GM_SIR_NATE_INSTRUCTIONS0,
	GM_SIR_NATE_INSTRUCTIONS1,
	GM_SIR_NATE_INSTRUCTIONS2,
	GM_SIR_NATE_INSTRUCTIONS3,
	GM_SIR_NATE_GREETING,
	GM_SIR_NATE_GET_STARTED,
	GM_SIR_NATE_END,

	GM_SHUTUP					= 307,
	GM_NONAMECHANGE				,
	GM_NOKILL					,
	GM_CH_SERVERS				,
	GM_CH_SAVECFG				,
	GM_CH_SOUND					,

	GM_M_KELLCAVES				= 314,
	GM_M_DARKMIRE				,
	GM_M_KATLITK				,
	GM_M_MINES					,
	GM_M_DUNGEON				,
	GM_M_CLOUD					,
	GM_TORNADO					,
	GM_NOTORNADO				,
	GM_OBIT_TORN				,
	GM_OBIT_TORN1				,
	GM_OBIT_TORN2				,
	GM_OBIT_TORN_SELF			,
	GM_OBIT_TORN_SELF1			,
	GM_OBIT_TORN_SELF2 			,

	GM_COOP_RESTARTING			= 329,

	// Always keep these last to test alignment
	GM_HELP1					= 331,
	GM_HELP2
} GameMsg_t;

#endif
