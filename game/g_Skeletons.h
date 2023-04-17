#ifndef G_SKELETONS_H
#define G_SKELETONS_H

#include "../qcommon/qcommon.h"
#include "Skeletons.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct G_SkeletalJoint_s
{
	int children;		// must be the first field
	float destAngles[3];
	float angVels[3];
	float angles[3];
	int changed[3];
	qboolean inUse;
} G_SkeletalJoint_t;

extern G_SkeletalJoint_t skeletalJoints[MAX_ARRAYED_SKELETAL_JOINTS];
extern struct ArrayedListNode_s jointNodes[MAX_ARRAYED_JOINT_NODES];

int CreateSkeleton(int structure);
void FreeSkeleton(int root);
float GetJointAngle(int jointIndex, int angleIndex);
qboolean SetJointAngVel(int jointIndex, int angleIndex, float destAngle, float angSpeed);

#ifdef __cplusplus
} //end extern "C"
#endif

#endif