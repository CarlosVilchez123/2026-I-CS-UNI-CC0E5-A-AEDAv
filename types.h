#ifndef __TYPES_H__
#define __TYPES_H__

#include <cstddef>
#include <cstdint>
#include <shared_mutex>
#include <mutex>
#include <string>
#include <sstream>


using Type      = int;
using T1        = int;          // int en 32-bit, long long en 64-bit
using Ref       = long;         // referencia/ID de objeto
using size      = std::size_t;  // tamaño sin signo
using level     = std::size_t;  // profundidad en el árbol
using flag      = std::size_t;  // booleano de ancho de máquina (0/1)
using sindex    = std::ptrdiff_t; // índice con signo (para loops inversos)
using BTreeChar = char;
using TypeBTree = char; 

using OStream  = std::ostream;
using IStream  = std::istream;
using OSStream = std::ostringstream;
using ISStream = std::istringstream;
using String   = std::string;

using String = std::string;


inline size   asSize  (sindex i) { return static_cast<size>  (i); }
inline sindex asSIndex(size   i) { return static_cast<sindex>(i); }

using SMutex = std::shared_mutex;
template <typename M = SMutex> using SLock = std::shared_lock<M>;
template <typename M = SMutex> using ULock = std::unique_lock<M>;
template <typename K, typename V> struct KVResult { K key; V ref; };


#endif // __TYPES_H__