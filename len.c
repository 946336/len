#include <stdlib.h>
#include <stdio.h>

#include <ctype.h>
#include <limits.h>

#include <stdbool.h>
#include <string.h>

#define MY_GETLINE_TABWIDTH tabWidth

/*******************************CONSTANTS************************************/

/* Error messages */
const char      *USAGE                  = "Usage: len [OPTIONS] {filename}";
const char      *BAD_ARG                = "Bad argument:";
const char      *BAD_STDIN              = "Cannot specify - with other files";
const char      *INCOMPLETE_ARG         = "Incomplete arguments to a flag";
const char      *NO_FILE                = "No file specified";
const char      *BAD_OPTION             = "Unrecognized option:";
const char      *NO_COMBINE             = "Cannot combine option:";

/* Help text */
const char *HELP_ME = 
"len version 1.1: Line length checker\n\n"
"Usage: len [OPTIONS] {filename,-}\n"
"       Specify [-] as the last option to read from stdin.\n\n"
"--NOTE-- len's behavior on non plaintext files is undefined\n"
"--NOTE-- len does not handle lines with nothing but whitespace\n"
"         particularly elegantly\n"
"--NOTE-- len does not support regular expressions\n\n"
"Options:\n"
"--NOTE-- Short options not requiring arguments can be combined\n"
"--NOTE-- Arguments to long options are mandatory to their short\n"
"         counterparts as well.\n"
"         i.e. -npr is equivalent to -n -p -r, but -nm is invalid\n"
"-m, --max: Specify a maximum line length(default: 80)\n"
"-M, --min: Specify a minimum line length (default: 1)\n"
"           --NOTE-- The newline character counts as a character\n"
"-t, --tab-width: Specify how many characters a tab counts as (default: 8)\n"
"-P, --print_matches: Print all lines that fall within the specified range\n"
"-p, --print-offenders: Print all lines that do no fall within the\n"
"                       specified range\n"
"-n, --line-numbers: Print line numbers as well (Off by default)\n"
"-c, --color: Enable coloring parts of lines that are outside of the\n"
"            specified range. Has no effect unless -P or -p is specified\n"
"-r, --truncate: Truncate long lines, indicated with \"+\"\n"
"-l, --line-length: Show line lengths\n"
"-h, --help: Display this help and exit\n\n"
"Return values:\n"
"\t  0 - All lines were within the specified range\n"
"\t  1 - At least one line was out of the specified range\n"
"\t255 - No arguments given\n"
"\t254 - Missing an argument or option\n"
"\t253 - Could not open file for reading\n"
"\t252 - Unable to allocate enough memory. Are you running low or is the\n"
"\t      file nothing but a single long line?\n"
"\t251 - Unrecognized option\n"
"\t250 - Bad combine. Cannot combine options that require arguments\n";


/* Option prefix */
const char      OPTION         = '-';

/* Used to fill out short lines to maxLen */
const char      REAR_PADDING   = '-';

/* Short options */
const char      MAX            = 'm';
const char      MIN            = 'M';
const char      TABWIDTH       = 't';
const char      MATCHES        = 'P'; /* Uppercase */
const char      OFFENDERS      = 'p'; /* Lowercase */
const char      LINE_NUMS      = 'n';
const char      COLOR          = 'c';
const char      TRUNCATE       = 'r';
const char      LINE_LENGTHS   = 'l';
const char      HELP           = 'h';

/* Specify this last to read from stdin */
const char      READ_STDIN     = '-';

/* Long arguments */
const char      *MAX_LONG            = "max";
const char      *MIN_LONG            = "min";
const char      *TABWIDTH_LONG       = "tab-width";
const char      *MATCHES_LONG        = "print-matches";
const char      *OFFENDERS_LONG      = "print-offenders";
const char      *LINE_NUMS_LONG      = "line-numbers";
const char      *COLOR_LONG          = "color";
const char      *HELP_LONG           = "help";
const char      *FLAGS_LONG          = "flags";
const char      *TRUNCATE_LONG       = "truncate";
const char      *LINE_LENGTHS_LONG   = "line-lengths";

const char      TRUNCATE_CHAR   = '+';

const char      TAB             = '\t';
const char      NULLCHAR        = '\0';

/* Return values */
const int       NO_ARGS                 = 255;
const int       BAD_ARGS                = 254;
const int       BAD_FILE                = 253;
const int       MEM_EXCEEDED            = 252;
const int       WHAT_IS_THAT_FLAG       = 251;
const int       BAD_COMBINE             = 250;

/* Default values */
static unsigned        maxLen          = 80;
static unsigned        minLen          = 1; /* Empty lines are 1 char long */
static unsigned        tabWidth        = 8;

/* Behavior flags controlled by args to program */
static bool            print           = false;
static bool            printAll        = false;
static bool            offenders       = true;   /* No effect without print */
static bool            lineNums        = false;
static bool            color           = false;
static bool            flags           = false;
static bool            truncate        = false;
static bool            lineLengths     = false;

/****************************************************************************/

/* Don't forget that the last arg is a filename                          */
/* Used to make sure that the arguments given to flags that require them */
/* are "valid"                                                           */
#define ARG_CHECK(I) if ((argc - 1) < ++I) {                                 \
                                fprintf(stderr, "%s %s %s\n", BAD_ARG,       \
                                argv[I - 1], "requires a numeric argument"); \
                                exit(BAD_ARGS);                              \
                     }                                                       \
                     if (!isdigit(argv[I][0])) {                             \
                                fprintf(stderr, "%s %s %s\n", BAD_ARG,       \
                                argv[I - 1], "requires a numeric argument"); \
                                exit(BAD_ARGS);                              \
                     }

/* Shothand for matching long ans short flags */
#define MATCH_S(I, J, SHORT_FLAG) argv[I][J] == SHORT_FLAG
#define MATCH_L(I, LONG_FLAG) !strcmp(&(argv[I][2]), LONG_FLAG)

/* Print flags for debugging purposes */
/* Printout enabled by the "flags" option */
inline static void print_flags(int i, int argc);

/* Changes the text color of the terminal */
inline static void term_red();
inline static void term_green();
inline static void term_default();

/* I won't use the GNU getline() here.                                   */
/* my_getline() will react to the MY_GETLINE_TABWIDTH macro by expanding */
/* tabs to that many spaces                                              */
size_t my_getline(char **buf, size_t *size, FILE *fd);

int parseArgs(int argc, char **argv);

int main(int argc, char **argv)
{
        if (argc == 1) {
                fprintf(stderr, "%s\n", USAGE);
                exit(NO_ARGS);
        }
        
        /* i will give the index of the first filename */
        int i = parseArgs(argc, argv);

        /* Sanity check: minLen must not be greater than maxLen          */
        if (maxLen < minLen) {
                fprintf(stderr, "%s\n", "Maximum length must be greater "
                                        "than minimum length!\n");
                exit(BAD_ARGS);
        }

        /* When the user specifies 0 chars, we have to account for the fact */
        /* that newlines are, in fact, characters, and will be counted by   */
        /* the program.                                                     */
        if (minLen == 0) minLen = 1;

        int numFiles = argc - i;

        /* These must persist and are set for each file examined */
        FILE *fd = NULL;
        char *buf = NULL;
        size_t size = 0;

        if (flags) print_flags(i, argc);

        /* If no file specified but '-' specified as last option, read from */
        /* stdin instead and reduce i so that we pretend stdin is a file.   */
        if ((argv[argc - 1][0] == READ_STDIN &&
            argv[argc - 1][1] == NULLCHAR)) {
                if (numFiles == 0) {
                        fd = stdin;
                        i--;
                }
        }

        /* If no file specified, print an error message and exit */
        if (argc - i < 1) {
                fprintf(stderr, "%s\n", NO_FILE);
                exit(BAD_FILE);
        }

        /* violated is tracked cumulatively. A violation in any file will */
        /* cause the enntire batch to be reported as bad                  */
        bool violated = false;

        /* Process each remaining argument as a filename */
        for (; i < argc; ++i) {
                if (argv[i][0] == READ_STDIN) fd = stdin;
                else fd = fopen(argv[i], "r");
                if (fd == NULL){
                        fprintf(stderr, "%s %s %s\n", "Could not open file",
                                                      argv[argc - 1],
                                                      "for reading");
                        exit(BAD_FILE);
                }

                if ((print || printAll) && color) term_default();

                /* In the case where more than one file is examined, we  */
                /* label the relevant contents of each file as such      */
                if (numFiles > 1) {
                        if ((print || printAll)) {
                                if (fd != stdin){
                                        fprintf(stdout, "\n--|%d: %s|--\n",
                                                numFiles - (argc - i) + 1,
                                                argv[i]);
                                }
                                else {
                                        fprintf(stdout, "\n--|%d: %s|--\n",
                                                numFiles - (argc - i) + 1,
                                                "Standard Input");
                                }
                        }
                }

                size_t line = 0;
                size_t len = -1;
                size_t index = 0;
                size_t charCount = 0;

                /* Assignment evaluates to the value assigned */
                while((len = my_getline(&buf, &size, fd)) !=
                                        (size_t) -1) {
                        /* Real life counting is 1-indexed */
                        ++line;

                        /* Don't process blank lines for violations,  */
                        /* but do print them when printing files      */
                        if (len == 1 && !printAll) continue;

                        /* Print lines that fit none, either, or any */
                        /* condition. Track violations of the range. */
                        /* Don't count newlines at the end of non    */
                        /* empty lines. ( > instead of >= )          */ 
                        if ((len < minLen || len > maxLen)) {
                                violated = true;
                                if (!offenders && !printAll) continue;
                        } else {
                                if (offenders && !printAll) continue;
                        }

                        /* Line numbers up to 10^7 - 1. If your files is   */
                        /* longer than that, you have bigger problems than */
                        /* the output from this program not lining up      */
                        if ((print || printAll) && lineNums) fprintf(stdout,
                                                             "%7lu",
                                                             (unsigned long)
                                                             line);

                        /* Line lengths up to 10^3 - 1. If your  lines are */
                        /* longer than that, you have other problems.      */
                        if ((print || printAll) && lineLengths) {
                                if ((color) && (len != 1)){
                                        if (len < minLen || len > maxLen)
                                                term_red();
                                        else term_green();
                                }
                                fprintf(stdout, " [%3u]", (unsigned) len);
                                if (color) term_default();
                        }

                        if ((print || printAll) && (lineNums || lineLengths)) {
                            fprintf(stdout, ": ");
                        }

                        bool overMaxLen = false;
                        bool overMinLen = false;
                        index = 0;

                        /* Yes, I know this looks stupid. Trust me. */
                        for (charCount = 0; charCount < (len - 1) &&
                                            index < (len - 1); ++index) {

                                /* Only turn red once we pass maxLen, but */
                                /* we have to remember that the length    */
                                /* counts the newline as a single char    */
                                /* and drops it in favor of a nullchar    */
                                if (!overMaxLen && charCount >= (maxLen - 1)) {
                                        overMaxLen = true;
                                        if ((print || printAll) && color)
                                                term_red();
                                        if (truncate) {
                                                fprintf(stdout, "%c",
                                                        TRUNCATE_CHAR);
                                                break;
                                        }
                                }

                                /* Only turn green once we pass minLen, but */
                                /* don't turn green if only printing lines  */
                                /* out of tolerance - not relevant there    */
                                if (!overMinLen &&
                                    (charCount >= (minLen - 1))) {
                                        if (print || printAll || !offenders) {
                                                overMinLen = true;
                                                if ((print || printAll)
                                                    && color && (minLen != 1))
                                                        if ((len <= maxLen) ||
                                                            printAll)
                                                                term_green();
                                        }
                                }

                                if ((print || printAll)) {
                                        fprintf(stdout, "%c", buf[index]);
                                        ++charCount;
                                }
                        }

                        /* We don't want to rear pad if no minimum length */
                        /* is set.                                        */
                        /* Don't punish empty lines, but don't forget to  */
                        /* account for the newline in nonempty lines      */
                        if (minLen != 1 && len > 1 && charCount < minLen) {
                                if ((print || printAll) && color) term_red();
                                for (; charCount < (minLen - 1); ++charCount) {
                                        if ((print || printAll) && color){
                                                fprintf(stdout, "%c",
                                                        REAR_PADDING);
                                        }
                                }
                        }

                        /* The last character should be a newline. We take */
                        /* this opporunity to reset terminal text color.   */
                        if ((print || printAll) && color) term_default();
                        if ((print || printAll))
                                fprintf(stdout, "%c", '\n');
                }

                if (fd != stdin) fclose(fd);
        }

        /* Extra newline at end of output for visual clarity */
        if (print || printAll) fprintf(stdout, "\n");

        free(buf);
        return violated ? EXIT_FAILURE : EXIT_SUCCESS;
}

int parseArgs(int argc, char **argv)
{
        int i;
        for (i = 1; argv[i] != NULL; ++(i)) {
                if (MATCH_S(i, 0, OPTION)){
                        if (MATCH_S(i, 1, NULLCHAR)) {
                                return i;
                        }                        
                        if (MATCH_S(i, 1, OPTION)){ /* Long form options */
                                if (MATCH_L(i, MAX_LONG)) {
                                        ARG_CHECK(i);
                                        maxLen = strtol(argv[i],
                                                        (char **)NULL, 10);
                                        break;
                                } else if (MATCH_L(i, MIN_LONG)) {
                                        ARG_CHECK(i);
                                        minLen = strtol(argv[i],
                                                 (char **)NULL, 10);
                                        break;
                                } else if (MATCH_L(i, TABWIDTH_LONG)) {
                                        ARG_CHECK(i);
                                        tabWidth = strtol(argv[i],
                                                         (char **)NULL, 10);
                                        break;
                                } else if (MATCH_L(i, MATCHES_LONG)) {
                                        if (offenders) printAll = true;
                                        if (!print) {
                                                print = true;
                                                offenders = false;
                                        }
                                } else if (MATCH_L(i, OFFENDERS_LONG)) {
                                        if (!offenders) printAll = true;
                                        if (!print) {
                                                print = true;
                                                offenders = true;
                                        }
                                } else if (MATCH_L(i, LINE_NUMS_LONG)) {
                                        lineNums = true;
                                } else if (MATCH_L(i, TRUNCATE_LONG)) {
                                        truncate = true;
                                } else if (MATCH_L(i, COLOR_LONG)) {
                                        color = true;
                                } else if (MATCH_L(i, FLAGS_LONG)) {
                                        flags = true;
                                } else if (MATCH_L(i, LINE_LENGTHS_LONG)) {
                                        lineLengths = true;
                                } else if (MATCH_L(i, HELP_LONG)) {
                                        fprintf(stdout, "%s\n", HELP_ME);
                                        exit(EXIT_SUCCESS);
                                /* Unrecognized option */
                                } else {
                                        fprintf(stderr, "%s [%s]\n",
                                                BAD_OPTION, argv[i]);
                                        exit(WHAT_IS_THAT_FLAG);
                                }
                        }

                        /* Loop through for short form options */
                        else for (int j = 1; argv[i][j] != '\0'; ++j) {
                        /* max cannot be combined with other options */
                        if (MATCH_S(i, j, MAX)) {
                                if ((j == 1) && (argv[i][j + 1] == NULLCHAR)) {
                                        ARG_CHECK(i);
                                        maxLen = strtol(argv[i],
                                                 (char **)NULL, 10);
                                        break;
                                }
                                else {
                                        fprintf(stderr, "%s [%c]\n",
                                                NO_COMBINE, argv[i][j]);
                                        exit(BAD_COMBINE);
                                }
                        /* min cannot be combined with other options */
                        } else if (MATCH_S(i, j, MIN)) {
                                if ((j == 1) && (argv[i][j + 1] == NULLCHAR)) {
                                        ARG_CHECK(i);
                                        minLen = strtol(argv[i],
                                                 (char **)NULL, 10);
                                        break;
                                }
                                else {
                                        fprintf(stderr, "%s [%c]\n",
                                                NO_COMBINE, argv[i][j]);
                                        exit(BAD_COMBINE);
                                }
                        /* tab-width cannot be combined with other options */
                        } else if (MATCH_S(i, j, TABWIDTH)) {
                                if ((j == 1) && (argv[i][j + 1] == NULLCHAR)) {
                                        ARG_CHECK(i);
                                        tabWidth = strtol(argv[i],
                                                   (char **)NULL, 10);
                                        break;
                                }
                                else {
                                        fprintf(stderr, "%s [%c]\n",
                                                NO_COMBINE, argv[i][j]);
                                        exit(BAD_COMBINE);
                                }
                        /* printAll overrides the offenders when set */
                        } else if (MATCH_S(i, j, MATCHES)) {
                                if (print) printAll = true;
                                if (!print) {
                                        print = true;
                                        offenders = false;
                                }
                        } else if (MATCH_S(i, j, OFFENDERS)) {
                                if (print) printAll = true;
                                if (!print) {
                                        print = true;
                                        offenders = true;
                                }
                        } else if (MATCH_S(i, j, LINE_NUMS)) {
                                lineNums = true;
                        } else if (MATCH_S(i, j, TRUNCATE)) {
                                truncate = true;
                        } else if (MATCH_S(i, j, COLOR)) {
                                color = true;
                        } else if (MATCH_S(i, j, LINE_LENGTHS)) {
                                lineLengths = true;
                        } else if (MATCH_S(i, j, HELP)) {
                                fprintf(stdout, "%s\n", HELP_ME);
                                exit(EXIT_SUCCESS);
                        /* Unrecognized option */
                        } else {
                                fprintf(stderr, "%s [%s]\n", BAD_OPTION,
                                        argv[i]);
                                exit(WHAT_IS_THAT_FLAG);
                        }

                        }
                } else break;
        }
        return i;
}

 /* Print all flags to check if they're set correctly */
inline static void print_flags(int i, int argc)
{
        fprintf(stderr, "argc: [%d], option groups specified:[%d]\n", argc, i);
        fprintf(stderr, "%s: %lu\n", "maxLen", (unsigned long) maxLen);
        fprintf(stderr, "%s: %lu\n", "minLen", (unsigned long) minLen);
        fprintf(stderr, "%s: %lu\n", "tabWidth", (unsigned long) tabWidth);
        fprintf(stderr, "%s: %s\n", "print", print ? "true" : "false");
        fprintf(stderr, "%s: %s\n", "printAll", printAll ? "true" : "false");
        fprintf(stderr, "%s: %s\n", "offenders",
                                    offenders ? "true" : "false");
        fprintf(stderr, "%s: %s\n", "matches", !offenders ? "true" : "false");
        fprintf(stderr, "%s: %s\n", "lineNums", lineNums ? "true" : "false");
        fprintf(stderr, "%s: %s\n", "color", color ? "true" : "false");
        fprintf(stderr, "%s: %s\n", "lineLengths",
                                    lineLengths ? "true" : "false");
}

static const char *ESC = "\033[";

inline static void term_red()
{
        printf("%s31m", ESC);
}

inline static void term_green()
{
        printf("%s32m", ESC);
}

inline static void term_default()
{
        printf("%s0m", ESC);
}

/* Expanding tabs is controlled by the MY_GETLINE_TABWIDTH define */
/* If MY_GETLINE_TABWIDTH is defined, my_getline() will replace   */
/* \t with however many spaces MY_GETLINE_TABWIDTH evaluates to   */
size_t my_getline(char **buf, size_t *size, FILE *fd)
{
        static const int INIT_SIZE = 256;
        static const char DELIM1 = '\n';
        static const char DELIM2 = '\r';
        static const char NULLCHAR = '\0';

        if (size == NULL) return (size_t) -1;

        if (*buf == NULL) {
                *buf = malloc(INIT_SIZE * sizeof(**buf));
                if (*buf == NULL) return (size_t) -1;
                *size = INIT_SIZE;
        }

        int c;
        size_t i = 0;

        char peeking;
        do {
                if ((i + 2) >= *size){
                        *buf = realloc(*buf, 2 * (*size) + 1);
                        *size *= 2;
                        if (*buf == NULL) {
                                return (size_t) -1;
                        }
                }

                c = fgetc(fd);
                if ((i + 2) >= *size){
                        *buf = realloc(*buf, 2 * (*size) + 1);
                        *size *= 2;
                        fprintf(stderr, "Size increased to %lu\n", *size);
                        if (*buf == NULL) {
                                return (size_t) -1;
                        }
                }
                /* Lines that are actually blank (no newline at all) are */
                /* reported as invalid. Should only happen in an empty   */
                /* file.                                                 */
                if (c < 0){
                        if (i == 0){
                                *size = -1;
                                return (size_t) -1;
                        }
                }
                else {
                        (*buf)[i] = c;
                        /* UNIX line endings:       \n   */
                        /* OSX line endings:        \r   */
                        /* Windows line eendings    \r\n */
                        #if defined(MY_GETLINE_TABWIDTH)
                                if ((*buf)[i] == TAB) {
                                        if (*size <=
                                            i + MY_GETLINE_TABWIDTH + 1) {
                                                *buf = realloc(*buf,
                                                              2 * (*size) + 1);
                                                *size *= 2;
                                                if (*buf == NULL)
                                                        return (size_t) -1;
                                        }
                                        for (size_t q = i;
                                             q < i + MY_GETLINE_TABWIDTH;
                                             ++q) {
                                                (*buf)[q] = ' ';
                                        }
                                        i += MY_GETLINE_TABWIDTH - 1;
                                }
                        #endif
                        /* UNIX or OS9 */
                        if (((*buf)[i] == DELIM1) || (*buf)[i] == DELIM2) {
                                /* Windows */
                                if ((c == DELIM2) &&
                                        (peeking = fgetc(fd)) != DELIM1)
                                                ungetc(peeking, fd);
                                (*buf)[++i] = NULLCHAR;
                                return i;
                        }
                        ++i;
                }
        } while(c > 0); /* EOF returns a negative value */

        (*buf)[i] = NULLCHAR;
        return i;
}
