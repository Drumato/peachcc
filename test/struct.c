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

int main()
{
    case_id_g = 0;

    struct
    {
        int a;
        int b;
    } x;
    x.a = 1;
    x.b = 2;

    assert(1, x.a);
    assert(2, x.b);
    assert(3, x.a + x.b);
    printf("struct.c OK\n\n");
    return 0;
}