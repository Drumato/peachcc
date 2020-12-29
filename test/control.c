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
    if (0)
        x = 2;
    else
        x = 3;
    assert(1, 3, x);
    return 0;
}
int case2()
{
    int x;
    if (1 - 1)
        x = 2;
    else
        x = 3;
    assert(2, 3, x);
    return 0;
}
int case3()
{
    int x;
    if (1)
        x = 2;
    else
        x = 3;
    assert(3, 2, x);
    return 0;
}
int case4()
{
    int x;
    if (2 - 1)
        x = 2;
    else
        x = 3;
    assert(4, 2, x);
    return 0;
}
int case5()
{
    int i;
    int j;
    i = 0;
    j = 0;
    for (i = 0; i <= 10; i = i + 1)
        j = i + j;

    assert(5, 55, j);
    return 0;
}
int case6()
{
    int i;
    i = 0;
    while (i < 10)
        i = i + 1;
    assert(6, 10, i);
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

    printf("control.c OK\n\n");
    return 0;
}