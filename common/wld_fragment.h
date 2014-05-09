#ifndef EQEMU_COMMON_WLD_FRAGMENT_H
#define EQEMU_COMMON_WLD_FRAGMENT_H

#include <stdint.h>
#include "texture_brush_set.h"
#include "placeable.h"
#include "geometry.h"
#include "bsp.h"
#include "light.h"
#include "any.h"

namespace EQEmu
{

class S3DLoader;
class WLDFragment
{
public:
	WLDFragment() { }
	~WLDFragment() { }

	int type;
	int name;
	EQEmu::Any data;
};

class WLDFragment03 : public WLDFragment
{
public:
	WLDFragment03(S3DLoader *loader, std::vector<WLDFragment> &out, char *frag_buffer, uint32_t frag_length, uint32_t frag_name, char *hash, bool old);
	~WLDFragment03() { }

	std::shared_ptr<Texture> GetData() { try { return EQEmu::any_cast<std::shared_ptr<Texture>>(data); } catch (EQEmu::bad_any_cast&) { return std::shared_ptr<Texture>(); } }
};

class WLDFragment04 : public WLDFragment
{
public:
	WLDFragment04(S3DLoader *loader, std::vector<WLDFragment> &out, char *frag_buffer, uint32_t frag_length, uint32_t frag_name, char *hash, bool old);
	~WLDFragment04() { }

	std::shared_ptr<TextureBrush> GetData() { try { return EQEmu::any_cast<std::shared_ptr<TextureBrush>>(data); } catch (EQEmu::bad_any_cast&) { return std::shared_ptr<TextureBrush>(); } }
};

class WLDFragment05 : public WLDFragment
{
public:
	WLDFragment05(S3DLoader *loader, std::vector<WLDFragment> &out, char *frag_buffer, uint32_t frag_length, uint32_t frag_name, char *hash, bool old);
	~WLDFragment05() { }

	uint32_t GetData() { try { return EQEmu::any_cast<uint32_t>(data); } catch (EQEmu::bad_any_cast&) { return 0; } }
};

class WLDFragment15 : public WLDFragment
{
public:
	WLDFragment15(S3DLoader *loader, std::vector<WLDFragment> &out, char *frag_buffer, uint32_t frag_length, uint32_t frag_name, char *hash, bool old);
	~WLDFragment15() { }

	std::shared_ptr<Placeable> GetData() { try { return EQEmu::any_cast<std::shared_ptr<Placeable>>(data); } catch (EQEmu::bad_any_cast&) { return std::shared_ptr<Placeable>(); } }
};

class WLDFragment1B : public WLDFragment
{
public:
	WLDFragment1B(S3DLoader *loader, std::vector<WLDFragment> &out, char *frag_buffer, uint32_t frag_length, uint32_t frag_name, char *hash, bool old);
	~WLDFragment1B() { }

	std::shared_ptr<Light> GetData() { try { return EQEmu::any_cast<std::shared_ptr<Light>>(data); } catch (EQEmu::bad_any_cast&) { return std::shared_ptr<Light>(); } }
};

class WLDFragment1C : public WLDFragment
{
public:
	WLDFragment1C(S3DLoader *loader, std::vector<WLDFragment> &out, char *frag_buffer, uint32_t frag_length, uint32_t frag_name, char *hash, bool old);
	~WLDFragment1C() { }

	uint32_t GetData() { try { return EQEmu::any_cast<uint32_t>(data); } catch (EQEmu::bad_any_cast&) { return 0; } }
};

class WLDFragment21 : public WLDFragment
{
public:
	WLDFragment21(S3DLoader *loader, std::vector<WLDFragment> &out, char *frag_buffer, uint32_t frag_length, uint32_t frag_name, char *hash, bool old);
	~WLDFragment21() { }

	std::shared_ptr<BSPTree> GetData() { try { return EQEmu::any_cast<std::shared_ptr<BSPTree>>(data); } catch (EQEmu::bad_any_cast&) { return std::shared_ptr<BSPTree>(); } }
};

class WLDFragment22 : public WLDFragment
{
public:
	WLDFragment22(S3DLoader *loader, std::vector<WLDFragment> &out, char *frag_buffer, uint32_t frag_length, uint32_t frag_name, char *hash, bool old);
	~WLDFragment22() { }
};

class WLDFragment28 : public WLDFragment
{
public:
	WLDFragment28(S3DLoader *loader, std::vector<WLDFragment> &out, char *frag_buffer, uint32_t frag_length, uint32_t frag_name, char *hash, bool old);
	~WLDFragment28() { }
};

class WLDFragment29 : public WLDFragment
{
public:
	WLDFragment29(S3DLoader *loader, std::vector<WLDFragment> &out, char *frag_buffer, uint32_t frag_length, uint32_t frag_name, char *hash, bool old);
	~WLDFragment29() { }

	std::shared_ptr<BSPRegion> GetData() { try { return EQEmu::any_cast<std::shared_ptr<BSPRegion>>(data); } catch (EQEmu::bad_any_cast&) { return std::shared_ptr<BSPRegion>(); } }
};

class WLDFragment30 : public WLDFragment
{
public:
	WLDFragment30(S3DLoader *loader, std::vector<WLDFragment> &out, char *frag_buffer, uint32_t frag_length, uint32_t frag_name, char *hash, bool old);
	~WLDFragment30() { }

	std::shared_ptr<TextureBrush> GetData() { try { return EQEmu::any_cast<std::shared_ptr<TextureBrush>>(data); } catch (EQEmu::bad_any_cast&) { return std::shared_ptr<TextureBrush>(); } }
};

class WLDFragment31 : public WLDFragment
{
public:
	WLDFragment31(S3DLoader *loader, std::vector<WLDFragment> &out, char *frag_buffer, uint32_t frag_length, uint32_t frag_name, char *hash, bool old);
	~WLDFragment31() { }

	std::shared_ptr<TextureBrushSet> GetData() { try { return EQEmu::any_cast<std::shared_ptr<TextureBrushSet>>(data); } catch (EQEmu::bad_any_cast&) { return std::shared_ptr<TextureBrushSet>(); } }
};

class WLDFragment36 : public WLDFragment
{
public:
	WLDFragment36(S3DLoader *loader, std::vector<WLDFragment> &out, char *frag_buffer, uint32_t frag_length, uint32_t frag_name, char *hash, bool old);
	~WLDFragment36() { }

	std::shared_ptr<Geometry> GetData() { try { return EQEmu::any_cast<std::shared_ptr<Geometry>>(data); } catch (EQEmu::bad_any_cast&) { return std::shared_ptr<Geometry>(); } }
};

}

#endif
