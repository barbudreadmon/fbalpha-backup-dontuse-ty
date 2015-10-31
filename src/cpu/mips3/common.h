#ifndef COMMON
#define COMMON

#ifdef IOS
#include <tr1/cstdint>
#else
#include <cstdint>
#endif

#ifdef __GNUC__
#define ALIGN_DECL(n)   __attribute__ ((aligned (n)))
#elif __MSVC__
#else
#define ALIGN_DECL(n)   __declspec(align(n))
#endif
namespace mips
{
using namespace std;

typedef uint64_t addr_t;

}

#endif // COMMON

