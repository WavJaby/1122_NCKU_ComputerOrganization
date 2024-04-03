// int main(int a, int b) {
//     a = a / b;
// }

// int main(int* p_c, int* p_a, int* p_b) {
//     *p_c++ = *p_a++ / *p_b++;
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

int main(int i, int j, int f,
         int* p_x, int* p_h, int* p_y,
         int* h, int* x, int* y) {
    p_y[0] = p_h[0] * p_x[0] + p_h[1] * p_x[2] + p_h[2] * p_x[4]; // h11*x1+h12*x3+h13*x5
    p_y[1] = p_h[0] * p_x[1] + p_h[1] * p_x[3] + p_h[2] * p_x[5]; // h11*x2+h12*x4+h13*x6

    p_y[2] = p_h[3] * p_x[0] + p_h[4] * p_x[2] + p_h[5] * p_x[4]; // h21*x1+h22*x3+h23*x5
    p_y[3] = p_h[3] * p_x[1] + p_h[4] * p_x[3] + p_h[5] * p_x[5]; // h21*x2+h22*x4+h23*x6

    p_y[4] = p_h[6] * p_x[0] + p_h[7] * p_x[2] + p_h[8] * p_x[4]; // h31*x1+h32*x5+h33*x5
    p_y[5] = p_h[6] * p_x[1] + p_h[7] * p_x[3] + p_h[8] * p_x[5]; // h31*x2+h32*x4+h33*x6
    
    i = 3;
    j = 2;
    f = 3;
}