/*
 * Copyright (C) 1997-2001 Id Software, Inc.
 * Copyright (C) 2016-2017 Daniel Gibson
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
 * Mesh handling
 *
 * =======================================================================
 */

#include "header/local.h"

#include "../files/DG_dynarr.h"

#define NUMVERTEXNORMALS 162
#define SHADEDOT_QUANT 16

/* precalculated dot products for quantized angles */
static float r_avertexnormal_dots[SHADEDOT_QUANT][256] = {
#include "../constants/anormtab.h"
};

typedef struct gl3_shadowinfo_s {
	vec3_t    lightspot;
	vec3_t    shadevector;
	dmdx_t*   paliashdr;
	entity_t* entity;
} gl3_shadowinfo_t;

DA_TYPEDEF(gl3_shadowinfo_t, ShadowInfoArray_t);
// collect all models casting shadows (each frame)
// to draw shadows last
static ShadowInfoArray_t shadowModels = {0};

DA_TYPEDEF(gl3_alias_vtx_t, AliasVtxArray_t);
DA_TYPEDEF(GLushort, UShortArray_t);
// dynamic arrays to batch all the data of a model, so we can render a model in one draw call
static AliasVtxArray_t vtxBuf = {0};
static UShortArray_t idxBuf = {0};

void
GL3_ShutdownMeshes(void)
{
	da_free(vtxBuf);
	da_free(idxBuf);

	da_free(shadowModels);
}

static void
DrawAliasFrameLerpCommands(dmdx_t *paliashdr, entity_t* entity, vec3_t shadelight,
	int *order, int *order_end, float* shadedots, float alpha, qboolean colorOnly,
	dxtrivertx_t *verts, vec4_t *s_lerped)
{
	// all the triangle fans and triangle strips of this model will be converted to
	// just triangles: the vertices stay the same and are batched in vtxBuf,
	// but idxBuf will contain indices to draw them all as GL_TRIANGLE
	// this way there's only one draw call (and two glBufferData() calls)
	// instead of (at least) dozens. *greatly* improves performance.

	// so first clear out the data from last call to this function
	// (the buffers are static global so we don't have malloc()/free() for each rendered model)
	da_clear(vtxBuf);
	da_clear(idxBuf);

	while (1)
	{
		GLushort nextVtxIdx = da_count(vtxBuf);
		GLenum type;
		int count;

		/* get the vertex count and primitive type */
		count = *order++;

		if (!count || order >= order_end)
		{
			break; /* done */
		}

		if (count < 0)
		{
			count = -count;

			type = GL_TRIANGLE_FAN;
		}
		else
		{
			type = GL_TRIANGLE_STRIP;
		}

		gl3_alias_vtx_t* buf = da_addn_uninit(vtxBuf, count);

		if (colorOnly)
		{
			int i;
			for(i=0; i<count; ++i)
			{
				int j=0;
				int index_xyz;
				gl3_alias_vtx_t* cur = &buf[i];
				index_xyz = order[2];
				order += 3;

				for(j=0; j<3; ++j)
				{
					cur->pos[j] = s_lerped[index_xyz][j];
					cur->color[j] = shadelight[j];
				}
				cur->color[3] = alpha;
			}
		}
		else
		{
			int i;
			for(i=0; i<count; ++i)
			{
				gl3_alias_vtx_t* cur = &buf[i];
				int index_xyz;
				int j = 0;
				float l;

				/* texture coordinates come from the draw list */
				cur->texCoord[0] = ((float *) order)[0];
				cur->texCoord[1] = ((float *) order)[1];

				index_xyz = order[2];

				order += 3;

				/* normals and vertexes come from the frame list */
				// shadedots is set above according to rotation (around Z axis I think)
				// to one of 16 (SHADEDOT_QUANT) presets in r_avertexnormal_dots
				l = shadedots[verts[index_xyz].lightnormalindex];

				for(j=0; j<3; ++j)
				{
					cur->pos[j] = s_lerped[index_xyz][j];
					cur->color[j] = l * shadelight[j];
				}
				cur->color[3] = alpha;
			}
		}

		// translate triangle fan/strip to just triangle indices
		if(type == GL_TRIANGLE_FAN)
		{
			GLushort i;
			for(i=1; i < count-1; ++i)
			{
				GLushort* add = da_addn_uninit(idxBuf, 3);

				add[0] = nextVtxIdx;
				add[1] = nextVtxIdx+i;
				add[2] = nextVtxIdx+i+1;
			}
		}
		else // triangle strip
		{
			GLushort i;
			for(i=1; i < count-2; i+=2)
			{
				// add two triangles at once, because the vertex order is different
				// for odd vs even triangles
				GLushort* add = da_addn_uninit(idxBuf, 6);

				add[0] = nextVtxIdx + i-1;
				add[1] = nextVtxIdx + i;
				add[2] = nextVtxIdx + i+1;

				add[3] = nextVtxIdx + i;
				add[4] = nextVtxIdx + i+2;
				add[5] = nextVtxIdx + i+1;
			}
			// add remaining triangle, if any
			if(i < count-1)
			{
				GLushort* add = da_addn_uninit(idxBuf, 3);

				add[0] = nextVtxIdx + i-1;
				add[1] = nextVtxIdx + i;
				add[2] = nextVtxIdx + i+1;
			}
		}
	}

	GL3_BindVAO(gl3state.vaoAlias);
	GL3_BindVBO(gl3state.vboAlias);

	glBufferData(GL_ARRAY_BUFFER, da_count(vtxBuf)*sizeof(gl3_alias_vtx_t), vtxBuf.p, GL_STREAM_DRAW);
	GL3_BindEBO(gl3state.eboAlias);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, da_count(idxBuf)*sizeof(GLushort), idxBuf.p, GL_STREAM_DRAW);
	glDrawElements(GL_TRIANGLES, da_count(idxBuf), GL_UNSIGNED_SHORT, NULL);
}

/*
 * Interpolates between two frames and origins
 */
static void
DrawAliasFrameLerp(dmdx_t *paliashdr, entity_t* entity, vec3_t shadelight)
{
	daliasxframe_t *frame, *oldframe;
	dxtrivertx_t *ov, *verts;
	int *order;
	float alpha;
	vec3_t move, delta, vectors[3];
	vec3_t frontv, backv;
	int i;
	float backlerp = entity->backlerp;
	float frontlerp = 1.0 - backlerp;
	float *lerp;
	int num_mesh_nodes;
	dmdxmesh_t *mesh_nodes;
	vec4_t *s_lerped;
	fmnodeinfo_t *nodeinfo;

	nodeinfo = entity->fmnodeinfo;

	// draw without texture? used for quad damage effect etc, I think
	qboolean colorOnly = 0 != (entity->flags &
			(RF_SHELL_RED | RF_SHELL_GREEN | RF_SHELL_BLUE | RF_SHELL_DOUBLE |
			 RF_SHELL_HALF_DAM));

	// TODO: maybe we could somehow store the non-rotated normal and do the dot in shader?
	float* shadedots = r_avertexnormal_dots[((int)(entity->angles[1] *
				(SHADEDOT_QUANT / 360.0))) & (SHADEDOT_QUANT - 1)];

	frame = (daliasxframe_t *)((byte *)paliashdr + paliashdr->ofs_frames
							  + entity->frame * paliashdr->framesize);
	verts = frame->verts;

	oldframe = (daliasxframe_t *)((byte *)paliashdr + paliashdr->ofs_frames
				+ entity->oldframe * paliashdr->framesize);
	ov = oldframe->verts;

	order = (int *)((byte *)paliashdr + paliashdr->ofs_glcmds);

	if (entity->flags & RF_TRANSLUCENT)
	{
		alpha = entity->alpha * 0.666f;
	}
	else
	{
		alpha = 1.0;
	}

	if (colorOnly)
	{
		GL3_UseProgram(gl3state.si3DaliasColor.shaderProgram);
	}
	else
	{
		GL3_UseProgram(gl3state.si3Dalias.shaderProgram);
	}

	if(gl3_colorlight->value == 0.0f)
	{
		float avg = 0.333333f * (shadelight[0]+shadelight[1]+shadelight[2]);
		shadelight[0] = shadelight[1] = shadelight[2] = avg;
	}

	/* move should be the delta back to the previous frame * backlerp */
	VectorSubtract(entity->oldorigin, entity->origin, delta);
	AngleVectors(entity->angles, vectors[0], vectors[1], vectors[2]);

	move[0] = DotProduct(delta, vectors[0]); /* forward */
	move[1] = -DotProduct(delta, vectors[1]); /* left */
	move[2] = DotProduct(delta, vectors[2]); /* up */

	VectorAdd(move, oldframe->translate, move);

	for (i = 0; i < 3; i++)
	{
		move[i] = backlerp * move[i] + frontlerp * frame->translate[i];

		frontv[i] = frontlerp * frame->scale[i];
		backv[i] = backlerp * oldframe->scale[i];
	}

	/* buffer for scalled vert from frame */
	s_lerped = R_VertBufferRealloc(paliashdr->num_xyz);

	lerp = s_lerped[0];

	R_LerpVerts(colorOnly, paliashdr->num_xyz, verts, ov, lerp, move, frontv, backv);

	YQ2_STATIC_ASSERT(sizeof(gl3_alias_vtx_t) == 9*sizeof(GLfloat), "invalid gl3_alias_vtx_t size");

	num_mesh_nodes = paliashdr->num_meshes;
	mesh_nodes = (dmdxmesh_t *)((char*)paliashdr + paliashdr->ofs_meshes);

	for (i = 0; i < num_mesh_nodes; i++)
	{
		if (nodeinfo && nodeinfo[i].flags & FMNI_NO_DRAW)
		{
			continue;
		}

		DrawAliasFrameLerpCommands(paliashdr, entity, shadelight,
			order + mesh_nodes[i].ofs_glcmds,
			order + Q_min(paliashdr->num_glcmds,
				mesh_nodes[i].ofs_glcmds + mesh_nodes[i].num_glcmds),
			shadedots, alpha, colorOnly, verts, s_lerped);
	}
}

static void
DrawAliasShadowCommands(int *order, int *order_end, vec3_t shadevector,
	float height, float lheight, vec4_t *s_lerped)
{
	// GL1 uses alpha 0.5, but in GL3 0.3 looks better
	GLfloat color[4] = {0, 0, 0, 0.3};

	// draw the shadow in a single draw call, just like the model

	da_clear(vtxBuf);
	da_clear(idxBuf);

	while (1)
	{
		int i, j, count;
		GLenum type;
		GLushort nextVtxIdx = da_count(vtxBuf);

		/* get the vertex count and primitive type */
		count = *order++;

		if (!count || order >= order_end)
		{
			break; /* done */
		}

		if (count < 0)
		{
			count = -count;

			type = GL_TRIANGLE_FAN;
		}
		else
		{
			type = GL_TRIANGLE_STRIP;
		}

		gl3_alias_vtx_t* buf = da_addn_uninit(vtxBuf, count);

		for(i=0; i<count; ++i)
		{
			vec3_t point;

			/* normals and vertexes come from the frame list */
			VectorCopy(s_lerped[order[2]], point);

			point[0] -= shadevector[0] * (point[2] + lheight);
			point[1] -= shadevector[1] * (point[2] + lheight);
			point[2] = height;

			VectorCopy(point, buf[i].pos);

			for(j=0; j<4; ++j)  buf[i].color[j] = color[j];

			order += 3;
		}

		// translate triangle fan/strip to just triangle indices
		if(type == GL_TRIANGLE_FAN)
		{
			GLushort i;
			for(i=1; i < count-1; ++i)
			{
				GLushort* add = da_addn_uninit(idxBuf, 3);

				add[0] = nextVtxIdx;
				add[1] = nextVtxIdx+i;
				add[2] = nextVtxIdx+i+1;
			}
		}
		else // triangle strip
		{
			GLushort i;
			for(i=1; i < count-2; i+=2)
			{
				// add two triangles at once, because the vertex order is different
				// for odd vs even triangles
				GLushort* add = da_addn_uninit(idxBuf, 6);

				add[0] = nextVtxIdx + i-1;
				add[1] = nextVtxIdx + i;
				add[2] = nextVtxIdx + i+1;

				add[3] = nextVtxIdx + i;
				add[4] = nextVtxIdx + i+2;
				add[5] = nextVtxIdx + i+1;
			}
			// add remaining triangle, if any
			if(i < count-1)
			{
				GLushort* add = da_addn_uninit(idxBuf, 3);

				add[0] = nextVtxIdx + i-1;
				add[1] = nextVtxIdx + i;
				add[2] = nextVtxIdx + i+1;
			}
		}
	}

	GL3_BindVAO(gl3state.vaoAlias);
	GL3_BindVBO(gl3state.vboAlias);

	glBufferData(GL_ARRAY_BUFFER, da_count(vtxBuf)*sizeof(gl3_alias_vtx_t), vtxBuf.p, GL_STREAM_DRAW);
	GL3_BindEBO(gl3state.eboAlias);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, da_count(idxBuf)*sizeof(GLushort), idxBuf.p, GL_STREAM_DRAW);
	glDrawElements(GL_TRIANGLES, da_count(idxBuf), GL_UNSIGNED_SHORT, NULL);
}

static void
DrawAliasShadow(gl3_shadowinfo_t* shadowInfo)
{
	int *order, i;
	float height = 0, lheight;
	int num_mesh_nodes;
	dmdxmesh_t *mesh_nodes;
	vec4_t *s_lerped;

	dmdx_t* paliashdr = shadowInfo->paliashdr;
	entity_t* entity = shadowInfo->entity;

	vec3_t shadevector;
	VectorCopy(shadowInfo->shadevector, shadevector);

	/* buffer for scalled vert from frame */
	s_lerped = R_VertBufferRealloc(paliashdr->num_xyz);

	// all in this scope is to set s_lerped
	{
		daliasxframe_t *frame, *oldframe;
		dxtrivertx_t *ov, *verts;
		float backlerp = entity->backlerp;
		float frontlerp = 1.0f - backlerp;
		vec3_t move, delta, vectors[3];
		vec3_t frontv, backv;

		frame = (daliasxframe_t *)((byte *)paliashdr + paliashdr->ofs_frames
								  + entity->frame * paliashdr->framesize);
		verts = frame->verts;

		oldframe = (daliasxframe_t *)((byte *)paliashdr + paliashdr->ofs_frames
					+ entity->oldframe * paliashdr->framesize);
		ov = oldframe->verts;

		/* move should be the delta back to the previous frame * backlerp */
		VectorSubtract(entity->oldorigin, entity->origin, delta);
		AngleVectors(entity->angles, vectors[0], vectors[1], vectors[2]);

		move[0] = DotProduct(delta, vectors[0]); /* forward */
		move[1] = -DotProduct(delta, vectors[1]); /* left */
		move[2] = DotProduct(delta, vectors[2]); /* up */

		VectorAdd(move, oldframe->translate, move);

		for (i = 0; i < 3; i++)
		{
			move[i] = backlerp * move[i] + frontlerp * frame->translate[i];

			frontv[i] = frontlerp * frame->scale[i];
			backv[i] = backlerp * oldframe->scale[i];
		}

		// false: don't extrude vertices for powerup - this means the powerup shell
		//  is not seen in the shadow, only the underlying model..
		R_LerpVerts(false, paliashdr->num_xyz, verts, ov, s_lerped[0], move, frontv, backv);
	}

	lheight = entity->origin[2] - shadowInfo->lightspot[2];
	order = (int *)((byte *)paliashdr + paliashdr->ofs_glcmds);
	height = -lheight + 0.1f;

	num_mesh_nodes = paliashdr->num_meshes;
	mesh_nodes = (dmdxmesh_t *)((char*)paliashdr + paliashdr->ofs_meshes);

	for (i = 0; i < num_mesh_nodes; i++)
	{
		DrawAliasShadowCommands(
			order + mesh_nodes[i].ofs_glcmds,
			order + Q_min(paliashdr->num_glcmds,
				mesh_nodes[i].ofs_glcmds + mesh_nodes[i].num_glcmds),
			shadevector, height, lheight, s_lerped);
	}
}

static qboolean
CullAliasModel(vec3_t bbox[8], entity_t *e)
{
	dmdx_t *paliashdr;

	gl3model_t* model = e->model;

	paliashdr = (dmdx_t *)model->extradata;

	if ((e->frame >= paliashdr->num_frames) || (e->frame < 0))
	{
		R_Printf(PRINT_DEVELOPER, "%s %s: no such frame %d\n",
				__func__, model->name, e->frame);
		e->frame = 0;
	}

	if ((e->oldframe >= paliashdr->num_frames) || (e->oldframe < 0))
	{
		R_Printf(PRINT_DEVELOPER, "%s %s: no such oldframe %d\n",
				__func__, model->name, e->oldframe);
		e->oldframe = 0;
	}

	return R_CullAliasMeshModel(paliashdr, frustum, e->frame, e->oldframe,
		e->angles, e->origin, bbox);
}

void
GL3_DrawAliasModel(entity_t *entity)
{
	int i;
	dmdx_t *paliashdr;
	float an;
	vec3_t bbox[8];
	vec3_t shadelight;
	vec3_t shadevector;
	gl3image_t *skin = NULL;
	hmm_mat4 origProjViewMat = {0}; // use for left-handed rendering
	// used to restore ModelView matrix after changing it for this entities position/rotation
	hmm_mat4 origModelMat = {0};

	if (!(entity->flags & RF_WEAPONMODEL))
	{
		if (CullAliasModel(bbox, entity))
		{
			return;
		}
	}

	if (entity->flags & RF_WEAPONMODEL)
	{
		if (gl_lefthand->value == 2)
		{
			return;
		}
	}

	gl3model_t* model = entity->model;
	paliashdr = (dmdx_t *)model->extradata;

	/* get lighting information */
	if (entity->flags &
		(RF_SHELL_HALF_DAM | RF_SHELL_GREEN | RF_SHELL_RED |
		 RF_SHELL_BLUE | RF_SHELL_DOUBLE))
	{
		VectorClear(shadelight);

		if (entity->flags & RF_SHELL_HALF_DAM)
		{
			shadelight[0] = 0.56;
			shadelight[1] = 0.59;
			shadelight[2] = 0.45;
		}

		if (entity->flags & RF_SHELL_DOUBLE)
		{
			shadelight[0] = 0.9;
			shadelight[1] = 0.7;
		}

		if (entity->flags & RF_SHELL_RED)
		{
			shadelight[0] = 1.0;
		}

		if (entity->flags & RF_SHELL_GREEN)
		{
			shadelight[1] = 1.0;
		}

		if (entity->flags & RF_SHELL_BLUE)
		{
			shadelight[2] = 1.0;
		}
	}
	else if (entity->flags & RF_FULLBRIGHT)
	{
		for (i = 0; i < 3; i++)
		{
			shadelight[i] = 1.0;
		}
	}
	else
	{
		if (!gl3_worldmodel || !gl3_worldmodel->lightdata)
		{
			shadelight[0] = shadelight[1] = shadelight[2] = 1.0F;
		}
		else
		{
			R_LightPoint(gl3_worldmodel->grid, entity, &gl3_newrefdef,
				gl3_worldmodel->surfaces, gl3_worldmodel->nodes, entity->origin,
				shadelight, r_modulate->value, lightspot);
		}

		/* player lighting hack for communication back to server */
		if (entity->flags & RF_WEAPONMODEL)
		{
			/* pick the greatest component, which should be
			   the same as the mono value returned by software */
			if (shadelight[0] > shadelight[1])
			{
				if (shadelight[0] > shadelight[2])
				{
					r_lightlevel->value = 150 * shadelight[0];
				}
				else
				{
					r_lightlevel->value = 150 * shadelight[2];
				}
			}
			else
			{
				if (shadelight[1] > shadelight[2])
				{
					r_lightlevel->value = 150 * shadelight[1];
				}
				else
				{
					r_lightlevel->value = 150 * shadelight[2];
				}
			}
		}
	}

	if (entity->flags & RF_MINLIGHT)
	{
		for (i = 0; i < 3; i++)
		{
			if (shadelight[i] > 0.1)
			{
				break;
			}
		}

		if (i == 3)
		{
			shadelight[0] = 0.1;
			shadelight[1] = 0.1;
			shadelight[2] = 0.1;
		}
	}

	if (entity->flags & RF_GLOW)
	{
		/* bonus items will pulse with time */
		float scale;

		scale = 0.1 * sin(gl3_newrefdef.time * 7);

		for (i = 0; i < 3; i++)
		{
			float	min;

			min = shadelight[i] * 0.8;
			shadelight[i] += scale;

			if (shadelight[i] < min)
			{
				shadelight[i] = min;
			}
		}
	}

	// Note: gl_overbrightbits are now applied in shader.

	/* ir goggles color override */
	if ((gl3_newrefdef.rdflags & RDF_IRGOGGLES) && (entity->flags & RF_IR_VISIBLE))
	{
		shadelight[0] = 1.0;
		shadelight[1] = 0.0;
		shadelight[2] = 0.0;
	}

	an = entity->angles[1] / 180 * M_PI;
	shadevector[0] = cos(-an);
	shadevector[1] = sin(-an);
	shadevector[2] = 1;
	VectorNormalize(shadevector);

	/* locate the proper data */
	c_alias_polys += paliashdr->num_tris;

	/* draw all the triangles */
	if (entity->flags & RF_DEPTHHACK)
	{
		/* hack the depth range to prevent view model from poking into walls */
		glDepthRange(gl3depthmin, gl3depthmin + 0.3 * (gl3depthmax - gl3depthmin));
	}

	if (entity->flags & RF_WEAPONMODEL)
	{
		extern hmm_mat4 GL3_MYgluPerspective(GLdouble fovy, GLdouble aspect, GLdouble zNear, GLdouble zFar);

		origProjViewMat = gl3state.uni3DData.transProjViewMat4;

		// render weapon with a different FOV (r_gunfov) so it's not distorted at high view FOV
		float screenaspect = (float)gl3_newrefdef.width / gl3_newrefdef.height;
		float dist = (r_farsee->value == 0) ? 4096.0f : 8192.0f;

		hmm_mat4 projMat;
		if (r_gunfov->value < 0)
		{
			projMat = GL3_MYgluPerspective(gl3_newrefdef.fov_y, screenaspect, 4, dist);
		}
		else
		{
			projMat = GL3_MYgluPerspective(r_gunfov->value, screenaspect, 4, dist);
		}

		if(gl_lefthand->value == 1.0F)
		{
			// to mirror gun so it's rendered left-handed, just invert X-axis column
			// of projection matrix
			for(int i=0; i<4; ++i)
			{
				projMat.Elements[0][i] = - projMat.Elements[0][i];
			}
			//GL3_UpdateUBO3D(); Note: GL3_RotateForEntity() will call this,no need to do it twice before drawing

			glCullFace(GL_BACK);
		}
		gl3state.uni3DData.transProjViewMat4 = HMM_MultiplyMat4(projMat, gl3state.viewMat3D);
	}

	//glPushMatrix();
	origModelMat = gl3state.uni3DData.transModelMat4;

	entity->angles[PITCH] = -entity->angles[PITCH];
	GL3_RotateForEntity(entity);
	entity->angles[PITCH] = -entity->angles[PITCH];

	/* select skin */
	if (entity->skin)
	{
		skin = entity->skin; /* custom player skin */
	}
	else
	{
		if (entity->skinnum < model->numskins)
		{
			skin = model->skins[entity->skinnum];
		}

		if (!skin)
		{
			skin = model->skins[0];
		}
	}

	if (!skin)
	{
		skin = gl3_notexture; /* fallback... */
	}

	GL3_Bind(skin->texnum);

	if (entity->flags & RF_TRANSLUCENT)
	{
		glEnable(GL_BLEND);
	}


	if ((entity->frame >= paliashdr->num_frames) ||
		(entity->frame < 0))
	{
		R_Printf(PRINT_DEVELOPER, "R_DrawAliasModel %s: no such frame %d\n",
				model->name, entity->frame);
		entity->frame = 0;
		entity->oldframe = 0;
	}

	if ((entity->oldframe >= paliashdr->num_frames) ||
		(entity->oldframe < 0))
	{
		R_Printf(PRINT_DEVELOPER, "R_DrawAliasModel %s: no such oldframe %d\n",
				model->name, entity->oldframe);
		entity->frame = 0;
		entity->oldframe = 0;
	}

	DrawAliasFrameLerp(paliashdr, entity, shadelight);

	//glPopMatrix();
	gl3state.uni3DData.transModelMat4 = origModelMat;
	GL3_UpdateUBO3D();

	if (entity->flags & RF_WEAPONMODEL)
	{
		gl3state.uni3DData.transProjViewMat4 = origProjViewMat;
		GL3_UpdateUBO3D();
		if(gl_lefthand->value == 1.0F)
			glCullFace(GL_FRONT);
	}

	if (entity->flags & RF_TRANSLUCENT)
	{
		glDisable(GL_BLEND);
	}

	if (entity->flags & RF_DEPTHHACK)
	{
		glDepthRange(gl3depthmin, gl3depthmax);
	}

	if (gl_shadows->value && gl3config.stencil && !(entity->flags & (RF_TRANSLUCENT | RF_WEAPONMODEL | RF_NOSHADOW)))
	{
		gl3_shadowinfo_t si = {0};
		VectorCopy(lightspot, si.lightspot);
		VectorCopy(shadevector, si.shadevector);
		si.paliashdr = paliashdr;
		si.entity = entity;

		da_push(shadowModels, si);
	}
}

void
GL3_ResetShadowAliasModels(void)
{
	da_clear(shadowModels);
}

void
GL3_DrawAliasShadows(void)
{
	size_t numShadowModels = da_count(shadowModels);
	if(numShadowModels == 0)
	{
		return;
	}

	//glPushMatrix();
	hmm_mat4 oldMat = gl3state.uni3DData.transModelMat4;

	glEnable(GL_BLEND);
	GL3_UseProgram(gl3state.si3DaliasColor.shaderProgram);

	if (gl3config.stencil)
	{
		glEnable(GL_STENCIL_TEST);
		glStencilFunc(GL_EQUAL, 1, 2);
		glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);
	}

	for(size_t i=0; i<numShadowModels; ++i)
	{
		gl3_shadowinfo_t* si = &shadowModels.p[i]; // XXX da_getptr(shadowModels, i);
		entity_t* e = si->entity;

		/* don't rotate shadows on ungodly axes */
		//glTranslatef(e->origin[0], e->origin[1], e->origin[2]);
		//glRotatef(e->angles[1], 0, 0, 1);
		hmm_mat4 rotTransMat = HMM_Rotate(e->angles[1], HMM_Vec3(0, 0, 1));
		VectorCopy(e->origin, rotTransMat.Elements[3]);
		gl3state.uni3DData.transModelMat4 = HMM_MultiplyMat4(oldMat, rotTransMat);
		GL3_UpdateUBO3D();

		DrawAliasShadow(si);
	}

	if (gl3config.stencil)
	{
		glDisable(GL_STENCIL_TEST);
	}

	glDisable(GL_BLEND);

	//glPopMatrix();
	gl3state.uni3DData.transModelMat4 = oldMat;
	GL3_UpdateUBO3D();
}
