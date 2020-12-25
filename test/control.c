int case_id_g;

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
    int x;
    if (0)
        x = 2;
    else
        x = 3;
    return x;
}
int case2()
{
    int x;
    if (1 - 1)
        x = 2;
    else
        x = 3;
    return x;
}
int case3()
{
    int x;
    if (1)
        x = 2;
    else
        x = 3;
    return x;
}
int case4()
{
    int x;
    if (2 - 1)
        x = 2;
    else
        x = 3;
    return x;
}
int case5()
{
    int i;
    int j;
    i = 0;
    j = 0;
    for (i = 0; i <= 10; i = i + 1)
        j = i + j;
    return j;
}
int case6()
{
    int i;
    i = 0;
    while (i < 10)
        i = i + 1;
    return i;
}
int case7()
{
    int i;
    i = 0;
    while (i < 10)
        i = i + 1;
    return i;
}

int main()
{
    case_id_g = 0;

    assert(3, case1());
    assert(3, case2());
    assert(2, case3());
    assert(2, case4());
    assert(55, case5());
    assert(10, case6());
    assert(10, case7());

    printf("control.c OK\n\n");
    return 0;
}