// int main(int a, int b) {
//     a = a / b;
// }

// int main(int i, int* p_c, int* p_a, int* p_b) {
//     *(p_c + i * 4) = *p_a / *p_b;
//     p_a = p_a + 4;
//     p_b = p_b + 4;
// }

/*
 * description: matrix - multiply benchmarking
 *
 *|h11 h12 h13| |x1 x2| |y1 y2| | h11*x1+h12*x3+h13*x5 h11*x2+h12*x4+h13*x6|
 *|h21 h22 h23|*|x3 x4|=|y3 y4|=| h21*x1+h22*x3+h23*x5 h21*x2+h22*x4+h23*x6|
 *|h31 h32 h33| |x5 x6| |y5 y6| | h31*x1+h32*x5+h33*x5 h31*x2+h32*x4+h33*x6|
 *
 * Element are to store in following order:
 *
 * matrix h[9]={h11,h12,h13, h21,h22,h23, h31,h32,h33}
 * vector x[6]={x1,x2, x3,x4, x5,x6}
 * vector y[6]={y1,y2, y3,y4, y5,y6}
 */
// int main(int i, int j, int f,
//          int* p_x, int* p_h, int* p_y,
//          int* h, int* x, int* y) {
//     p_h = h + (i * 3 + f) * 4;
//     p_x = x + (j + f * 2) * 4;
//     p_y = y + (i * 2 + j) * 4;

//     *p_y = *p_y + *p_h * *p_x;
// }

int main(int i,
         int* p_x, int* p_h, int* p_y,
         int* h, int* x, int* y) {
    for (i = 0; i < 3; i += 1) {
        for (int j = 0; j < 2; j += 1) {
            p_y = y + (i * 2 + j) * 4;
            for (int f = 0; f < 3; f += 1) {
                p_h = h + (i * 3 + f) * 4;
                p_x = x + (j + f * 2) * 4;

                *p_y w *p_y + *p_h * *p_x;
            }
        }
    }
}