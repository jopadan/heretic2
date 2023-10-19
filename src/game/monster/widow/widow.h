/*
 * Copyright (C) 1997-2001 Id Software, Inc.
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
 * Black Widow (stage 1) animations.
 *
 * =======================================================================
 */

#define FRAME_idle01          	0
#define FRAME_idle02          	1
#define FRAME_idle03          	2
#define FRAME_idle04          	3
#define FRAME_idle05          	4
#define FRAME_idle06          	5
#define FRAME_idle07          	6
#define FRAME_idle08          	7
#define FRAME_idle09          	8
#define FRAME_idle10          	9
#define FRAME_idle11          	10
#define FRAME_walk01          	11
#define FRAME_walk02          	12
#define FRAME_walk03          	13
#define FRAME_walk04          	14
#define FRAME_walk05          	15
#define FRAME_walk06          	16
#define FRAME_walk07          	17
#define FRAME_walk08          	18
#define FRAME_walk09          	19
#define FRAME_walk10          	20
#define FRAME_walk11          	21
#define FRAME_walk12          	22
#define FRAME_walk13          	23
#define FRAME_run01           	24
#define FRAME_run02           	25
#define FRAME_run03           	26
#define FRAME_run04           	27
#define FRAME_run05           	28
#define FRAME_run06           	29
#define FRAME_run07           	30
#define FRAME_run08           	31
#define FRAME_firea01         	32
#define FRAME_firea02         	33
#define FRAME_firea03         	34
#define FRAME_firea04         	35
#define FRAME_firea05         	36
#define FRAME_firea06         	37
#define FRAME_firea07         	38
#define FRAME_firea08         	39
#define FRAME_firea09         	40
#define FRAME_fireb01         	41
#define FRAME_fireb02         	42
#define FRAME_fireb03         	43
#define FRAME_fireb04         	44
#define FRAME_fireb05         	45
#define FRAME_fireb06         	46
#define FRAME_fireb07         	47
#define FRAME_fireb08         	48
#define FRAME_fireb09         	49
#define FRAME_firec01         	50
#define FRAME_firec02         	51
#define FRAME_firec03         	52
#define FRAME_firec04         	53
#define FRAME_firec05         	54
#define FRAME_firec06         	55
#define FRAME_firec07         	56
#define FRAME_firec08         	57
#define FRAME_firec09         	58
#define FRAME_fired01         	59
#define FRAME_fired02         	60
#define FRAME_fired02a        	61
#define FRAME_fired03         	62
#define FRAME_fired04         	63
#define FRAME_fired05         	64
#define FRAME_fired06         	65
#define FRAME_fired07         	66
#define FRAME_fired08         	67
#define FRAME_fired09         	68
#define FRAME_fired10         	69
#define FRAME_fired11         	70
#define FRAME_fired12         	71
#define FRAME_fired13         	72
#define FRAME_fired14         	73
#define FRAME_fired15         	74
#define FRAME_fired16         	75
#define FRAME_fired17         	76
#define FRAME_fired18         	77
#define FRAME_fired19         	78
#define FRAME_fired20         	79
#define FRAME_fired21         	80
#define FRAME_fired22         	81
#define FRAME_spawn01         	82
#define FRAME_spawn02         	83
#define FRAME_spawn03         	84
#define FRAME_spawn04         	85
#define FRAME_spawn05         	86
#define FRAME_spawn06         	87
#define FRAME_spawn07         	88
#define FRAME_spawn08         	89
#define FRAME_spawn09         	90
#define FRAME_spawn10         	91
#define FRAME_spawn11         	92
#define FRAME_spawn12         	93
#define FRAME_spawn13         	94
#define FRAME_spawn14         	95
#define FRAME_spawn15         	96
#define FRAME_spawn16         	97
#define FRAME_spawn17         	98
#define FRAME_spawn18         	99
#define FRAME_pain01          	100
#define FRAME_pain02          	101
#define FRAME_pain03          	102
#define FRAME_pain04          	103
#define FRAME_pain05          	104
#define FRAME_pain06          	105
#define FRAME_pain07          	106
#define FRAME_pain08          	107
#define FRAME_pain09          	108
#define FRAME_pain10          	109
#define FRAME_pain11          	110
#define FRAME_pain12          	111
#define FRAME_pain13          	112
#define FRAME_pain201         	113
#define FRAME_pain202         	114
#define FRAME_pain203         	115
#define FRAME_transa01        	116
#define FRAME_transa02        	117
#define FRAME_transa03        	118
#define FRAME_transa04        	119
#define FRAME_transa05        	120
#define FRAME_transb01        	121
#define FRAME_transb02        	122
#define FRAME_transb03        	123
#define FRAME_transb04        	124
#define FRAME_transb05        	125
#define FRAME_transc01        	126
#define FRAME_transc02        	127
#define FRAME_transc03        	128
#define FRAME_transc04        	129
#define FRAME_death01         	130
#define FRAME_death02         	131
#define FRAME_death03         	132
#define FRAME_death04         	133
#define FRAME_death05         	134
#define FRAME_death06         	135
#define FRAME_death07         	136
#define FRAME_death08         	137
#define FRAME_death09         	138
#define FRAME_death10         	139
#define FRAME_death11         	140
#define FRAME_death12         	141
#define FRAME_death13         	142
#define FRAME_death14         	143
#define FRAME_death15         	144
#define FRAME_death16         	145
#define FRAME_death17         	146
#define FRAME_death18         	147
#define FRAME_death19         	148
#define FRAME_death20         	149
#define FRAME_death21         	150
#define FRAME_death22         	151
#define FRAME_death23         	152
#define FRAME_death24         	153
#define FRAME_death25         	154
#define FRAME_death26         	155
#define FRAME_death27         	156
#define FRAME_death28         	157
#define FRAME_death29         	158
#define FRAME_death30         	159
#define FRAME_death31         	160
#define FRAME_kick01          	161
#define FRAME_kick02          	162
#define FRAME_kick03          	163
#define FRAME_kick04          	164
#define FRAME_kick05          	165
#define FRAME_kick06          	166
#define FRAME_kick07          	167
#define FRAME_kick08          	168
#define MODEL_SCALE				2.000000
