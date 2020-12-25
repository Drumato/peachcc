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

    assert(0, ""[0]);
    assert(1, sizeof(""));

    assert(97, "abc"[0]);
    assert(98, "abc"[1]);
    assert(99, "abc"[2]);
    assert(0, "abc"[3]);
    assert(4, sizeof("abc"));

    assert(7, "\a"[0]);
    assert(8, "\b"[0]);
    assert(9, "\t"[0]);
    assert(10, "\n"[0]);
    assert(11, "\v"[0]);
    assert(12, "\f"[0]);
    assert(13, "\r"[0]);
    assert(27, "\e"[0]);

    assert(0, "\0"[0]);
    assert(16, "\20"[0]);
    assert(65, "\101"[0]);
    assert(104, "\1500"[0]);
    assert(0, "\x00"[0]);
    assert(119, "\x77"[0]);

    printf("string.c OK\n\n");
    return 0;
}