
#include "mymalloc.h"
#include "stdio.h"
#include <assert.h>
#include <stdint.h>
#include <sys/time.h>

//3.1 


int noops = 0;
#define EXPECT_ERR(...) do { \
    __VA_ARGS__; \
    if( !mylasterr() ){ \
        printf("\nOOPS an expected error did not occur\n"); \
        ++noops; \
    } \
}while(0)
#define EXPECT_OK(...) do { \
    __VA_ARGS__; \
    if( mylasterr() ){ \
        printf("\nOOPS expected no error, but got one\n"); \
        ++noops; \
    } \
}while(0)

#ifndef NDEBUG
#define WALK_MD walk_md()
#else
#define WALK_MD do{}while(0)
#endif

// code coverage: test 'free' coalescing adjacent free blocks
void testE(){
    struct timeval m ;
    struct timeval n;
    gettimeofday(&m , NULL);
    // with 3 contiguous used blocks, freeing the central block
    // as a couple of cases to check.
    char *p;
    char *q;
    char *r;
    // allocate and free in same order
    EXPECT_OK(p = (char*)malloc(10));
    EXPECT_OK(q = (char*)malloc(10));
    EXPECT_OK(r = (char*)malloc(10));
    EXPECT_OK(free(p));
    EXPECT_OK(free(q)); // check coalesces with prev free
    EXPECT_OK(free(r)); // coalesce prev and next 'free'
    assert( walk_md() == 0 );  // all blocks freed again.
    // next... allocate strange order ...
    EXPECT_OK(p = (char*)malloc(10));
    EXPECT_OK(q = (char*)malloc(10));
    EXPECT_OK(r = (char*)malloc(10));
    EXPECT_OK(free(q));
    EXPECT_OK(free(p)); // check coalesces with prev free
    EXPECT_OK(free(r)); // coalesce prev and next 'free'
    assert( walk_md() == 0 );  // all blocks freed again.
    // one more case to try quickly...
    //    FREE,USED,FREE  ---> single FREE block
    EXPECT_OK(p = (char*)malloc(10));
    EXPECT_OK(q = (char*)malloc(10));
    EXPECT_OK(free(q)); // coalesce with only following free space
    EXPECT_OK(free(p)); // coalesce with only following free space
    assert( walk_md() == 0 );  // all blocks freed again.
    gettimeofday( &n, NULL);
    double time = (n.tv_sec-m.tv_sec) + (( n.tv_usec - m.tv_usec)/10000000.0);
    printf(" %s DONE -- %0.5f s\n",__PRETTY_FUNCTION__, time);
}
void testF(){
    struct timeval m ;
    struct timeval n;
    gettimeofday(&m , NULL);
    {
        EXPECT_ERR(free( (void*)0x1234567 ));
        char *p;
        char *q;
        EXPECT_ERR(malloc(0));
        EXPECT_OK(p = (char*)malloc(MEMORY-MDSZ));
        EXPECT_OK(free(p));
        EXPECT_OK(p = (char*)malloc(MEMORY-MDSZ));
        EXPECT_ERR(q = (char*)malloc(1));
        EXPECT_OK(free(p));
        assert( walk_md() == 0 );  // all blocks freed again.
        (void)q; // avoid unusued var warning
    }
    static void* p[4096];
    int i;
    for(i=0; i<4096 && (p[i]=malloc(1)) != NULL; ++i){
        ;// we allocated p[i] succesfully
        assert( p[i]!=NULL );
    }
    // now we did not get p[i]...
    assert( p[i] == NULL );
    // free in same order allocated
    int j;
    for(j=0; j<i; ++j){
        EXPECT_OK(free(p[j]));
    }
    assert( walk_md() == 0 );
    //
    // same test, but free in opposite order
    for(i=0; i<4096 && (p[i]=malloc(1)) != NULL; ++i){
        ;// we allocated p[i] succesfully
    }
    // now we did not get p[i]...
    assert( p[i] == NULL );
    // free in same order allocated
    for(j=i-1; j>=0; --j){
        EXPECT_OK(free(p[j]));
    }
    assert( walk_md() == 0 );
    gettimeofday( &n, NULL);
    double time = ((n.tv_sec-m.tv_sec) + ( ( n.tv_usec - m.tv_usec)/10000000.0));
    printf(" %s DONE -- %0.5f s\n",__PRETTY_FUNCTION__, time);
}
void testA(){
    struct timeval m ;
    struct timeval n;
    gettimeofday(&m , NULL);
    int x = 0xdeadBEEF;
    EXPECT_ERR(free( (int*)(intptr_t)x ));
    (void)x;
    gettimeofday( &n, NULL);
    double time = (double)(n.tv_sec-m.tv_sec) + (( n.tv_usec - m.tv_usec)/10000000.0);
    printf(" %s DONE -- %0.5f s\n",__PRETTY_FUNCTION__, time);
}
void testB(){
    struct timeval m ;
    struct timeval n;
    gettimeofday(&m , NULL);
    char* p;
    EXPECT_OK(p = (char*)malloc( 200 ));
    EXPECT_ERR(free( p + 10 ));

    EXPECT_OK(free( p )); // clean up after the test
    assert( walk_md() == 0 );
    gettimeofday( &n, NULL);
    double time = ((n.tv_sec-m.tv_sec) + ( ( n.tv_usec - m.tv_usec)/10000000.0));
    printf(" %s DONE -- %0.5f s\n",__PRETTY_FUNCTION__, time);
}
void testC(){
    struct timeval m ;
    struct timeval n;
    gettimeofday(&m , NULL);
    char* p;
    EXPECT_OK(p = (char*)malloc(100));
    EXPECT_OK(free( p ));
    EXPECT_ERR(free( p ));
    EXPECT_OK(p = (char*)malloc( 100 ));
    EXPECT_OK(free( p ));
    EXPECT_OK(p = (char*)malloc( 100 ));
    EXPECT_OK(free( p ));
    assert( walk_md() == 0 );
    gettimeofday( &n, NULL);
    double time = ((n.tv_sec-m.tv_sec) + ( ( n.tv_usec - m.tv_usec)/10000000.0));
    printf(" %s DONE -- %0.5f s\n",__PRETTY_FUNCTION__, time);
}



///4
double workla(){
  struct timeval m ;
  struct timeval n;
  gettimeofday(&m , NULL);
  int i =0; 
  while( i<150) {
    void * a = malloc(sizeof(char));
    free(a);
    i++;
  }

  gettimeofday( &n, NULL);

  return ((double) ((n.tv_sec-m.tv_sec) + ( ( n.tv_usec - m.tv_usec)/10000000.0)));

}

double worklb(){
  struct timeval m ;
  struct timeval n;
  gettimeofday(&m , NULL);

  void * arr[50]; 

  int i =0;
  int s =0;
  while( s<3){
    while ( i<50){
      arr[i] = malloc( sizeof(char));
      i++;
    }
    int a =0;
    while( a<50){
      free( arr[a]);
      a++;
    }
    i=0;
    a=0;
    s++;
  }


  gettimeofday( &n, NULL);

  return ((double) ((n.tv_sec-m.tv_sec) + ( ( n.tv_usec - m.tv_usec)/10000000.0)));
}

double worklc(){
  struct timeval m ;
  struct timeval n;
  gettimeofday(&m , NULL);
  void  * arr[100];
  int e =0;
  while( e<50) {
    arr[e] = NULL;
    e++;

  }
  int malloced =0;
  int mall =0;
  int s =0;
  int free = 0;
  while( malloced<50) {
    if( mall ==50) {
      mall =0;
    }
    if( free ==50){
      free=0;
    }
    s = rand(); 
    //printf( "%i\n" , s);
    if( (s % 2)==0){

      if( arr[free] != NULL){
        //printf("inside");
        free( arr[free]);
        arr[free] = NULL;
        free++;

        malloced--;
      }
    }else{
      arr[mall] = malloc(1);
      malloced++;
      mall++;
    }
  }

  malloced =0;

  while( malloced <50){

    if( arr[malloced] != NULL) {

      free(arr[malloced]);
      s++;  
    }
    malloced++;
  }




  // printf( "%i\n" , count );

  gettimeofday( &n, NULL);

  return ((double) ((n.tv_sec-m.tv_sec) + ( ( n.tv_usec - m.tv_usec)/10000000.0)));

}

double worklE(){
  struct timeval m ;
  struct timeval n;
  gettimeofday(&m , NULL);
  void *arr[200]; 
  int i =1;
  int j =0; 
  while( i<100) {
    arr[j] = malloc(i);
    i= i+10;
    j++;
  }
  int s =0;
  while( s<j){
    free( arr[s]);
    s++;
  }
  gettimeofday( &n, NULL);

  return ((double) ((n.tv_sec-m.tv_sec) + ( ( n.tv_usec - m.tv_usec)/10000000.0)));
}
double worklf(){
  struct timeval m ;
  struct timeval n;
  gettimeofday(&m , NULL);
  int i =0; 
  while( i<1500) {
    void * a = malloc(sizeof(char));
    free(a);
    i++;
  }

  gettimeofday( &n, NULL);

  return ((double) ((n.tv_sec-m.tv_sec) + ( ( n.tv_usec - m.tv_usec)/10000000.0)));

}


double worklD() {

  struct timeval m ;
  struct timeval n;
  gettimeofday(&m , NULL);
  void  * arr[100];
  int e =0;
  while( e<50) {
    arr[e] = NULL;
    e++;

  }

  int i;
  int s =0;
  int count =0;
  //randomly choose between mallc or free at each step.
  // if malloc, use rand length
  for ( i =0 ; i<50; i++){
    s = rand() % 65;
    while( s< 1) {
      s = rand() % 65;// choosing a random number less than 64
    }
    if( s % 2 == 0) { // if even do free if not do malloc
      if( arr[count] != 0) {
        free(arr[count]);
        arr[count] =NULL;
        count++;
        //printf( "%s\n" , "inside");
        //printf( "%i\n" , s);
      }

    }else{
      // printf( "%i\n" , s);
      arr[i] = malloc(s);
    }
    //printf( "%p\n" , (void *)malloc(100));
  }
  while( count< 50){
    if( arr[count] != NULL){
      free( arr[count]);
    }
    count++;
  }

  // printf( "%i\n" , count );

  gettimeofday( &n, NULL);

  return ((double) ((n.tv_sec-m.tv_sec) + ( ( n.tv_usec - m.tv_usec)/10000000.0)));

}


int main(int argc,char**argv){

  double a =0;
  double b= 0;
  double c =0;
  double d =0;
  double e =0;
  double f =0;
  int i =0;
  while( i<100) {
    a =a + workla();
    b = b + worklb();
    c = c + worklc();
    d = d + worklD();
    e = e + worklE();
    f = f + worklf();
    i++;
  }
  a = a/100;
  b = b/100;
  c = c/100;
  d= d/100;
  e= e/100;
  f = f/100;
  printf("%0.8f\n" , a);
  printf("%0.8f\n" , b);
  printf("%0.8f\n" , c);
  printf("%0.8f\n" , d);
  printf("%0.8f\n" , e);
  printf("%0.8f\n" , f);
  testA();
    testB();
    testC();
    testE();
    testF();
  return 0;
}
// vim: ts=4 sw=4 et cindent

