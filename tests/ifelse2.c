int a;
int b;

void main() {
  /*!npk b between 0 and 3 */
  if(b > 2) {
    a = 1;
  }
  else {
    a = 2;
    if(b ==0) {
      a = 3;
    }
  }
  assert(a <= 2);
  assert(a >= 1);
}
