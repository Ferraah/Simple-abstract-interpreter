int a;
int b;
int c;

void main() {
  a = 1;
  b = 0;
  c = 3;
  b = c;
  assert(a == 1);
}