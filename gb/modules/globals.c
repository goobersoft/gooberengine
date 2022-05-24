

// keywords (which are only for syntactic saugar)
#define local     /* local */
#define external  /* external */

#define and  &&
#define AND  &
#define or   ||
#define OR   |
#define not  !
#define NOT  ~

// this stands for "function return" or "this function
// returns more than one value and uses param pointers for it."
#define fret void

//  definitions as functions:
//    There is no reason for there to be "true()" instead of "true"
//    but the aesthetics of the function indicate that the value is
//    a constant. You cannot assign a function to something else.
//    You will find though in later modules that this is contrary to
//    the case, but for all intents and purposes, most constants take
//    the form of a function which take no parameters.

// C does not have a sense of booleans. instead it relies on ints.
// this is good because trying to mix integers into boolean logic
// is annoying with other languages but is ok with C and C++
#define true()         1
#define false()        0
#define undefined()   -1

// the absolute highest limit for a 32-bit signed integer.
#define min_int()    -2147483648
#define max_int()     2147483647

// the app runs at 60 FPS - this is the millisecond measurement
#define tick_delay()   16667

// easter egg: Bob's LoL
// this is 2^20 (1048576) - 2^15 (32768) 
// result is (2^15)-(2^5)-1
// the number looks amusing to me, so maybe I might use it somewhere.
#define bobslol()     1015808

// converts a float to a fixed point integer.
#define ftoi(f) ((int)f*1000)

// typedefs

typedef size_t         index_t;
typedef unsigned char  bool_t;
typedef unsigned char  byte_t;
typedef unsigned short uword_t;
typedef unsigned int   uint_t;
typedef unsigned long  ulong_t;

// using calloc() will zero the entire memory block.
// I would always use this over malloc just in case of garbage data
#define alloc(x)     calloc(sizeof(x),1)
#define allocv(x,n)  calloc(sizeof(x),n)

// generic getters / setters / basic arithematic
#define get(a,b)     (a->b)
#define set(a,b,c)   ((a->b)=c)
#define add(a,b,c)   ((a->b)+=c)
#define mul(a,b,c)   ((a->b)*=c)
#define div(a,b,c)   ((a->b)/=c)

#define get2(a,b,c)   (a->b->c)
#define set2(a,b,c,d) (a->b->c=d)

#define streq(a,b)    (strcmp(a,b)==0)

#define eq(a,b)   a==b
#define neq(a,b)  a!=b
#define gt(a,b)   a>b
#define gte(a,b)  a>=b
#define lt(a,b)   a<b
#define lte(a,b)  a<=b

// newline character
#define nl()            "\n"

#define log             SDL_Log
#define ref(x)          (&x)
#define ptr(x)          void*x
#define ptrarr(x)       x**
#define val(x)          *x
#define val0(x,n)       x[n]

#define loop(i,a,b)     for(int i=a;i<b;i++)

#define type()            typedef struct
#define null()            NULL

#define layer_top(x)      (1000+x)
#define layer_fg(x)       (800+x)
#define layer_normal(x)   (500+x)
#define layer_bg(x)       (200+x)
#define layer_bottom(x)   (x)

#define pi()              3141

#define bin(f)      0xf
#define _1111       f
#define _1110       e
#define _1101       d
#define _1100       c
#define _1011       b
#define _1010       a
#define _1001       9
#define _1000       8
#define _0111       7
#define _0110       6
#define _0101       5
#define _0100       4
#define _0011       3
#define _0010       2
#define _0001       1
#define _0000       0

#define tile8(x)    (8*x)
#define tile10(x)   (10*x)

#define millisecond(x)  (1000*x)
#define microsecond(x)  (1000000*x)

//  function pointers
//    f1: function 1 identifier
//    f2: function 2 identifier
//    rt: return types
//    ag: arglist (char,int), etc
#define fnptr(f1,f2,rt,ag) rt(*f1)ag=f2

#define copy(a,b,i,ps,pf)       \
  for (int i=0;i<pf-ps;i++) {   \
    a[ps+i]=b[i];               \
  }


