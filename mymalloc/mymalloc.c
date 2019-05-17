#include "mymalloc.h"			//takes in our header file
#include <stddef.h>         	//definition for size_t
#include <stdio.h>              //definiton for print
#include <stdint.h>
#include <assert.h>

#define MYERROR(errnum, msg) do{ \
    lasterr = errnum; \
    myerror(msg, file, line); \
}while(0)

static char bigblock[MEMORY];

// to catch expected errors from free (which returns void)
// (allows EXPECT_OK and EXPECT_ERR macros to work, without
//  visually inspecting error messages)
static int lasterr = 0;
inline int mylasterr() {
    return lasterr;
}
static inline void clear_lasterr(){
    lasterr = 0;
}
static inline void myerror(char const* msg, char const*file, int const line){
    printf("Error: %s %s:%d\n", msg, file, line);
    fflush(stdout);
}

/** convert offset of an Md in bigblock to an Md* */
static inline Md* mdptr(Md offset){
    assert( offset >=0 && offset < MEMORY );
    Md* ret = (Md*) &bigblock[offset];
    // If we are not initialized yet, *ret might be garbage
    return ret;
}
/** AbsMd(md) is offset in bigblock of the next metadata */
static inline Md absMd(Md const md){
    return md>=0? md: -md;
}
/** offset of next metadata in bigblock (or MEMORY if no next metadata) */
static inline Md next_offset(Md const offset){
    return absMd(*mdptr(offset));
}

static inline int check_first(){
    static int first_time = 1;
    if(first_time){
        first_time=0;
        *mdptr(0) = -MEMORY; // init: next metadata past end of bigblock
        // size of free block = MEMORY - (offset(0) + MDSZ)
        assert( next_offset(0) == MEMORY );
#ifndef NDEBUG
        printf(" INIT:\n"); walk_md();
#endif
        return 1;
    }
    return 0;
}

void* mymalloc(size_t sz, char const*file, int const line){
    clear_lasterr();
#ifndef NDEBUG
    printf("\nmymalloc(%lu,...)",(long unsigned)sz); fflush(stdout);
#endif
    if( sz <= 0 ){
        MYERROR(1,"malloc cannot allocate zero or fewer bytes");
        return NULL;
    }
    check_first();
    // for each Blk b in list
    // for(int i=0; i<10; ++i) {printf(" %d",i); }
    Md offset;
    for(offset=0; offset<MEMORY; offset= next_offset(offset)){
        Md mdval = *mdptr(offset);
        //printf(" md[%d]=%d\n", (int)offset,(int)mdval);
        if(mdval < 0) {                     // -ve ~ free block
            Md free_sz = next_offset(offset) - (offset + MDSZ);
            // Even at end, next_offset==MEMORY, so free_sz calc stil OK.
            if( sz > free_sz ){
                continue;
            }
            if( sz == free_sz || sz==free_sz-1 || sz==free_sz-2 ){
                *mdptr(offset) = -mdval;    // make it positive ~ "USED"
#ifndef NDEBUG
                printf("mymalloc OK\n"); walk_md();
#endif
                return (void*) &bigblock[offset+MDSZ];
            }else{
                // convert FREE into USED + FREE', where FREE' has at least 1 byte
                //
                //       +--- this is -ve (FREE)
                //       |                       +--- this is +ve (USED)
                //       |                       |
                //       vv                      vv
                //         <---- free_sz ------->
                // FREE  MDffffffffffffffffffffffMD
                // --->  MDuuuuuuuuuuuuuuuuMDffffMD
                //         ^                     ^
                //         |                     |
                //         |                     +--- this stays same (== mdval)
                //         |
                //         +---- return this as void*
                //         <----- sz ----->
                //
                //       ^^                ^^
                //       these are modified.
                //       ^
                //       |
                //       +--- offset is here.
                //
                Md new_md_offset = (offset + MDSZ + sz);
                *mdptr(offset) = +new_md_offset;
                assert( mdval < 0 );
                *mdptr(new_md_offset) = mdval;
#ifndef NDEBUG
                printf("mymalloc OK\n"); walk_md();
#endif
                return (void*) &bigblock[offset+MDSZ];
            }
        }
    }
#ifndef NDEBUG
    printf("no-mem:\n"); walk_md();
    char msg[80];
    sprintf(&msg[0],"malloc(%lu) failed: out of memory",(long unsigned)sz);
#else
    char const* msg = "malloc failed: out of memory";
#endif
    MYERROR(2,&msg[0]);
    return NULL;
}
void myfree( void* ptr, char const* file, int const line ){
    clear_lasterr();
    if(check_first()){
        MYERROR(3,"No blocks malloc'ed, cannot free anything (probably ok)");
        return;
    }
    if( ptr == NULL ){
        MYERROR(44,"myfree(NULL) does nothing");
        return;
    }
    if(!( (char*)ptr >= &bigblock[MDSZ] && (char*)ptr < &bigblock[MEMORY] )){
        MYERROR(4,"myfree ptr not within valid range of bigblock");
        return;
    }
    Md poff = (char*)ptr - &bigblock[0];
#ifndef NDEBUG
    printf("\nmyfree(poff=%d,...)",(int)poff);
#endif
    int prev_offset = -1;
    Md prev_mdval = 0;
    Md offset;
    for(offset=0; offset<MEMORY; offset= next_offset(offset)){
        Md mdval = *mdptr(offset);
        //printf(" free:off=%d,md=%d\n",(int)offset,(int)mdval); fflush(stdout);
        if( poff == offset + MDSZ ){
            // ptr was given out by us, we must free it
            if( mdval < 0 ){
                MYERROR(5,"myfree: ptr already freed");
                return;
            }
            assert( mdval > offset + MDSZ && mdval <= MEMORY );
            mdval = -mdval;
            *mdptr(offset) = mdval;
            assert( *mdptr(offset) < 0 ); // free!
            // check offset free coalescing with next free
            Md nxt = next_offset(offset);
            if(nxt<MEMORY){
                Md nxtval = *mdptr(nxt);
                if( nxtval < 0 ){ // coalesce free blocks offset & nxt
                    *mdptr(offset) = nxtval;
                    mdval = nxtval;
                    //printf(" offset %d --> %d\n",(int)offset,(int)nxtval);
                }
            }
            // check for prev free coalescing with offset
            if( prev_mdval < 0 ){
                *mdptr(prev_offset) = mdval;
                //printf(" prev_offset %d --> %d\n",(int)prev_offset,(int)mdval);
            }
#ifndef NDEBUG
            printf("Block freed!\n"); walk_md();
#endif
            return;
        }
        prev_offset = offset;
        prev_mdval = mdval;
    }
    MYERROR(6,"myfree: ptr was either previously freed or never malloced");
}

#ifndef NDEBUG
int walk_md(){
    static int verbose=1;
    if(check_first())
        return 0;
    int prev_mdval=0;       // There is not previous [yet]
    Md offset;
    int nused=0; // count # valid metadata blocks
    for(offset=0; offset<MEMORY; offset= next_offset(offset)){
        Md mdval = *mdptr(offset);
        if(verbose>=1){
            printf(" walk: off %d val %d\n",(int)offset,(int)mdval);
            fflush(stdout);
        }
        assert( absMd(mdval) > MDSZ );
        assert( absMd(mdval) <= MEMORY );
        Md blk_sz = next_offset(offset) - (offset + MDSZ);
        assert( blk_sz > 0 );
        if(mdval < 0) {                     // -ve ~ free block
            // not allowed 2 free blocks in a row
            if( prev_mdval!=0 ) assert( prev_mdval > 0 );
        }
        if(mdval > 0) ++nused;
        prev_mdval = mdval;
    }
    if( nused == 0 ){ // we must have a single huge free block again
        assert( *mdptr(0) == -MEMORY );
    }
    if(verbose>=2){
        printf("\nwalk_md OK!\n");
        fflush(stdout);
    }
    return nused;
}
#endif

// vim: sw=4 ts=4 et cindent
