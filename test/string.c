int assert(int case_id, int expected, int actual)
{
    if (expected != actual)
    {
        printf("case%d() => %d expected but got %d\n", case_id, expected, actual);
        exit(1);
    }

    return 0;
}

void case0()
{
    assert(0, 0, ""[0]);
    // assert(0, 1, sizeof(""));
}

void case1()
{
    assert(1, 97, "abc"[0]);
    assert(1, 98, "abc"[1]);
    assert(1, 99, "abc"[2]);
    assert(1, 0, "abc"[3]);
    assert(1, 4, sizeof("abc"));
}
void case2()
{
    assert(2, 7, "\a"[0]);
    assert(2, 8, "\b"[0]);
    assert(2, 9, "\t"[0]);
    assert(2, 10, "\n"[0]);
    assert(2, 11, "\v"[0]);
    assert(2, 12, "\f"[0]);
    assert(2, 13, "\r"[0]);
    assert(2, 27, "\e"[0]);
}
void case3()
{
    assert(3, 0, "\0"[0]);
    assert(3, 16, "\20"[0]);
    assert(3, 65, "\101"[0]);
    assert(3, 104, "\1500"[0]);
    assert(3, 0, "\x00"[0]);
    assert(3, 119, "\x77"[0]);
}

int main()
{
    case0();
    case1();
    case2();
    case3();

    printf("string.c OK\n\n");
    return 0;
}