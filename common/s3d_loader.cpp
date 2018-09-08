#include "s3d_loader.h"
#include "pfs.h"
#include "wld_structs.h"
#include "safe_alloc.h"
#include "dependency/container.h"

void decode_string_hash(char *str, size_t len) {
	uint8_t encarr[] = { 0x95, 0x3A, 0xC5, 0x2A, 0x95, 0x7A, 0x95, 0x6A };
	for (size_t i = 0; i < len; ++i) {
		str[i] ^= encarr[i % 8];
	}
}

EQEmu::S3DLoader::S3DLoader() {
	_logger = Container::Get().Resolve<ILogger>();
}

EQEmu::S3DLoader::~S3DLoader() {
}

bool EQEmu::S3DLoader::ParseWLDFile(std::string file_name, std::string wld_name, std::vector<S3D::WLDFragment> &out) {
	out.clear();
	std::vector<char> buffer;
	char *current_hash;
	bool old = false;

	EQEmu::PFS::Archive archive;
	if (!archive.Open(file_name)) {
		_logger->LogDebug("Unable to open file {0}.", file_name);
		return false;
	}

	if (!archive.Get(wld_name, buffer)) {
		_logger->LogDebug("Unable to open wld file %s.", wld_name);
		return false;
	}

	size_t idx = 0;
	SafeStructAllocParse(wld_header, header);
	
	if (header->magic != 0x54503d02) {
		_logger->LogDebug("Header magic of {0:x} did not match expected 0x54503d02", header->magic);
		return false;
	}

	if (header->version == 0x00015500) {
		old = true;
	}

	SafeBufferAllocParse(current_hash, header->hash_length);
	decode_string_hash(current_hash, header->hash_length);

	out.clear();
	out.reserve(header->fragments);

	_logger->LogTrace("Parsing WLD fragments.");
	for (uint32_t i = 0; i < header->fragments; ++i) {
		SafeStructAllocParse(wld_fragment_header, frag_header);

		_logger->LogTrace("Dispatching WLD fragment of type {0:x}", frag_header->id);
		switch (frag_header->id) {
			case 0x03: {
				S3D::WLDFragment03 f(out, &buffer[idx], frag_header->size, frag_header->name_ref, current_hash, old);
				f.type = frag_header->id;
				f.name = frag_header->name_ref;
				out.push_back(f);
				break;
			}
			case 0x04: {
				S3D::WLDFragment04 f(out, &buffer[idx], frag_header->size, frag_header->name_ref, current_hash, old);
				f.type = frag_header->id;
				f.name = frag_header->name_ref;
				out.push_back(f);
				  break;
			}
			case 0x05: {
				S3D::WLDFragment05 f(out, &buffer[idx], frag_header->size, frag_header->name_ref, current_hash, old);
				f.type = frag_header->id;
				f.name = frag_header->name_ref;
				out.push_back(f);
				break;
			}
			case 0x10: {
				S3D::WLDFragment10 f(out, &buffer[idx], frag_header->size, frag_header->name_ref, current_hash, old);
				f.type = frag_header->id;
				f.name = frag_header->name_ref;
				out.push_back(f);
				break;
			}
			case 0x11: {
				S3D::WLDFragment11 f(out, &buffer[idx], frag_header->size, frag_header->name_ref, current_hash, old);
				f.type = frag_header->id;
				f.name = frag_header->name_ref;
				out.push_back(f);
				break;
			}
			case 0x12: {
				S3D::WLDFragment12 f(out, &buffer[idx], frag_header->size, frag_header->name_ref, current_hash, old);
				f.type = frag_header->id;
				f.name = frag_header->name_ref;
				out.push_back(f);
				break;
			}
			case 0x13: {
				S3D::WLDFragment13 f(out, &buffer[idx], frag_header->size, frag_header->name_ref, current_hash, old);
				f.type = frag_header->id;
				f.name = frag_header->name_ref;
				out.push_back(f);
				break;
			}
			case 0x14: {
				S3D::WLDFragment14 f(out, &buffer[idx], frag_header->size, frag_header->name_ref, current_hash, old);
				 f.type = frag_header->id;
				 f.name = frag_header->name_ref;
				 out.push_back(f);
				 break;
			}
			case 0x15: {
				S3D::WLDFragment15 f(out, &buffer[idx], frag_header->size, frag_header->name_ref, current_hash, old);
				f.type = frag_header->id;
				f.name = frag_header->name_ref;
				out.push_back(f);
				break;
			}
			case 0x1B: {
				S3D::WLDFragment1B f(out, &buffer[idx], frag_header->size, frag_header->name_ref, current_hash, old);
				f.type = frag_header->id;
				f.name = frag_header->name_ref;
				out.push_back(f);
				break;
			}
			case 0x1C: {
				S3D::WLDFragment1C f(out, &buffer[idx], frag_header->size, frag_header->name_ref, current_hash, old);
				f.type = frag_header->id;
				f.name = frag_header->name_ref;
				out.push_back(f);
				break;
			}
			case 0x21: {
				S3D::WLDFragment21 f(out, &buffer[idx], frag_header->size, frag_header->name_ref, current_hash, old);
				f.type = frag_header->id;
				f.name = frag_header->name_ref;
				out.push_back(f);
				break;
			}
			case 0x22: {
				S3D::WLDFragment22 f(out, &buffer[idx], frag_header->size, frag_header->name_ref, current_hash, old);
				f.type = frag_header->id;
				f.name = frag_header->name_ref;
				out.push_back(f);
				break;
			}
			case 0x28: {
				S3D::WLDFragment28 f(out, &buffer[idx], frag_header->size, frag_header->name_ref, current_hash, old);
				f.type = frag_header->id;
				f.name = frag_header->name_ref;
				out.push_back(f);
				break;
			}
			case 0x29: {
				S3D::WLDFragment29 f(out, &buffer[idx], frag_header->size, frag_header->name_ref, current_hash, old);
				f.type = frag_header->id;
				f.name = frag_header->name_ref;
				out.push_back(f);
				break;
			}
			case 0x2D: {
				S3D::WLDFragment2D f(out, &buffer[idx], frag_header->size, frag_header->name_ref, current_hash, old);
				f.type = frag_header->id;
				f.name = frag_header->name_ref;
				out.push_back(f);
				break;
			}
			case 0x30: {
				S3D::WLDFragment30 f(out, &buffer[idx], frag_header->size, frag_header->name_ref, current_hash, old);
				f.type = frag_header->id;
				f.name = frag_header->name_ref;
				out.push_back(f);
				break;
			}
			case 0x31: {
				 S3D::WLDFragment31 f(out, &buffer[idx], frag_header->size, frag_header->name_ref, current_hash, old);
				 f.type = frag_header->id;
				 f.name = frag_header->name_ref;
				 out.push_back(f);
				 break;
			}
			case 0x36: {
				S3D::WLDFragment36 f(out, &buffer[idx], frag_header->size, frag_header->name_ref, current_hash, old);
				f.type = frag_header->id;
				f.name = frag_header->name_ref;

				out.push_back(f);
				break;
			}
			default:
				S3D::WLDFragment f;
				f.type = frag_header->id;
				f.name = frag_header->name_ref;
				out.push_back(f);
				break;
		}

		idx += frag_header->size - 4;
	}

	return true;
}
