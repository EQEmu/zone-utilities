#ifndef EQEMU_COMMON_WLD_FRAGMENT_H
#define EQEMU_COMMON_WLD_FRAGMENT_H

#include <stdint.h>
#include <any>

#include "s3d_texture_brush_set.h"
#include "placeable.h"
#include "s3d_geometry.h"
#include "s3d_bsp.h"
#include "light.h"
#include "wld_fragment_reference.h"
#include "s3d_skeleton_track.h"

namespace EQEmu
{

namespace S3D
{

class S3DLoader;
class WLDFragment
{
public:
	WLDFragment() { }
	~WLDFragment() { }

	int type;
	int name;
	std::any data;
};

class WLDFragment03 : public WLDFragment
{
public:
	WLDFragment03(std::vector<WLDFragment> &out, char *frag_buffer, uint32_t frag_length, uint32_t frag_name, char *hash, bool old);
	~WLDFragment03() { }

	std::shared_ptr<Texture> GetData() { try { return std::any_cast<std::shared_ptr<Texture>>(data); } catch (std::bad_any_cast&) { return std::shared_ptr<Texture>(); } }
};

class WLDFragment04 : public WLDFragment
{
public:
	WLDFragment04(std::vector<WLDFragment> &out, char *frag_buffer, uint32_t frag_length, uint32_t frag_name, char *hash, bool old);
	~WLDFragment04() { }

	std::shared_ptr<TextureBrush> GetData() { try { return std::any_cast<std::shared_ptr<TextureBrush>>(data); } catch (std::bad_any_cast&) { return std::shared_ptr<TextureBrush>(); } }
};

class WLDFragment05 : public WLDFragment
{
public:
	WLDFragment05(std::vector<WLDFragment> &out, char *frag_buffer, uint32_t frag_length, uint32_t frag_name, char *hash, bool old);
	~WLDFragment05() { }

	uint32_t GetData() { try { return std::any_cast<uint32_t>(data); } catch (std::bad_any_cast&) { return 0; } }
};

class WLDFragment10 : public WLDFragment
{
public:
	WLDFragment10(std::vector<WLDFragment> &out, char *frag_buffer, uint32_t frag_length, uint32_t frag_name, char *hash, bool old);
	~WLDFragment10() { }

	std::shared_ptr<SkeletonTrack> GetData() { try { return std::any_cast<std::shared_ptr<SkeletonTrack>>(data); } catch (std::bad_any_cast&) { return std::shared_ptr<SkeletonTrack>(); } }
};

class WLDFragment11 : public WLDFragment
{
public:
	WLDFragment11(std::vector<WLDFragment> &out, char *frag_buffer, uint32_t frag_length, uint32_t frag_name, char *hash, bool old);
	~WLDFragment11() { }

	uint32_t GetData() { try { return std::any_cast<uint32_t>(data); } catch (std::bad_any_cast&) { return 0; } }
};

class WLDFragment12 : public WLDFragment
{
public:
	WLDFragment12(std::vector<WLDFragment> &out, char *frag_buffer, uint32_t frag_length, uint32_t frag_name, char *hash, bool old);
	~WLDFragment12() { }

	std::shared_ptr<SkeletonTrack::BoneOrientation> GetData() { try { return std::any_cast<std::shared_ptr<SkeletonTrack::BoneOrientation>>(data); } catch (std::bad_any_cast&) { return std::shared_ptr<SkeletonTrack::BoneOrientation>(); } }
};

class WLDFragment13 : public WLDFragment
{
public:
	WLDFragment13(std::vector<WLDFragment> &out, char *frag_buffer, uint32_t frag_length, uint32_t frag_name, char *hash, bool old);
	~WLDFragment13() { }

	uint32_t GetData() { try { return std::any_cast<uint32_t>(data); } catch (std::bad_any_cast&) { return 0; } }
};

class WLDFragment14 : public WLDFragment
{
public:
	WLDFragment14(std::vector<WLDFragment> &out, char *frag_buffer, uint32_t frag_length, uint32_t frag_name, char *hash, bool old);
	~WLDFragment14() { }

	std::shared_ptr<WLDFragmentReference> GetData() { try { return std::any_cast<std::shared_ptr<WLDFragmentReference>>(data); } catch (std::bad_any_cast&) { return std::shared_ptr<WLDFragmentReference>(); } }
};

class WLDFragment15 : public WLDFragment
{
public:
	WLDFragment15(std::vector<WLDFragment> &out, char *frag_buffer, uint32_t frag_length, uint32_t frag_name, char *hash, bool old);
	~WLDFragment15() { }

	std::shared_ptr<Placeable> GetData() { try { return std::any_cast<std::shared_ptr<Placeable>>(data); } catch (std::bad_any_cast&) { return std::shared_ptr<Placeable>(); } }
};

class WLDFragment1B : public WLDFragment
{
public:
	WLDFragment1B(std::vector<WLDFragment> &out, char *frag_buffer, uint32_t frag_length, uint32_t frag_name, char *hash, bool old);
	~WLDFragment1B() { }

	std::shared_ptr<Light> GetData() { try { return std::any_cast<std::shared_ptr<Light>>(data); } catch (std::bad_any_cast&) { return std::shared_ptr<Light>(); } }
};

class WLDFragment1C : public WLDFragment
{
public:
	WLDFragment1C(std::vector<WLDFragment> &out, char *frag_buffer, uint32_t frag_length, uint32_t frag_name, char *hash, bool old);
	~WLDFragment1C() { }

	uint32_t GetData() { try { return std::any_cast<uint32_t>(data); } catch (std::bad_any_cast&) { return 0; } }
};

class WLDFragment21 : public WLDFragment
{
public:
	WLDFragment21(std::vector<WLDFragment> &out, char *frag_buffer, uint32_t frag_length, uint32_t frag_name, char *hash, bool old);
	~WLDFragment21() { }

	std::shared_ptr<S3D::BSPTree> GetData() { try { return std::any_cast<std::shared_ptr<S3D::BSPTree>>(data); } catch (std::bad_any_cast&) { return std::shared_ptr<S3D::BSPTree>(); } }
};

class WLDFragment22 : public WLDFragment
{
public:
	WLDFragment22(std::vector<WLDFragment> &out, char *frag_buffer, uint32_t frag_length, uint32_t frag_name, char *hash, bool old);
	~WLDFragment22() { }
};

class WLDFragment28 : public WLDFragment
{
public:
	WLDFragment28(std::vector<WLDFragment> &out, char *frag_buffer, uint32_t frag_length, uint32_t frag_name, char *hash, bool old);
	~WLDFragment28() { }
};

class WLDFragment29 : public WLDFragment
{
public:
	WLDFragment29(std::vector<WLDFragment> &out, char *frag_buffer, uint32_t frag_length, uint32_t frag_name, char *hash, bool old);
	~WLDFragment29() { }

	std::shared_ptr<S3D::BSPRegion> GetData() { try { return std::any_cast<std::shared_ptr<S3D::BSPRegion>>(data); } catch (std::bad_any_cast&) { return std::shared_ptr<S3D::BSPRegion>(); } }
};

class WLDFragment2D : public WLDFragment
{
public:
	WLDFragment2D(std::vector<WLDFragment> &out, char *frag_buffer, uint32_t frag_length, uint32_t frag_name, char *hash, bool old);
	~WLDFragment2D() { }

	uint32_t GetData() { try { return std::any_cast<uint32_t>(data); } catch (std::bad_any_cast&) { return 0; } }
};

class WLDFragment30 : public WLDFragment
{
public:
	WLDFragment30(std::vector<WLDFragment> &out, char *frag_buffer, uint32_t frag_length, uint32_t frag_name, char *hash, bool old);
	~WLDFragment30() { }

	std::shared_ptr<TextureBrush> GetData() { try { return std::any_cast<std::shared_ptr<TextureBrush>>(data); } catch (std::bad_any_cast&) { return std::shared_ptr<TextureBrush>(); } }
};

class WLDFragment31 : public WLDFragment
{
public:
	WLDFragment31(std::vector<WLDFragment> &out, char *frag_buffer, uint32_t frag_length, uint32_t frag_name, char *hash, bool old);
	~WLDFragment31() { }

	std::shared_ptr<TextureBrushSet> GetData() { try { return std::any_cast<std::shared_ptr<TextureBrushSet>>(data); } catch (std::bad_any_cast&) { return std::shared_ptr<TextureBrushSet>(); } }
};

class WLDFragment36 : public WLDFragment
{
public:
	WLDFragment36(std::vector<WLDFragment> &out, char *frag_buffer, uint32_t frag_length, uint32_t frag_name, char *hash, bool old);
	~WLDFragment36() { }

	std::shared_ptr<Geometry> GetData() { try { return std::any_cast<std::shared_ptr<Geometry>>(data); } catch (std::bad_any_cast&) { return std::shared_ptr<Geometry>(); } }
};

}

}

#endif
