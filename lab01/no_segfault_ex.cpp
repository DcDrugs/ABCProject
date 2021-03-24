#include <iostream>
int main() {
    int a[5] = {1, 2, 3, 4, 5};
    unsigned total = 0;
<<<<<<< HEAD
    for (int j = 0; j < sizeof(a) / sizeof(a[0]); j++) {
=======
    for (int j = 0; j < sizeof(a); j++) {
>>>>>>> 288a0d4e7794452fa1c5e2203d20a995068bda9c
        total += a[j];
    }
    std::cout << "sum of array is " << total << std::endl;
}
