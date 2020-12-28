// 本当はconst char *formatだけど
extern int printf(char *format, ...);

int main()
{
    struct
    {
        int foo;
        char bar;
        int baz;
    } x;
    x.foo = 30;
    x.bar = 'a';
    x.baz = 20;

    printf("x => {.foo = %d, .bar = %c, .baz = %d}\n", x.foo, x.bar, x.baz);
    return 0;
}