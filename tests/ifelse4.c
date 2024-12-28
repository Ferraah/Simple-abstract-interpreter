int a;
int b;

void main() {
  /*!npk a between 0 and 10 */
  /*!npk b between 0 and 10*/
  if(b > 8) {
    a = 1;
  }
  else {
    if(b != 5) {
      if(b != 2) {
        a = 2;
      }
    }
  }
  assert(a <= 10);
  assert(a >= 0);
}
