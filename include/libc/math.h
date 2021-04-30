static inline int abs(int a) {
    return a > 0? a: -a;
}

/* 判断给定的点是否在矩形内 */
static inline int in_rect(int x, int y, int w, int h, int a, int b) {
    if (a > x && a < x+w && b > y && b < y+h) {
        return 1;
    }
    return 0;
}