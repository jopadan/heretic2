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
 * Baracuda Shark animations.
 *
 * =======================================================================
 */

#define FRAME_flpbit01        	0
#define FRAME_flpbit02        	1
#define FRAME_flpbit03        	2
#define FRAME_flpbit04        	3
#define FRAME_flpbit05        	4
#define FRAME_flpbit06        	5
#define FRAME_flpbit07        	6
#define FRAME_flpbit08        	7
#define FRAME_flpbit09        	8
#define FRAME_flpbit10        	9
#define FRAME_flpbit11        	10
#define FRAME_flpbit12        	11
#define FRAME_flpbit13        	12
#define FRAME_flpbit14        	13
#define FRAME_flpbit15        	14
#define FRAME_flpbit16        	15
#define FRAME_flpbit17        	16
#define FRAME_flpbit18        	17
#define FRAME_flpbit19        	18
#define FRAME_flpbit20        	19
#define FRAME_flptal01        	20
#define FRAME_flptal02        	21
#define FRAME_flptal03        	22
#define FRAME_flptal04        	23
#define FRAME_flptal05        	24
#define FRAME_flptal06        	25
#define FRAME_flptal07        	26
#define FRAME_flptal08        	27
#define FRAME_flptal09        	28
#define FRAME_flptal10        	29
#define FRAME_flptal11        	30
#define FRAME_flptal12        	31
#define FRAME_flptal13        	32
#define FRAME_flptal14        	33
#define FRAME_flptal15        	34
#define FRAME_flptal16        	35
#define FRAME_flptal17        	36
#define FRAME_flptal18        	37
#define FRAME_flptal19        	38
#define FRAME_flptal20        	39
#define FRAME_flptal21        	40
#define FRAME_flphor01        	41
#define FRAME_flphor02        	42
#define FRAME_flphor03        	43
#define FRAME_flphor04        	44
#define FRAME_flphor05        	45
#define FRAME_flphor06        	46
#define FRAME_flphor07        	47
#define FRAME_flphor08        	48
#define FRAME_flphor09        	49
#define FRAME_flphor10        	50
#define FRAME_flphor11        	51
#define FRAME_flphor12        	52
#define FRAME_flphor13        	53
#define FRAME_flphor14        	54
#define FRAME_flphor15        	55
#define FRAME_flphor16        	56
#define FRAME_flphor17        	57
#define FRAME_flphor18        	58
#define FRAME_flphor19        	59
#define FRAME_flphor20        	60
#define FRAME_flphor21        	61
#define FRAME_flphor22        	62
#define FRAME_flphor23        	63
#define FRAME_flphor24        	64
#define FRAME_flpver01        	65
#define FRAME_flpver02        	66
#define FRAME_flpver03        	67
#define FRAME_flpver04        	68
#define FRAME_flpver05        	69
#define FRAME_flpver06        	70
#define FRAME_flpver07        	71
#define FRAME_flpver08        	72
#define FRAME_flpver09        	73
#define FRAME_flpver10        	74
#define FRAME_flpver11        	75
#define FRAME_flpver12        	76
#define FRAME_flpver13        	77
#define FRAME_flpver14        	78
#define FRAME_flpver15        	79
#define FRAME_flpver16        	80
#define FRAME_flpver17        	81
#define FRAME_flpver18        	82
#define FRAME_flpver19        	83
#define FRAME_flpver20        	84
#define FRAME_flpver21        	85
#define FRAME_flpver22        	86
#define FRAME_flpver23        	87
#define FRAME_flpver24        	88
#define FRAME_flpver25        	89
#define FRAME_flpver26        	90
#define FRAME_flpver27        	91
#define FRAME_flpver28        	92
#define FRAME_flpver29        	93
#define FRAME_flppn101        	94
#define FRAME_flppn102        	95
#define FRAME_flppn103        	96
#define FRAME_flppn104        	97
#define FRAME_flppn105        	98
#define FRAME_flppn201        	99
#define FRAME_flppn202        	100
#define FRAME_flppn203        	101
#define FRAME_flppn204        	102
#define FRAME_flppn205        	103
#define FRAME_flpdth01        	104
#define FRAME_flpdth02        	105
#define FRAME_flpdth03        	106
#define FRAME_flpdth04        	107
#define FRAME_flpdth05        	108
#define FRAME_flpdth06        	109
#define FRAME_flpdth07        	110
#define FRAME_flpdth08        	111
#define FRAME_flpdth09        	112
#define FRAME_flpdth10        	113
#define FRAME_flpdth11        	114
#define FRAME_flpdth12        	115
#define FRAME_flpdth13        	116
#define FRAME_flpdth14        	117
#define FRAME_flpdth15        	118
#define FRAME_flpdth16        	119
#define FRAME_flpdth17        	120
#define FRAME_flpdth18        	121
#define FRAME_flpdth19        	122
#define FRAME_flpdth20        	123
#define FRAME_flpdth21        	124
#define FRAME_flpdth22        	125
#define FRAME_flpdth23        	126
#define FRAME_flpdth24        	127
#define FRAME_flpdth25        	128
#define FRAME_flpdth26        	129
#define FRAME_flpdth27        	130
#define FRAME_flpdth28        	131
#define FRAME_flpdth29        	132
#define FRAME_flpdth30        	133
#define FRAME_flpdth31        	134
#define FRAME_flpdth32        	135
#define FRAME_flpdth33        	136
#define FRAME_flpdth34        	137
#define FRAME_flpdth35        	138
#define FRAME_flpdth36        	139
#define FRAME_flpdth37        	140
#define FRAME_flpdth38        	141
#define FRAME_flpdth39        	142
#define FRAME_flpdth40        	143
#define FRAME_flpdth41        	144
#define FRAME_flpdth42        	145
#define FRAME_flpdth43        	146
#define FRAME_flpdth44        	147
#define FRAME_flpdth45        	148
#define FRAME_flpdth46        	149
#define FRAME_flpdth47        	150
#define FRAME_flpdth48        	151
#define FRAME_flpdth49        	152
#define FRAME_flpdth50        	153
#define FRAME_flpdth51        	154
#define FRAME_flpdth52        	155
#define FRAME_flpdth53        	156
#define FRAME_flpdth54        	157
#define FRAME_flpdth55        	158
#define FRAME_flpdth56        	159
#define MODEL_SCALE				1.000000
