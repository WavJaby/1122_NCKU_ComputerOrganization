// int main(int a, int b) {
//     a = a / b;
// }
int main(int* p_c, int* p_a, int* p_b) {
    *p_c = *p_a++ / *p_b;
}