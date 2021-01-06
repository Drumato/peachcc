static int decl();
static int decl2(int arg, char arg2);
static int decl3(int arg, char **arg2);

// 本当は const char *format, ... だけど，今は気にしない
extern int printf(char *format, ...);

static int assert(int case_id, int expected, int actual)
{
    if (expected != actual)
    {
        printf("case%d() => %d expected but got %d\n", case_id, expected, actual);
        exit(1);
    }

    return 0;
}
static int ret3()
{
    return 3;
    return 5;
}

static int add2(int x, int y)
{
    return x + y;
}

static int sub2(int x, int y)
{
    return x - y;
}

static int add6(int a, int b, int c, int d, int e, int f)
{
    return a + b + c + d + e + f;
}

static int addx(int *x, int y)
{
    return *x + y;
}

static int sub_char(char a, char b, char c)
{
    return a - b - c;
}
static int sub_long(long a, long b, long c)
{
    return a - b - c;
}

static int fib(int x)
{
    if (x <= 1)
        return 1;

    return fib(x - 1) + fib(x - 2);
}

int case1()
{
    assert(1, 3, ret3());
    return 0;
}
int case2()
{
    assert(2, 8, add2(3, 5));
    return 0;
}
int case3()
{
    assert(3, 2, sub2(5, 3));
    return 0;
}
int case4()
{
    assert(4, 21, add6(1, 2, 3, 4, 5, 6));
    return 0;
}
int case5()
{
    assert(5, 66, add6(1, 2, add6(3, 4, 5, 6, 7, 8), 9, 10, 11));
    return 0;
}
int case6()
{
    assert(6, 136, add6(1, 2, add6(3, add6(4, 5, 6, 7, 8, 9), 10, 11, 12, 13), 14, 15, 16));
    return 0;
}
int case7()
{
    assert(7, 55, fib(9));
    return 0;
}
int case8()
{
    assert(8, 1, sub_char(7, 3, 3));
    return 0;
}
int case9()
{
    assert(8, 1, sub_long(7, 3, 3));
    return 0;
}

int main()
{
    case1();
    case2();
    case3();
    case4();
    case5();
    case6();
    case7();
    case8();
    case9();

    printf("function.c OK\n\n");

    return 0;
}