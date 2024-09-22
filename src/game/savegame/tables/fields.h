/*
 * Copyright (C) 1997-2001 Id Software, Inc.
 * Copyright (C) 2011 Yamagi Burmeister
 * Copyright (c) ZeniMax Media Inc.
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
 * Game fields to be saved.
 *
 * =======================================================================
 */

{"classname", FOFS(classname), F_LSTRING},
{"model", FOFS(model), F_LSTRING},
{"spawnflags", FOFS(spawnflags), F_INT},
{"speed", FOFS(speed), F_FLOAT},
{"accel", FOFS(accel), F_FLOAT},
{"decel", FOFS(decel), F_FLOAT},
{"target", FOFS(target), F_LSTRING},
{"targetname", FOFS(targetname), F_LSTRING},
{"pathtarget", FOFS(pathtarget), F_LSTRING},
{"deathtarget", FOFS(deathtarget), F_LSTRING},
{"killtarget", FOFS(killtarget), F_LSTRING},
{"combattarget", FOFS(combattarget), F_LSTRING},
{"message", FOFS(message), F_LSTRING},
{"team", FOFS(team), F_LSTRING},
{"wait", FOFS(wait), F_FLOAT},
{"delay", FOFS(delay), F_FLOAT},
{"random", FOFS(random), F_FLOAT},
{"style", FOFS(style), F_INT},
{"count", FOFS(count), F_INT},
{"health", FOFS(health), F_INT},
{"sounds", FOFS(sounds), F_INT},
{"light", 0, F_IGNORE},
{"dmg", FOFS(dmg), F_INT},
{"mass", FOFS(mass), F_INT},
{"volume", FOFS(volume), F_FLOAT},
{"attenuation", FOFS(attenuation), F_FLOAT},
{"map", FOFS(map), F_LSTRING},
{"origin", FOFS(s.origin), F_VECTOR},
{"angles", FOFS(s.angles), F_VECTOR},
{"angle", FOFS(s.angles), F_ANGLEHACK},
{"skinnum", FOFS(s.skinnum), F_INT},
{"time", FOFS(time), F_FLOAT},
{"text_msg", FOFS(text_msg), F_LSTRING},
{"jumptarget", FOFS(jumptarget), F_LSTRING},
{"scripttarget", FOFS(scripttarget), F_LSTRING},
{"materialtype", FOFS(materialtype), F_INT},
{"scale", FOFS(s.scale), F_FLOAT},
{"color", FOFS(s.color), F_RGBA},
{"frame", FOFS(s.frame), F_INT},
{"mintel", FOFS(mintel), F_INT},
{"melee_range", FOFS(melee_range), F_FLOAT},
{"missile_range", FOFS(missile_range), F_FLOAT},
{"min_missile_range", FOFS(min_missile_range), F_FLOAT},
{"bypass_missile_chance", FOFS(bypass_missile_chance), F_INT},
{"jump_chance", FOFS(jump_chance), F_INT},
{"wakeup_distance", FOFS(wakeup_distance), F_FLOAT},
{"c_mode", FOFS(monsterinfo.c_mode), F_INT, F_INT},
{"homebuoy", FOFS(homebuoy), F_LSTRING},
{"wakeup_target", FOFS(wakeup_target), F_LSTRING},
{"pain_target", FOFS(pain_target), F_LSTRING},

// temp spawn vars -- only valid when the spawn function is called
{"lip", STOFS(lip), F_INT, FFL_SPAWNTEMP},
{"distance", STOFS(distance), F_INT, FFL_SPAWNTEMP},
{"height", STOFS(height), F_INT, FFL_SPAWNTEMP},
{"noise", STOFS(noise), F_LSTRING, FFL_SPAWNTEMP},
{"pausetime", STOFS(pausetime), F_FLOAT, FFL_SPAWNTEMP},
{"item", STOFS(item), F_LSTRING, FFL_SPAWNTEMP},
{"gravity", STOFS(gravity), F_LSTRING, FFL_SPAWNTEMP},
{"sky", STOFS(sky), F_LSTRING, FFL_SPAWNTEMP},
{"skyrotate", STOFS(skyrotate), F_FLOAT, FFL_SPAWNTEMP},
{"skyaxis", STOFS(skyaxis), F_VECTOR, FFL_SPAWNTEMP},
{"minyaw", STOFS(minyaw), F_FLOAT, FFL_SPAWNTEMP},
{"maxyaw", STOFS(maxyaw), F_FLOAT, FFL_SPAWNTEMP},
{"minpitch", STOFS(minpitch), F_FLOAT, FFL_SPAWNTEMP},
{"maxpitch", STOFS(maxpitch), F_FLOAT, FFL_SPAWNTEMP},
{"nextmap", STOFS(nextmap), F_LSTRING, FFL_SPAWNTEMP},
{"rotate", STOFS(rotate), F_INT, FFL_SPAWNTEMP},
{"target2", FOFS(target2), F_LSTRING},
{"pathtargetname",  FOFS(pathtargetname), F_LSTRING},
{"zangle", STOFS(zangle), F_FLOAT, FFL_SPAWNTEMP},
{"file", STOFS(file), F_LSTRING, FFL_SPAWNTEMP},
{"radius", STOFS(radius), F_INT, FFL_SPAWNTEMP},
{"offensive", STOFS(offensive), F_INT, FFL_SPAWNTEMP},
{"defensive", STOFS(defensive), F_INT, FFL_SPAWNTEMP},
{"spawnflags2", STOFS(spawnflags2), F_INT, FFL_SPAWNTEMP},
{"cooptimeout", STOFS(cooptimeout), F_INT, FFL_SPAWNTEMP},

{"script", STOFS(script), F_LSTRING, FFL_SPAWNTEMP},
{"parm1", STOFS(parms[0]), F_LSTRING, FFL_SPAWNTEMP},
{"parm2", STOFS(parms[1]), F_LSTRING, FFL_SPAWNTEMP},
{"parm3", STOFS(parms[2]), F_LSTRING, FFL_SPAWNTEMP},
{"parm4", STOFS(parms[3]), F_LSTRING, FFL_SPAWNTEMP},
{"parm5", STOFS(parms[4]), F_LSTRING, FFL_SPAWNTEMP},
{"parm6", STOFS(parms[5]), F_LSTRING, FFL_SPAWNTEMP},
{"parm7", STOFS(parms[6]), F_LSTRING, FFL_SPAWNTEMP},
{"parm8", STOFS(parms[7]), F_LSTRING, FFL_SPAWNTEMP},
{"parm9", STOFS(parms[8]), F_LSTRING, FFL_SPAWNTEMP},
{"parm10", STOFS(parms[9]), F_LSTRING, FFL_SPAWNTEMP},
{0, 0, 0, 0}
