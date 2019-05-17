In this project i have made my own malloc to gain deeper understanding dynamically allocating memory. This was a group project and we both learned a lot from it. 

bigblock[MEMORY] is a data buffer from which we will malloc and
free using mymalloc and myfree functions. MEMORY is the size of
the memory block.
Md is metadata type. We call sizeof(Md) 'MDSZ'.
- If +ve, the following bytes are a used block.
- If -ve, the following bytes are a free block.
- The absolute value of the metadata is the offset within bigblock
of the following metadata block, and must be a forward offset
the the next metadata offset, or equal to MEMORY.
check_first() initializes bigblock[] metadata if this is the first time
we are
entering mymalloc/myfree. It initializes bigblock[0] to be a single
large
free block, whose metadata value must be -MEMORY (-ve for "free", and
absolute
value is MEMORY, so we know we are the last metadata block).
Md* mdptr(Md offset) takes in an offset and converts offset of an Md in
bigblock
to an Metadata pointer and returns it typecasted to Md*
Md absMd(Md const md) is the offset in bigblock of the next metadata. It
will always
return a negativ value of md type Md
Md next_offset(Md offset) takes an offset of a metadatablock in
bigblock[]
and returns the offset of the next metadata block; i.e., it returns the
absolute value of the metadata value beginning at bigblock[offset].
void *mymalloc(size_t sz, char const* file, int line) accepts a client
parameter sz and returns a pointer to a free region within bigblock[]
that is at least sz bytes long and that the client may freely use.
Parameters file and line are filled in for error reporting purposes
by macros malloc (and free) in mymalloc.h
The return value will always belocated MDSZ after a metadatablock whose
positive value indicates that the block is "used".
We start with the metadata block always at bigblock[0], and iterate
over the forward list maintained by absolute values of metadata entries.
When we find the first free block (-ve metadata value) of sufficient
size, we form the return value. Before returning, if the free space
is suffiently large, we create a new metablock for the remaining
free space, and insert it into the forward list of metadata blocks.
void free(void* ptr, char const* file, int line) frees a 'ptr' argument
that has previously been obtained from mymalloc.
At entry, we check that malloc has been called at least once (we are
initialized), then check that ptr lies within a feasible region of
bigblock[]. If so, then we search forward along the metadata blocks,
beginning at bigblock[0], for a metadata block positioned exactly
MDSZ before the offset of 'ptr' within bigblock.
If no such block exists, we report an error.
Otherwise, we examine the metadata associated with 'ptr', as well
as metadata of the immediately preceding (if any) and immediately
following block.
if ptr is described as 'free' (-ve metadata) we report a possibledouble-free error; otherwise we free the region by changing the
sign of its originally positive value.
We then maintain an invariant that we never allow two consecutive
blocks to occur. First we amalgamate the freed block of ptr with
its subsequent free block (if there is one). Thus the free block
at ptr may be enlarged from its original size. Then we amalgamate
a previous free block with the current (possibly larger) free block.
The most work done when we change a Free/Used/Free 3-block pattern
into a single Free block.
Invariants:
walk_md:
For debugging, we wrote a 'walk_md' function that we called when exiting
every call to mymalloc/myfree that checked invariants such as no
2 free blocks in a row, metadata blocks forming a forward list of
offsets into bigblock[], terminating at offset MEMORY, etc.
While possible to detect errors such as user overwriting metadata
by calling walk_md at the beginning of every mymalloc/myfree, we
did not do so, expecting proper usage (as per system malloc/free).
of not overwriting mymalloced memory regions.
0.00000020
0.00000176
0.00901781
0.00000033
0.00000004
0.00000204
above are the averages for all the tests from a to f.
they vary in seconds.
