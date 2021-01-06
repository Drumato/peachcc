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
    x = 2;
    {
        int x;
        x = 3;
    }
    assert(1, 2, x);

    return 0;
}
int case2()
{
    int y;
    y = 2;
    {
        int y;
        y = 3;

        {
            int y;
            y = 4;
        }
    }
    assert(2, 2, y);

    return 0;
}
int case3()
{
    int z;
    z = 2;
    {
        z = 3;
    }
    assert(3, 3, z);

    return 0;
}
int main()
{
    case1();
    case2();
    case3();

    printf("scope.c OK\n\n");
    return 0;
}