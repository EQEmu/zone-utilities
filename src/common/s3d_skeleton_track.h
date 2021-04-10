#ifndef EQEMU_COMMON_S3D_SKELETON_TRACK_H
#define EQEMU_COMMON_S3D_SKELETON_TRACK_H

#include "wld_fragment.h"
#include <vector>

namespace EQEmu
{

namespace S3D
{

class SkeletonTrack
{
public:
	struct BoneOrientation
	{
		int16_t rotate_denom;
		int16_t rotate_x_num;
		int16_t rotate_y_num;
		int16_t rotate_z_num;
		int16_t shift_x_num;
		int16_t shift_y_num;
		int16_t shift_z_num;
		int16_t shift_denom;
		
	};

	struct Bone
	{
		std::shared_ptr<BoneOrientation> orientation;
		std::shared_ptr<Geometry> model;
		std::vector<std::shared_ptr<Bone>> children;
	};

	SkeletonTrack() { }
	~SkeletonTrack() { }

	void SetName(std::string nname) { name = nname; }
	
	std::string &GetName() { return name; }

	std::vector<std::shared_ptr<Bone>> &GetBones() { return bones; }
private:
	std::string name;
	std::vector<std::shared_ptr<Bone>> bones;
};

}

}

#endif
