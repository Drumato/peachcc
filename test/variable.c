int case_id_g;
int g1;
int g2[4];

int assert(int expected, int actual)
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

int case1()
{
    int a;
    a = 3;
    return a;
}
int case2()
{
    int a;
    a = 3;
    int z;
    z = 5;
    return a + z;
}
int case3()
{
    int a;
    int b;
    a = b = 3;
    return a + b;
}
int case4()
{
    int foo123;
    foo123 = 3;
    int bar;
    bar = 5;
    return foo123 + bar;
}

int case5()
{
    int x;
    return sizeof(x);
}
int case6()
{
    int x;
    return sizeof x;
}
int case7()
{
    int *x;
    return sizeof(x);
}
int case8()
{
    int x[4];
    return sizeof(x);
}
int case9()
{
    int x[3][4];
    return sizeof(x);
}
int case10()
{
    int x[3][4];
    return sizeof(*x);
}
int case11()
{
    int x[3][4];
    return sizeof(**x);
}
int case12()
{
    int x[3][4];
    return sizeof(**x) + 1;
}
int case13()
{
    int x[3][4];
    return sizeof **x + 1;
}
int case14()
{
    int x[3][4];
    return sizeof(**x + 1);
}

int case15()
{
    int x;
    x = 1;
    return sizeof(x = 2);
}
int main()
{
    case_id_g = 0;

    assert(3, case1());
    assert(8, case2());
    assert(6, case3());

    assert(8, case4());

    assert(8, case5());
    assert(8, case6());
    assert(8, case7());
    assert(32, case8());
    assert(96, case9());
    assert(32, case10());

    assert(8, case11());
    assert(9, case12());
    assert(9, case13());
    assert(8, case14());
    assert(8, case15());
    assert(0, g1);

    g1 = 3;
    assert(3, g1);

    g2[0] = 0;
    g2[1] = 1;
    g2[2] = 2;
    g2[3] = 3;
    assert(0, g2[0]);
    assert(1, g2[1]);
    assert(2, g2[2]);
    assert(3, g2[3]);

    assert(8, sizeof(g1));
    assert(32, sizeof(g2));

    char c;
    c = 1;
    assert(1, c);
    assert(1, sizeof(c));

    char xx[10];
    assert(10, sizeof(xx));

    int foo;
    foo = 0;
    assert(0, foo);
    foo++;
    assert(1, foo);
    assert(1, foo++);
    assert(2, foo);
    assert(3, ++foo);
    assert(3, foo);
    assert(3, foo--);
    assert(2, foo);
    assert(1, --foo);
    assert(1, foo);

    int *bar;
    bar = &foo;
    assert(3, *bar++);

    printf("variable.c OK\n\n");
    return 0;
}
