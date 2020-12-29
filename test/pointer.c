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
    int x;
    x = 3;
    int *y;
    y = &x;
    int **z;
    z = &y;

    assert(1, 3, *&x);
    assert(1, 3, **z);

    return 0;
}
int case2()
{
    int x;
    x = 3;
    int y;
    y = 5;

    assert(2, 5, *(&x + 1));
    assert(2, 3, *(&y - 1));
    assert(2, 5, *(&x - (-1)));

    return 0;
}
int case3()
{
    int x;
    int *y;
    x = 3;
    y = &x;
    *y = 5;

    assert(3, 5, x);
    return 0;
}
int case4()
{
    int x;
    x = 3;
    int y;
    y = 5;
    *(&x + 1) = 7;
    assert(4, 7, y);
    return 0;
}
int case5()
{
    int x;
    x = 3;
    int y;
    y = 5;
    *(&y - 2 + 1) = 7;
    assert(5, 7, x);
    return 0;
}
int case6()
{
    int x;
    x = 3;
    assert(6, 5, (&x + 2) - &x + 3);
    return 0;
}
int case7()
{
    int x[2];
    int *y;
    y = &x;
    *y = 3;
    assert(7, 3, *x);
    return 0;
}
int case8()
{
    int x[3];
    *x = 3;
    *(x + 1) = 4;
    *(x + 2) = 5;
    assert(8, 3, *x);
    assert(8, 4, *(x + 1));
    assert(8, 5, *(x + 2));

    assert(8, 3, x[0]);
    assert(8, 4, x[1]);
    assert(8, 5, x[2]);
    return 0;
}
int case9()
{
    int x[2][3];
    int *y;
    y = x;
    *y = 0;
    *(y + 1) = 1;
    *(y + 2) = 2;
    *(y + 3) = 3;
    *(y + 4) = 4;
    *(y + 5) = 5;

    assert(9, 0, **x);
    assert(9, 1, *(*x + 1));
    assert(9, 2, *(*x + 2));
    assert(9, 3, **(x + 1));
    assert(9, 4, *(*(x + 1) + 1));
    assert(9, 5, *(*(x + 1) + 2));
}
int case10()
{
    int x[3];
    *x = 3;
    x[1] = 4;
    x[2] = 5;
    assert(10, 3, *x);
    assert(10, 4, *(x + 1));
    assert(10, 5, *(x + 2));
    return 0;
}
int case11()
{
    int x[2][3];
    int *y;
    y = x;
    y[0] = 0;
    y[1] = 1;
    y[2] = 2;
    y[3] = 3;
    y[4] = 4;
    y[5] = 5;

    assert(11, 0, x[0][0]);
    assert(11, 1, x[0][1]);
    assert(11, 2, x[0][2]);
    assert(11, 3, x[1][0]);
    assert(11, 4, x[1][1]);
    assert(11, 5, x[1][2]);
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
    case10();
    case11();

    printf("pointer.c OK\n\n");
    return 0;
}