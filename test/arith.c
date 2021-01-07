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
    assert(0, 0, 0); // 行コメントテスト2
    assert(0, 42, 42);
    assert(0, 13, 015);
    assert(0, 64, 0x40);
    assert(0, 15, 0b1111);
}
void case1()
{

    assert(1, 21, 5 + 20 - 4);
    assert(1, 41, 12 + 34 - 5);
}
void case2()
{
    assert(2, 47, 5 + 6 * 7);
    assert(2, 15, 5 * (9 - 6));
    assert(2, 4, (3 + 5) / 2);
}
void case3()
{
    assert(3, 10, -10 + 20);
    assert(3, 2, 5 % 3);
    assert(3, 1, 10 % 3);
    assert(3, 200, 3200 % 300);
    assert(3, 10, -(-10));
    assert(3, 10, -(-(+10)));

    assert(3, 0, !1);
    assert(3, 0, !2);
    assert(3, 1, !0);
}
void case4()
{
    /* ブロックコメントテスト1 */
    assert(4, 0, 0 == 1);
    assert(4, 1, 42 == 42);
    assert(4, 1, 0 != 1);
    assert(4, 0, 42 != 42);
}
void case5()
{
    /* ブロックコメントテスト2 
    */
    assert(5, 1, 0 < 1);
    assert(5, 0, 1 < 1);
    assert(5, 0, 2 < 1);
    assert(5, 1, 0 <= 1);
    assert(5, 1, 1 <= 1);
    assert(5, 0, 2 <= 1);
}
void case6()
{
    assert(6, 1, 1 > 0);
    assert(6, 0, 1 > 1);
    assert(6, 0, 1 > 2);
    assert(6, 1, 1 >= 0);
    assert(6, 1, 1 >= 1);
    assert(6, 0, 1 >= 2);
}
void case7()
{
    assert(7, 1, 1 || 0);
    assert(7, 1, 0 || 1);
    assert(7, 1, 1 || 1);
    assert(7, 0, 0 || 0);
    assert(7, 0, 1 && 0);
    assert(7, 0, 0 && 1);
    assert(7, 1, 1 && 1);
    assert(7, 0, 0 && 0);
}
void case8()
{
    assert(8, 1, 1 ? 1 : 0);
    assert(8, 0, 0 ? 1 : 0);
    assert(8, 1, 2 - 1 ? 1 : 0);
    assert(8, 0, 1 - 1 ? 1 : 0);
}

int main()
{
    // 行コメントテスト1
    case0();
    case1();
    case2();
    case3();
    case4();
    case5();
    case6();
    case7();
    case8();

    printf("arith.c OK\n\n");
    return 0;
}