
/////////////////
// fixed point //
/////////////////

typedef int fixed_t;

fixed_t fixed(int n, int m) {
  return (n*1000)+(m%1000);
}

fixed_t fixed_add(int n1,int m1, int n2,int m2) {

}

fixed_t fixed_mul(int n1,int m1, int n2,int m2) {

}

void fixed_to_string ( fixed_t n, char * rt ) {
  sprintf(rt,"%d.%d",n/1000,n%1000);
}