// Identity.cpp

int main() {
  const unsigned int Dim = 10;
  float a[Dim][Dim];
  int i, j;

  for (i = 0; i < Dim; i++) {
    for (j = 0; j < Dim; j++) a[i][j] = 0.0;
  }

  for (i = 0; i < Dim; i++) a[i][i] = 1.0;
}