#include "s3d_loader.h"
#include "pfs.h"
#include "wld_structs.h"
#include "safe_alloc.h"

void decode_string_hash(char *str, size_t len) {
	uint8_t encarr[] = { 0x95, 0x3A, 0xC5, 0x2A, 0x95, 0x7A, 0x95, 0x6A };
	for (size_t i = 0; i < len; ++i) {
		str[i] ^= encarr[i % 8];
	}
}

EQEmu::S3DLoader::S3DLoader() {
}

EQEmu::S3DLoader::~S3DLoader() {
}

bool EQEmu::S3DLoader::ParseWLDFile(std::string file_name, std::string wld_name, std::vector<WLDFragment> &out) {
	out.clear();
	std::vector<char> buffer;
	char *current_hash;
	bool old = false;

	EQEmu::PFS::Archive archive;
	if (!archive.Open(file_name)) {
		return false;
	}

	if (!archive.Get(wld_name, buffer)) {
		return false;
	}

	size_t idx = 0;
	SafeStructAllocParse(wld_header, header);
	
	if (header->magic != 0x54503d02)
		return false;

	if (header->version == 0x00015500)
		old = true;

	SafeBufferAllocParse(current_hash, header->hash_length);
	decode_string_hash(current_hash, header->hash_length);

	out.clear();
	out.reserve(header->fragments);

	for (uint32_t i = 0; i < header->fragments; ++i) {
		SafeStructAllocParse(wld_fragment_header, frag_header);

		switch (frag_header->id) {
			case 0x03: {
				WLDFragment03 f(this, out, &buffer[idx], frag_header->size, frag_header->name_ref, current_hash, old);
				f.type = frag_header->id;
				f.name = frag_header->name_ref;
				out.push_back(f);
				break;
			}
			case 0x04: {
				WLDFragment04 f(this, out, &buffer[idx], frag_header->size, frag_header->name_ref, current_hash, old);
				f.type = frag_header->id;
				f.name = frag_header->name_ref;
				out.push_back(f);
				  break;
			}
			case 0x05: {
				WLDFragment05 f(this, out, &buffer[idx], frag_header->size, frag_header->name_ref, current_hash, old);
				f.type = frag_header->id;
				f.name = frag_header->name_ref;
				out.push_back(f);
				break;
			}
			case 0x10: {
				WLDFragment10 f(this, out, &buffer[idx], frag_header->size, frag_header->name_ref, current_hash, old);
				f.type = frag_header->id;
				f.name = frag_header->name_ref;
				out.push_back(f);
				break;
			}
			case 0x11: {
				WLDFragment11 f(this, out, &buffer[idx], frag_header->size, frag_header->name_ref, current_hash, old);
				f.type = frag_header->id;
				f.name = frag_header->name_ref;
				out.push_back(f);
				break;
			}
			case 0x12: {
				WLDFragment12 f(this, out, &buffer[idx], frag_header->size, frag_header->name_ref, current_hash, old);
				f.type = frag_header->id;
				f.name = frag_header->name_ref;
				out.push_back(f);
				break;
			}
			case 0x13: {
				WLDFragment13 f(this, out, &buffer[idx], frag_header->size, frag_header->name_ref, current_hash, old);
				f.type = frag_header->id;
				f.name = frag_header->name_ref;
				out.push_back(f);
				break;
			}
			case 0x14: {
				 WLDFragment14 f(this, out, &buffer[idx], frag_header->size, frag_header->name_ref, current_hash, old);
				 f.type = frag_header->id;
				 f.name = frag_header->name_ref;
				 out.push_back(f);
				 break;
			}
			case 0x15: {
				WLDFragment15 f(this, out, &buffer[idx], frag_header->size, frag_header->name_ref, current_hash, old);
				f.type = frag_header->id;
				f.name = frag_header->name_ref;
				out.push_back(f);
				break;
			}
			case 0x1B: {
				WLDFragment1B f(this, out, &buffer[idx], frag_header->size, frag_header->name_ref, current_hash, old);
				f.type = frag_header->id;
				f.name = frag_header->name_ref;
				out.push_back(f);
				break;
			}
			case 0x1C: {
				WLDFragment1C f(this, out, &buffer[idx], frag_header->size, frag_header->name_ref, current_hash, old);
				f.type = frag_header->id;
				f.name = frag_header->name_ref;
				out.push_back(f);
				break;
			}
			case 0x21: {
				WLDFragment21 f(this, out, &buffer[idx], frag_header->size, frag_header->name_ref, current_hash, old);
				f.type = frag_header->id;
				f.name = frag_header->name_ref;
				out.push_back(f);
				break;
			}
			case 0x22: {
				WLDFragment22 f(this, out, &buffer[idx], frag_header->size, frag_header->name_ref, current_hash, old);
				f.type = frag_header->id;
				f.name = frag_header->name_ref;
				out.push_back(f);
				break;
			}
			case 0x28: {
				WLDFragment28 f(this, out, &buffer[idx], frag_header->size, frag_header->name_ref, current_hash, old);
				f.type = frag_header->id;
				f.name = frag_header->name_ref;
				out.push_back(f);
				break;
			}
			case 0x29: {
				WLDFragment29 f(this, out, &buffer[idx], frag_header->size, frag_header->name_ref, current_hash, old);
				f.type = frag_header->id;
				f.name = frag_header->name_ref;
				out.push_back(f);
				break;
			}
			case 0x2D: {
				WLDFragment2D f(this, out, &buffer[idx], frag_header->size, frag_header->name_ref, current_hash, old);
				f.type = frag_header->id;
				f.name = frag_header->name_ref;
				out.push_back(f);
				break;
			}
			case 0x30: {
				WLDFragment30 f(this, out, &buffer[idx], frag_header->size, frag_header->name_ref, current_hash, old);
				f.type = frag_header->id;
				f.name = frag_header->name_ref;
				out.push_back(f);
				break;
			}
			case 0x31: {
				 WLDFragment31 f(this, out, &buffer[idx], frag_header->size, frag_header->name_ref, current_hash, old);
				 f.type = frag_header->id;
				 f.name = frag_header->name_ref;
				 out.push_back(f);
				 break;
			}
			case 0x36: {
				WLDFragment36 f(this, out, &buffer[idx], frag_header->size, frag_header->name_ref, current_hash, old);
				f.type = frag_header->id;
				f.name = frag_header->name_ref;

				out.push_back(f);
				break;
			}
			default:
				WLDFragment f;
				f.type = frag_header->id;
				f.name = frag_header->name_ref;
				out.push_back(f);
				break;
		}

		idx += frag_header->size - 4;
	}

	return true;
}
