int case_id_g;

static int decl();
static int decl2(int arg, char arg2);
static int decl3(int arg, char **arg2);

// 本当は const char *format, ... だけど，今は気にしない
int printf(char *format, ...);

static int assert(int expected, int actual)
{
    if (expected == actual)
    {
        printf("cases[%d] => %d\n", case_id_g, actual);
        case_id_g = case_id_g + 1;
    }
    else
    {
        printf("cases[%d] => %d expected but got %d\n", case_id_g, expected, actual);
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

static int fib(int x)
{
    if (x <= 1)
        return 1;
    return fib(x - 1) + fib(x - 2);
}

int main()
{
    case_id_g = 0;
    assert(3, ret3());
    assert(8, add2(3, 5));
    assert(2, sub2(5, 3));
    assert(21, add6(1, 2, 3, 4, 5, 6));
    assert(66, add6(1, 2, add6(3, 4, 5, 6, 7, 8), 9, 10, 11));
    assert(136, add6(1, 2, add6(3, add6(4, 5, 6, 7, 8, 9), 10, 11, 12, 13), 14, 15, 16));

    assert(7, add2(3, 4));
    assert(1, sub2(4, 3));
    assert(55, fib(9));

    assert(1, sub_char(7, 3, 3));

    printf("function.c OK\n\n");
    return 0;
}