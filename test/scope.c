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
    int x;
    x = 2;
    {
        int x;
        x = 3;
    }
    assert(2, x);

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
    assert(2, y);

    int z;
    z = 2;
    {
        z = 3;
    }
    assert(3, z);
}