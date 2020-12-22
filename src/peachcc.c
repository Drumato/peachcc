#include "peachcc.h"

static int read_file(const char *file_path, char **buf);
static bool parse_cmd_args(int argc, char **argv, CompileOption **cmd_opt);

int main(int argc, char **argv)
{
    peachcc_opt_g = calloc(1, sizeof(CompileOption));

    if (!parse_cmd_args(argc, argv, &peachcc_opt_g))
    {
        fprintf(stderr, "usage: ./peachcc [-d]<file-name>\n");
        exit(1);
    }

    // コンパイル対象の読み込み
    int status;
    if ((status = read_file(peachcc_opt_g->input_file, &c_program_g)) != 0)
    {
        fprintf(stderr, "read from %s failed.\n", peachcc_opt_g->input_file);
        exit(1);
    }

    TokenList *tokens = new_vec();
    tokenize(tokens, c_program_g);

    Program *program = parse(tokens);

    if (peachcc_opt_g->debug)
    {
        dump_ast(program);
    }

    // アセンブリの書き込み先のopen
    FILE *output_file;
    if ((output_file = fopen(peachcc_opt_g->output_file, "w")) == NULL)
    {
        perror("create output_file failed.");
        exit(1);
    }

    codegen(output_file, program);

    // トークンやASTのフリーは必ずプログラムの最後で
    free(cur_g);
    cur_g = NULL;
    free(peachcc_opt_g);
    peachcc_opt_g = NULL;

    return 0;
}

static int read_file(const char *file_path, char **buf)
{
    int size, status, fd;
    struct stat s;
    if ((fd = open(file_path, O_RDONLY)) < 0)
    {
        perror("open failed");
        return -1;
    }

    /* Get the size of the file. */
    if ((status = fstat(fd, &s)) != 0)
    {
        perror("fstat failed");
        return -1;
    }

    size = s.st_size;
    if ((*buf = (char *)mmap(0, size, PROT_READ, MAP_PRIVATE, fd, 0)) == (char *)-1)
    {
        perror("mmap failed");
        return -1;
    }

    return 0;
}

static bool parse_cmd_args(int argc, char **argv, CompileOption **cmd_opt)
{
    (*cmd_opt)->debug = false;
    (*cmd_opt)->output_file = "asm.s";
    (*cmd_opt)->input_file = "main.c";

    const struct option longopts[] = {
        //{    *name,           has_arg, *flag, val },
        {"input_file", required_argument, 0, 'o'},
        {"output_file", required_argument, 0, 'o'},
        {"debug", no_argument, 0, 'd'},
        {0, 0, 0, 0}, // termination
    };
    int longindex = 0;

    char c;

    while ((c = getopt_long(argc, argv, "di:o:", longopts, &longindex)) != -1)
    {
        switch (c)
        {
        case 'i':
            peachcc_opt_g->input_file = optarg;
            break;
        case 'o':
            peachcc_opt_g->output_file = optarg;
            break;
        case 'd':
            peachcc_opt_g->debug = true;
            break;
        default:
            break;
        }
    }
    return true;
}