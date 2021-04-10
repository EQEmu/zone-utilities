#ifndef EQEMU_COMMON_SAFE_ALLOC_H
#define EQEMU_COMMON_SAFE_ALLOC_H

#define SafeVarAllocParse(type, var_name) if(idx + sizeof(type) > (uint32_t)buffer.size()) { return false; } \
	type var_name = *(type*)&buffer[idx]; \
	idx += sizeof(type);

#define SafeStructAllocParse(type, var_name) if(idx + sizeof(type) > (uint32_t)buffer.size()) { return false; } \
	type *var_name = (type*)&buffer[idx]; \
	idx += sizeof(type);

#define SafeBufferAllocParse(var_name, length) if(idx + length > (uint32_t)buffer.size()) { return false; } \
	var_name = (char*)&buffer[idx]; \
	idx += length;

#define SafeStringAllocParse(var_name) std::string var_name = &buffer[idx]; \
	if ((idx + (uint32_t)var_name.length() + 1) > (uint32_t)buffer.size()) { return false; } \
	idx += (uint32_t)var_name.length() + 1;

#define SafeStringNoAllocParse(var_name) var_name = &buffer[idx]; \
	if ((idx + (uint32_t)var_name.length() + 1) > (uint32_t)buffer.size()) { return false; } \
	idx += (uint32_t)var_name.length() + 1;

#endif
