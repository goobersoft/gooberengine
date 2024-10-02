


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

// converts a float to a fixed point integer.
#define ftoi(f) ((int)f*1000)

// typedefs
typedef size_t         index_t;     // an integer 0 to 4 billion
typedef unsigned char  bool_t;      // meant to be true or false
typedef unsigned char  byte_t;      // integer 0-255
typedef unsigned short ushort_t;    // integer 0-65535 
typedef unsigned int   uint_t;      // integer 0-4.2bil
typedef unsigned long  ulong_t;     // integer 0-1.844674407e19
typedef char *         cstr_t;      // a atring literal
typedef int            mint_t;      // "milli-int" stands for 1/1000th of a whole

typedef struct timeval timeval_t;

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

#define eq(a,b)   (a==b)
#define neq(a,b)  (a!=b)
#define gt(a,b)   (a>b)
#define gte(a,b)  (a>=b)
#define lt(a,b)   (a<b)
#define lte(a,b)  (a<=b)

#define bit(b)      (1<<(b))
#define bon(a,b)    (a | bit(b))
#define boff(a,b)   (a & (~bit(b)))
#define btog(a,b)   (a ^ bit(b))
#define bget(a,b)   ((a&bit(b))==bit(b))


// newline character
#define nl()            "\n"
// null terminator
#define nt()            '\0'

#define log             SDL_Log
#define ref(x)          (&x)
#define ptr(x)          (x*)
#define aptr(x)         (x**)
#define val(x)          x[0]
#define valn(x,n)       x[n]

#define var(x)          void*x

#define loop(i,b)       for(int i=0;i<b;i++)
#define loop2(i,a,b)    for(int i=a;i<b;i++)

#define type()          typedef struct
#define union()         typedef union

// this is used inside of a type() block.
// x: the name of the field
// t: the data type
// b: the amount of bits to use for the data type
// example:
//    field( f_visible, bool_t, 1 );
#define field(x,t,b)      t x:b

// local / foreign are just syntactic sugar.
// the purpose of these are to indicate when the object is freed:
//    LOCAL     fields are freed with the object
//    FOREIGN   fields do not get freed with the object
//    TRANSIENT fields negotiate their ownership with other modules
#define local(x)          x
#define foreign(x)        x
#define transient(x)      x

// null() -> NULL -> 0
#define null()            NULL

// arbitrary layer constants.
// you can use x as an offset
#define layer_top(x)      (1000+x)
#define layer_fg(x)       (800+x)
#define layer_normal(x)   (500+x)
#define layer_bg(x)       (200+x)
#define layer_bottom(x)   (x)

// It's PI * 100000
#define pi()              3141592

#define tile8(x)         (x/8)
#define tile10(x)        (x/10)

#define pixel8(x)        (x*8)
#define pixel10(x)       (x*10)

#define seconds(x)       (60*x)
#define milliseconds(x)  (1000*x)
#define microseconds(x)  (1000000*x)

//  function pointers
//    f1: function dest
//    f2: function source
//    rt: return type
//    ag: arglist (char,int), etc
//  example:
//    fnptr(do_thing,other_thing,int,(int,int,int))
#define fnptr(f1,f2,rt,ag) rt(*f1)ag=f2

// a: dest array
// b: source array
// ps: dest array start
// l: length
#define copy(a,b,ps,l)      \
  for(int i=0;i<l;i++) {    \
    a[ps+i]=b[i];           \
  }

// a: array
// l: array length
// v: value to use
#define clear(a,l,v)        \
  for(int i=0;i<l;i++) {    \
    a[i]=v;                 \
  }

#define cast(a,b)   ((b)a)
#define ptrtoint(a) (cast(a,uintptr_t))

#define isnull(a)  (a==null())
#define exists(a)  (a!=null())
#define iszero(a)  (a==0)
#define istrue(a)  (a==true())
#define isfalse(a) (a==false())

// I don't remember writing this.
#define new(a) a##_t*a(){a##_t*self=alloc(a##_t);init_##a(self);return self;}

#define center(wl,w) ((wl-w)/2) 