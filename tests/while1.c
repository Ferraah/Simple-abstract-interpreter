int a; // 1
int x;

void main() {
  a = 0; //2
  x = 0;
  //3
  while( a < 10 ){  //4
    a = a + 1; //5 

    x = 0;
    while(x < 10){
      x = x+1;
    }
    assert(x == 10);
  }

  assert(a == 10);
  assert( x > 0 );
  assert( x <= 10);
  //6
}
