#ifndef MYMALLOC_H
#define MYMALLOC_H 

#include <stdlib.h>

/** make malloc and free invoke our versions */
#define malloc(x) mymalloc(x,__FILE__,__LINE__)
#define free(x) myfree(x,__FILE__,__LINE__)

/** size of memory buffer from which we'll allocate */
#define MEMORY 4096

/** Our metadata is a single uint16_t.
 * - Md is positive for used blocks,
 * - negative for free blocks, and
 * - the absolute value is the offset of the next metadata block.
 * 
 * - Md blocks are contained in bigblock.
 * - An Md block at bigblock[offset] points:
 *   - either to the offset of the next valid Md
 *   - or to offset==MEMORY.
 *
 * Good tests may need to know sizeof(Md)
 */
typedef int16_t Md;
#define MDSZ sizeof(Md)


/** \return ptr to memory block of at least size bytes,
 * or NULL if out of memory. */
void* mymalloc(size_t sz, char const* file, int const line);
/** Free a \c ptr that was previously obtained from mymalloc */
void myfree(void* ptr, char const* file, int const line);

/** return nonzero error status of last call to mymalloc/myfree.
 * Use this to check that expected errors during 'free' actually
 * happened. Any call to mymalloc/myfree will set the error
 * status to zero. */
int mylasterr();

// optimized compilation is unable to access
// code that checks that invariants are satisfied
#ifndef NDEBUG
/** assert if there are invariants. Return
 * count of number of used Md blocks. */
int walk_md();
#endif

// vim: sw=4 ts=4 et cindent
#endif // MYMALLOC_H
