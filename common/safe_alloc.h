#ifndef EQEMU_COMMON_SAFE_ALLOC_H
#define EQEMU_COMMON_SAFE_ALLOC_H

#define SafeVarAllocParse(type, var_name) if(idx + sizeof(type) > buffer.size()) { return false; } \
	type var_name = *(type*)&buffer[idx]; \
	idx += sizeof(type);

#define SafeStructAllocParse(type, var_name) if(idx + sizeof(type) > buffer.size()) { return false; } \
	type *var_name = (type*)&buffer[idx]; \
	idx += sizeof(type);

#define SafeBufferAllocParse(var_name, length) if(idx + length > buffer.size()) { return false; } \
	var_name = (char*)&buffer[idx]; \
	idx += length;

#endif
