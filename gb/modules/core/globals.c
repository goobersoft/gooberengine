


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

#define int2ptr(x)    ((void*)(size_t)(x))
#define ptr2int(x)    ((int)(size_t)(x))

// using calloc() will zero the entire memory block.
// I would always use this over malloc just in case of garbage data
#define alloc(x)     calloc(sizeof(x),1)
#define allocv(x,n)  calloc(sizeof(x),n)

#define clone(r,o)   (r[0]=o[0])

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
#define ptr(x)          (x*)
#define aptr(x)         (x**)
#define val(x)          (*x)
#define valn(x,n)       x[n]

#define var(x)          void*x

#define loop(i,b)         for(int i=0;i<b;i++)
#define loop2(i,a,b)      for(int i=a;i<b;i++)

#define type()            typedef struct
#define union()           typedef union

// this is used inside of a type() block.
// x: the name of the field
// t: the data type
// b: the amount of bits to use for the data type
// example:
//    field( f_visible, bool_t, 1 );
#define field(x,t,b)      t x:b

// local / foreign are just syntactic sugar.
// the purpose of these are to indicate when the object is freed:
//    LOCAL fields are freed with the object
//    FOREIGN fields do not get freed with the object
#define local(x)          x
#define foreign(x)        x

#define null()            NULL

#define layer_top(x)      (1000+x)
#define layer_fg(x)       (800+x)
#define layer_normal(x)   (500+x)
#define layer_bg(x)       (200+x)
#define layer_bottom(x)   (x)

#define pi()              3141592

#define tile8(x)         (8*x)
#define tile10(x)        (10*x)

#define milliseconds(x)  (1000*x)
#define microseconds(x)  (1000000*x)

//  function pointers
//    f1: function 1 identifier
//    f2: function 2 identifier
//    rt: return types
//    ag: arglist (char,int), etc
#define fnptr(f1,f2,rt,ag) rt(*f1)ag=f2

#define copy(a,b,ps,pf)         \
  for (int i=0;i<pf-ps;i++) {   \
    a[ps+i]=b[i];               \
  }

#define cast(a,b) ((b)a)

