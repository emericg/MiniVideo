/*!
 * \file      portable_endian.h
 * \license   https://creativecommons.org/publicdomain/zero/1.0/
 *
 * Original author:
 * - Mathias Panzenböck (https://gist.github.com/panzi/6856583)
 * Related resources:
 * - https://en.wikipedia.org/wiki/Endianness
 * - https://sourceforge.net/p/predef/wiki/Endianness/
 * - http://stackoverflow.com/questions/2100331/c-macro-definition-to-determine-big-endian-or-little-endian-machine
 */

#ifndef PORTABLE_ENDIAN_H
#define PORTABLE_ENDIAN_H
/* ************************************************************************** */

#if defined(__linux__) || defined(__CYGWIN__)

  #include <endian.h>

#elif defined(__APPLE__)

  #include <libkern/OSByteOrder.h>

  #define htobe16(x) OSSwapHostToBigInt16(x)
  #define htole16(x) OSSwapHostToLittleInt16(x)
  #define be16toh(x) OSSwapBigToHostInt16(x)
  #define le16toh(x) OSSwapLittleToHostInt16(x)

  #define htobe32(x) OSSwapHostToBigInt32(x)
  #define htole32(x) OSSwapHostToLittleInt32(x)
  #define be32toh(x) OSSwapBigToHostInt32(x)
  #define le32toh(x) OSSwapLittleToHostInt32(x)

  #define htobe64(x) OSSwapHostToBigInt64(x)
  #define htole64(x) OSSwapHostToLittleInt64(x)
  #define be64toh(x) OSSwapBigToHostInt64(x)
  #define le64toh(x) OSSwapLittleToHostInt64(x)

  #define __BYTE_ORDER      BYTE_ORDER
  #define __BIG_ENDIAN      BIG_ENDIAN
  #define __LITTLE_ENDIAN   LITTLE_ENDIAN
  #define __PDP_ENDIAN      PDP_ENDIAN

#elif defined(__OpenBSD__) || defined(__FreeBSD__)

  #include <sys/endian.h>

#elif defined(__NetBSD__) || defined(__DragonFly__)

  #include <sys/endian.h>

  #define be16toh(x) betoh16(x)
  #define le16toh(x) letoh16(x)

  #define be32toh(x) betoh32(x)
  #define le32toh(x) letoh32(x)

  #define be64toh(x) betoh64(x)
  #define le64toh(x) letoh64(x)

#elif defined(__sun)

  #include <sys/byteorder.h>

  #define htobe16(x) BE_16(x)
  #define htole16(x) LE_16(x)
  #define be16toh(x) BE_IN16(x)
  #define le16toh(x) LE_IN16(x)

  #define htobe32(x) BE_32(x)
  #define htole32(x) LE_32(x)
  #define be32toh(x) BE_IN32(x)
  #define le32toh(x) LE_IN32(x)

  #define htobe64(x) BE_64(x)
  #define htole64(x) LE_64(x)
  #define be64toh(x) BE_IN64(x)
  #define le64toh(x) LE_IN64(x)

#elif defined(_WIN16) || defined(_WIN32) || defined(_WIN64)

  #include <winsock2.h>

  #ifdef __MINGW32__
    #include <sys/param.h>
  #elif _MSC_VER
    // Windows doesn't provide any way to detect endianness through macros.
    // We force little endian because while there may be some NT kernel version
    // running on big endian (?), there is no known version of BE Windows...
    #define LITTLE_ENDIAN   1234
    #define BIG_ENDIAN      4321
    #define PDP_ENDIAN      3412
    #define BYTE_ORDER      LITTLE_ENDIAN
  #endif

  #define htobe16(x) htons(x)
  #define htole16(x) (x)
  #define be16toh(x) ntohs(x)
  #define le16toh(x) (x)

  #define htobe32(x) htonl(x)
  #define htole32(x) (x)
  #define be32toh(x) ntohl(x)
  #define le32toh(x) (x)

  #define htobe64(x) htonll(x)
  #define htole64(x) (x)
  #define be64toh(x) ntohll(x)
  #define le64toh(x) (x)

  #define __BYTE_ORDER      BYTE_ORDER
  #define __BIG_ENDIAN      BIG_ENDIAN
  #define __LITTLE_ENDIAN   LITTLE_ENDIAN
  #define __PDP_ENDIAN      PDP_ENDIAN

#else

  #error platform not supported

#endif

/* ************************************************************************** */
#endif // PORTABLE_ENDIAN_H
