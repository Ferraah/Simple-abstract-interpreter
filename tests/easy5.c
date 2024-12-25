int a;
int b;

void main() {
  /*!npk a between 0 and 2 */
  b = a*(2+a);
  assert(b >= 0);
  assert(b <= 8);
}
