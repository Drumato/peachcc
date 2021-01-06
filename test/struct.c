int assert(int case_id, int expected, int actual)
{
    if (expected != actual)
    {
        printf("case%d() => %d expected but got %d\n", case_id, expected, actual);
        exit(1);
    }

    return 0;
}

int case0()
{
    struct
    {
        int a;
        int b;
    } x;
    x.a = 1;
    x.b = 2;

    assert(0, 1, x.a);
    assert(0, 2, x.b);
    assert(0, 3, x.a + x.b);

    return 0;
}
int case1()
{
    struct
    {
        int a;
        int b;
    } x[3];

    assert(1, 24, sizeof(x));

    x[0].a = 3;
    x[1].b = 5;
    x[2].a = 10;

    assert(1, 3, x[0].a);
    assert(1, 5, x[1].b);
    assert(1, 10, x[2].a);
    assert(1, 18, x[0].a + x[1].b + x[2].a);

    return 0;
}
int case2()
{
    struct
    {
        int vals[3];
    } x;
    x.vals[0] = 10;
    x.vals[1] = 20;
    x.vals[2] = 30;

    assert(2, 10, x.vals[0]);
    assert(2, 20, x.vals[1]);
    assert(2, 30, x.vals[2]);

    assert(2, 60, x.vals[0] + x.vals[1] + x.vals[2]);

    return 0;
}
int case3()
{
    struct
    {
        struct
        {
            char b;
        } a;
    } x;
    x.a.b = 6;
    assert(3, 6, x.a.b);
    return 0;
}

int case4()
{
    struct t
    {
        int a;
        int b;
    } x;
    struct t y;
    assert(4, 8, sizeof(y));
    return 0;
}

int case5()
{
    struct t
    {
        char a[2];
    };
    {
        struct t
        {
            char a[4];
        };
    }

    struct t y;
    assert(5, 2, sizeof(y));
    return 0;
}
int case6()
{
    struct t
    {
        int x;
    };
    int t;
    t = 1;
    struct t y;
    y.x = 2;
    assert(6, 3, t + y.x);
    return 0;
}
int case7()
{
    struct t
    {
        char a;
    } x;
    struct t *y;
    y = &x;
    x.a = 3;
    assert(7, 3, y->a);
    return 0;
}
int case8()
{
    struct t
    {
        char a;
    } x;
    struct t *y;
    y = &x;
    y->a = 3;
    assert(8, 3, x.a);
    return 0;
}

int main()
{

    case0();
    case1();
    case2();
    case3();
    case4();
    case5();
    case6();
    case7();
    case8();

    printf("struct.c OK\n\n");
    return 0;
}