int a;
int b;
int c;
int z;

void main() {
  a = 1;
  z = 0;

  /*!npk b between 1 and 2 */
  c = a / b;

  c = (a / z)/z;
  assert(a ==1);
}
