#pragma once
#include <string>
#include <vector>
#include <functional>
#include <set>
#include <map>

namespace archivist{
	namespace sierra{

		namespace primitives{
			constexpr uint16_t ltou16(const uint16_t local)
			{
				#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__ 
					return ((0xff00 & local)>>8) | ((0x00ff & local)<<8) ;
				#else
					return local;
				#endif
			}
			
			constexpr uint32_t ltou32(uint32_t local)
			{
				#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__ 
					return 
					((0xff000000 & local)>>24) |
					((0x00ff0000 & local)>> 8) |
					((0x0000ff00 & local)<< 8) |
					((0x000000ff & local)<<24)
					;
				#else
					return local;
				#endif
			}
			
			constexpr uint64_t ltou64(const uint64_t local)
			{
				#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__ 
					uint32_t left  = (0xffffffff00000000 & local)>>32;
					uint32_t right = (0x00000000ffffffff & local);
					return 
						ltou32(left) |
						(((uint64_t)ltou32(right))<<32);
					;
				#else
					return local;
				#endif
			}
		}

		class SierraType;

		class SierraField{
		public:
			std::string name;
			SierraType* type;
			size_t offset;
			std::function<char*(char*)> converter;
		};

		class SierraType{
		public:
			std::string name;
			std::vector<SierraField> fields;
			size_t size;
		};

		class SierraContext{
		public:
			std::map<std::string,SierraType> _types;
			SierraContext();
		};

		extern std::function<void*(size_t)> sierra_allocate;
		extern std::function<void*(void*,size_t)> sierra_reallocate;
		extern std::function<void(void*)> sierra_free;
	}
}
 