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
 * Fixbot animations
 *
 * =======================================================================
 */

#define FRAME_charging_01 0
#define FRAME_charging_02 1
#define FRAME_charging_03 2
#define FRAME_charging_04 3
#define FRAME_charging_05 4
#define FRAME_charging_06 5
#define FRAME_charging_07 6
#define FRAME_charging_08 7
#define FRAME_charging_09 8
#define FRAME_charging_10 9
#define FRAME_charging_11 10
#define FRAME_charging_12 11
#define FRAME_charging_13 12
#define FRAME_charging_14 13
#define FRAME_charging_15 14
#define FRAME_charging_16 15
#define FRAME_charging_17 16
#define FRAME_charging_18 17
#define FRAME_charging_19 18
#define FRAME_charging_20 19
#define FRAME_charging_21 20
#define FRAME_charging_22 21
#define FRAME_charging_23 22
#define FRAME_charging_24 23
#define FRAME_charging_25 24
#define FRAME_charging_26 25
#define FRAME_charging_27 26
#define FRAME_charging_28 27
#define FRAME_charging_29 28
#define FRAME_charging_30 29
#define FRAME_charging_31 30
#define FRAME_landing_01 31
#define FRAME_landing_02 32
#define FRAME_landing_03 33
#define FRAME_landing_04 34
#define FRAME_landing_05 35
#define FRAME_landing_06 36
#define FRAME_landing_07 37
#define FRAME_landing_08 38
#define FRAME_landing_09 39
#define FRAME_landing_10 40
#define FRAME_landing_11 41
#define FRAME_landing_12 42
#define FRAME_landing_13 43
#define FRAME_landing_14 44
#define FRAME_landing_15 45
#define FRAME_landing_16 46
#define FRAME_landing_17 47
#define FRAME_landing_18 48
#define FRAME_landing_19 49
#define FRAME_landing_20 50
#define FRAME_landing_21 51
#define FRAME_landing_22 52
#define FRAME_landing_23 53
#define FRAME_landing_24 54
#define FRAME_landing_25 55
#define FRAME_landing_26 56
#define FRAME_landing_27 57
#define FRAME_landing_28 58
#define FRAME_landing_29 59
#define FRAME_landing_30 60
#define FRAME_landing_31 61
#define FRAME_landing_32 62
#define FRAME_landing_33 63
#define FRAME_landing_34 64
#define FRAME_landing_35 65
#define FRAME_landing_36 66
#define FRAME_landing_37 67
#define FRAME_landing_38 68
#define FRAME_landing_39 69
#define FRAME_landing_40 70
#define FRAME_landing_41 71
#define FRAME_landing_42 72
#define FRAME_landing_43 73
#define FRAME_landing_44 74
#define FRAME_landing_45 75
#define FRAME_landing_46 76
#define FRAME_landing_47 77
#define FRAME_landing_48 78
#define FRAME_landing_49 79
#define FRAME_landing_50 80
#define FRAME_landing_51 81
#define FRAME_landing_52 82
#define FRAME_landing_53 83
#define FRAME_landing_54 84
#define FRAME_landing_55 85
#define FRAME_landing_56 86
#define FRAME_landing_57 87
#define FRAME_landing_58 88
#define FRAME_pushback_01 89
#define FRAME_pushback_02 90
#define FRAME_pushback_03 91
#define FRAME_pushback_04 92
#define FRAME_pushback_05 93
#define FRAME_pushback_06 94
#define FRAME_pushback_07 95
#define FRAME_pushback_08 96
#define FRAME_pushback_09 97
#define FRAME_pushback_10 98
#define FRAME_pushback_11 99
#define FRAME_pushback_12 100
#define FRAME_pushback_13 101
#define FRAME_pushback_14 102
#define FRAME_pushback_15 103
#define FRAME_pushback_16 104
#define FRAME_takeoff_01 105
#define FRAME_takeoff_02 106
#define FRAME_takeoff_03 107
#define FRAME_takeoff_04 108
#define FRAME_takeoff_05 109
#define FRAME_takeoff_06 110
#define FRAME_takeoff_07 111
#define FRAME_takeoff_08 112
#define FRAME_takeoff_09 113
#define FRAME_takeoff_10 114
#define FRAME_takeoff_11 115
#define FRAME_takeoff_12 116
#define FRAME_takeoff_13 117
#define FRAME_takeoff_14 118
#define FRAME_takeoff_15 119
#define FRAME_takeoff_16 120
#define FRAME_ambient_01 121
#define FRAME_ambient_02 122
#define FRAME_ambient_03 123
#define FRAME_ambient_04 124
#define FRAME_ambient_05 125
#define FRAME_ambient_06 126
#define FRAME_ambient_07 127
#define FRAME_ambient_08 128
#define FRAME_ambient_09 129
#define FRAME_ambient_10 130
#define FRAME_ambient_11 131
#define FRAME_ambient_12 132
#define FRAME_ambient_13 133
#define FRAME_ambient_14 134
#define FRAME_ambient_15 135
#define FRAME_ambient_16 136
#define FRAME_ambient_17 137
#define FRAME_ambient_18 138
#define FRAME_ambient_19 139
#define FRAME_paina_01 140
#define FRAME_paina_02 141
#define FRAME_paina_03 142
#define FRAME_paina_04 143
#define FRAME_paina_05 144
#define FRAME_paina_06 145
#define FRAME_painb_01 146
#define FRAME_painb_02 147
#define FRAME_painb_03 148
#define FRAME_painb_04 149
#define FRAME_painb_05 150
#define FRAME_painb_06 151
#define FRAME_painb_07 152
#define FRAME_painb_08 153
#define FRAME_pickup_01 154
#define FRAME_pickup_02 155
#define FRAME_pickup_03 156
#define FRAME_pickup_04 157
#define FRAME_pickup_05 158
#define FRAME_pickup_06 159
#define FRAME_pickup_07 160
#define FRAME_pickup_08 161
#define FRAME_pickup_09 162
#define FRAME_pickup_10 163
#define FRAME_pickup_11 164
#define FRAME_pickup_12 165
#define FRAME_pickup_13 166
#define FRAME_pickup_14 167
#define FRAME_pickup_15 168
#define FRAME_pickup_16 169
#define FRAME_pickup_17 170
#define FRAME_pickup_18 171
#define FRAME_pickup_19 172
#define FRAME_pickup_20 173
#define FRAME_pickup_21 174
#define FRAME_pickup_22 175
#define FRAME_pickup_23 176
#define FRAME_pickup_24 177
#define FRAME_pickup_25 178
#define FRAME_pickup_26 179
#define FRAME_pickup_27 180
#define FRAME_freeze_01 181
#define FRAME_shoot_01 182
#define FRAME_shoot_02 183
#define FRAME_shoot_03 184
#define FRAME_shoot_04 185
#define FRAME_shoot_05 186
#define FRAME_shoot_06 187
#define FRAME_weldstart_01 188
#define FRAME_weldstart_02 189
#define FRAME_weldstart_03 190
#define FRAME_weldstart_04 191
#define FRAME_weldstart_05 192
#define FRAME_weldstart_06 193
#define FRAME_weldstart_07 194
#define FRAME_weldstart_08 195
#define FRAME_weldstart_09 196
#define FRAME_weldstart_10 197
#define FRAME_weldmiddle_01 198
#define FRAME_weldmiddle_02 199
#define FRAME_weldmiddle_03 200
#define FRAME_weldmiddle_04 201
#define FRAME_weldmiddle_05 202
#define FRAME_weldmiddle_06 203
#define FRAME_weldmiddle_07 204
#define FRAME_weldend_01 205
#define FRAME_weldend_02 206
#define FRAME_weldend_03 207
#define FRAME_weldend_04 208
#define FRAME_weldend_05 209
#define FRAME_weldend_06 210
#define FRAME_weldend_07 211

#define MODEL_SCALE 1.000000
