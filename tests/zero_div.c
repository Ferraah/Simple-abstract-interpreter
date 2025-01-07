int a;
int b;
int c;
int z;

void main() {
  a = 1;
  z = 0;

  /*!npk b between 1 and 2 */

  // No errors expected
  c = a / b;
  // Error expected
  c = (a / z)/z;

}
