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
int case1(int num)
{
    int x;
    x = 3;
    int y;
    y = 5;

    if (num == 0)
    {
        return *(&x + 1);
    }
    else if (num == 1)
    {
        return *(&y - 1);
    }
    else
    {
        return *(&x - (-1));
    }
}
int case2()
{
    int x;
    x = 3;
    int y;
    y = 5;
    *(&x + 1) = 7;
    return y;
}
int case3()
{
    int x;
    x = 3;
    int y;
    y = 5;
    *(&y - 2 + 1) = 7;
    return x;
}
int case4()
{
    int x;
    x = 3;
    return (&x + 2) - &x + 3;
}

int main()
{
    case_id_g = 0;
    int x;
    x = 3;
    int *y;
    y = &x;
    int **z;
    z = &y;

    assert(3, *&x);
    assert(3, **z);
    assert(5, case1(0));
    assert(3, case1(1));
    assert(5, case1(2));

    *y = 5;
    assert(5, x);
    assert(7, case2());
    assert(7, case3());
    assert(5, case4());

    int a[3];
    int *ref_a;
    ref_a = &a;
    *ref_a = 3;

    assert(3, *a);

    *a = 3;
    *(a + 1) = 4;
    *(a + 2) = 5;

    assert(3, *a);
    assert(4, *(a + 1));
    assert(5, *(a + 2));

    int bb[2][3];
    int *ref_bb;
    ref_bb = bb;
    *ref_bb = 0;
    *(ref_bb + 1) = 1;
    *(ref_bb + 2) = 2;
    *(ref_bb + 3) = 3;
    *(ref_bb + 4) = 4;
    *(ref_bb + 5) = 5;

    assert(0, **bb);
    assert(1, *(*bb + 1));
    assert(2, *(*bb + 2));
    assert(3, **(bb + 1));
    assert(4, *(*(bb + 1) + 1));
    assert(5, *(*(bb + 1) + 2));

    a[0] = 3;
    a[1] = 4;
    a[2] = 5;
    assert(3, *a);
    assert(4, *(a + 1));
    2 [a] = 5;
    assert(10, *(a + 2));

    ref_bb[0] = 10;
    ref_bb[1] = 20;
    ref_bb[2] = 30;
    ref_bb[3] = 40;
    ref_bb[4] = 50;
    ref_bb[5] = 60;
    assert(10, bb[0][0]);
    assert(20, bb[0][1]);
    assert(30, bb[0][2]);
    assert(40, bb[1][0]);
    assert(50, bb[1][1]);
    assert(60, bb[1][2]);

    printf("pointer.c OK\n\n");
    return 0;
}