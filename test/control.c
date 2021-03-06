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
int case7()
{
    int res;
    res = 0;
    for (int i = 1; i <= 10; i++)
        res = res + i;

    assert(7, 55, res);
    return 0;
}
int case8()
{
    int i;
    i = 3;
    int j;
    j = 0;
    for (int i = 0; i <= 10; i = i + 1)
        j = j + i;
    assert(8, 3, i);
    return 0;
}
int case9()
{
    int i;
    i = 0;
    goto a;
a:
    i++;
b:
    i++;
c:
    i++;
    assert(9, 3, i);
    return 0;
}
int case10()
{
    int i;
    i = 0;
    goto e;
d:
    i++;
e:
    i++;
f:
    i++;
    assert(10, 2, i);
    return 0;
}
int case11()
{
    int i;
    i = 0;
    goto i;
g:
    i++;
h:
    i++;
i:
    i++;
    assert(11, 1, i);
    return 0;
}
int case12()
{
    int i;
    i = 0;
    for (; i < 10; i++)
    {
        if (i == 3)
            break;
    }
    assert(12, 3, i);
    return 0;
}
int case13()
{
    int i;
    i = 0;
    while (1)
    {
        if (i++ == 3)
            break;
    }
    assert(13, 4, i);
    return 0;
}
int case14()
{
    int i;
    i = 0;
    for (; i < 10; i++)
    {
        for (;;)
            break;
        if (i == 3)
            break;
    }
    assert(14, 3, i);
    return 0;
}
int case15()
{
    int i;
    i = 0;
    while (1)
    {
        while (1)
            break;
        if (i++ == 3)
            break;
    }
    assert(15, 4, i);
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
    case12();
    case13();
    case14();
    case15();

    printf("control.c OK\n\n");
    return 0;
}