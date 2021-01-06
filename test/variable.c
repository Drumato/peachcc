extern int printf(char *format, ...);
int g1;
int g2[4];

int assert(int case_id, int expected, int actual)
{
    if (expected != actual)
    {
        printf("case%d() => %d expected but got %d\n", case_id, expected, actual);
        exit(1);
    }

    return 0;
}

int case1()
{
    int a;
    a = 3;
    assert(1, 3, a);

    return 0;
}
int case2()
{
    int a;
    a = 3;
    int z;
    z = 5;
    assert(2, 8, a + z);
    return 0;
}
int case3()
{
    int a;
    int b;
    a = b = 3;
    assert(3, 6, a + b);
    return 0;
}
int case4()
{
    int foo123;
    foo123 = 3;
    int bar;
    bar = 5;
    assert(4, 8, foo123 + bar);
    return 0;
}

int case5()
{
    int x;
    assert(5, 4, sizeof(x));
    return 0;
}

int case6()
{
    int x;
    assert(6, 4, sizeof x);
    return 0;
}
int case7()
{
    int *x;
    assert(7, 8, sizeof(x));
    return 0;
}
int case8()
{
    int x[4];
    assert(8, 16, sizeof(x));
    return 0;
}
int case9()
{
    int x[3][4];
    assert(9, 48, sizeof(x));
    return 0;
}
int case10()
{
    int x[3][4];
    assert(10, 16, sizeof(*x));
    return 0;
}
int case11()
{
    int x[3][4];
    assert(11, 4, sizeof(**x));
    return 0;
}
int case12()
{
    int x[3][4];
    assert(12, 5, sizeof(**x) + 1);
    return 0;
}
int case13()
{
    int x[3][4];
    assert(13, 5, sizeof **x + 1);
    return 0;
}
int case14()
{
    int x[3][4];
    assert(14, 4, sizeof(**x + 1));

    return 0;
}

int case15()
{
    int x;
    x = 1;
    assert(15, 4, sizeof(x = 2));
    return 0;
}
int case16()
{
    g2[0] = 0;
    g2[1] = 1;
    g2[2] = 2;
    g2[3] = 3;
    assert(16, 0, g2[0]);
    assert(16, 1, g2[1]);
    assert(16, 2, g2[2]);
    assert(16, 3, g2[3]);

    assert(16, 4, sizeof(g1));
    assert(16, 16, sizeof(g2));
    return 0;
}

int case17()
{
    char c;
    c = 1;
    assert(17, 1, c);
    assert(17, 1, sizeof(c));

    return 0;
}
int case18()
{
    char xx[10];
    assert(18, 10, sizeof(xx));
    return 0;
}
int case19()
{
    int foo;
    foo = 0;
    assert(19, 0, foo);
    foo++;
    assert(19, 1, foo);
    assert(19, 1, foo++);
    assert(19, 2, foo);
    assert(19, 3, ++foo);
    assert(19, 3, foo);
    assert(19, 3, foo--);
    assert(19, 2, foo);
    assert(19, 1, --foo);
    assert(19, 1, foo);
    return 0;
}
void case20()
{
    void *x;
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
    case10();
    case11();
    case12();
    case13();
    case14();
    case15();
    case16();
    case17();
    case18();
    case19();
    case20();

    printf("variable.c OK\n\n");
    return 0;
}
