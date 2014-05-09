#ifndef EQEMU_COMMON_BSP_H
#define EQEMU_COMMON_BSP_H

namespace EQEmu
{

class BSPRegion
{
public:
	BSPRegion() { }
	~BSPRegion() { }

	void SetFlags(uint32_t nflags) { flags = nflags; }
	void AddRegion(uint32_t r) { regions.push_back(r); }
	void SetName(std::string nname) { name = nname; }
	void SetExtendedInfo(std::string next) { extended_info = next; }

	uint32_t GetFlags() { return flags; }
	std::vector<uint32_t> &GetRegions() { return regions; }
	std::string &GetName() { return name; }
	std::string &GetExtendedInfo() { return extended_info; }
private:
	uint32_t flags;
	std::vector<uint32_t> regions;
	std::string name;
	std::string extended_info;
};

class BSPTree
{
public:
	struct BSPNode
	{
		uint32_t number;
		float normal[3];
		float split_dist;
		uint32_t region;
		int32_t special;
		uint32_t left;
		uint32_t right;
	};

	BSPTree() { }
	~BSPTree() { }

	std::vector<BSPNode> &GetNodes() { return nodes; }
private:
	std::vector<BSPNode> nodes;
};

}

#endif
