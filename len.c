#include <stdlib.h>
#include <stdio.h>

#include <ctype.h>
#include <limits.h>

#include <stdbool.h>
#include <string.h>

#define MY_GETLINE_TABWIDTH tabWidth
#define MY_GETLINE_TABSTOPS true

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
"len version 1.3: Line length checker\n"
"Usage: len [OPTIONS] {filename,-}\n"
"       Specify [-] as the last option to read from stdin.\n"
"Options:\n"
"       Short options not requiring arguments can be combined\n"
"       Arguments to long options are mandatory to their short\n"
"               counterparts as well\n"
"               i.e. -npr is equivalent to -n -p -r, but -nm is invalid\n"
"-m, --max: Specify a maximum line length (default: 80)\n"
"-M, --min: Specify a minimum line length (default: 1)\n"
"-t, --tab-width: Specify how many characters a tab counts as (default: 8)\n"
"-P, --print-matches: Print all lines that fall within the specified range\n"
"-p, --print-offenders: Print all lines that do not fall within the\n"
"                       specified range\n"
"-n, --line-numbers: Show line numbers\n"
"-c, --color: Enable coloring parts of lines based on other options\n"
"-r, --truncate: Truncate long lines\n"
"-l, --line-length: Show line lengths\n"
"-N, --count-newlines: Include newline characters when calculating line "
"length.\n"
"-i, --invert-colors: Makes -c color backgrounds instead of text\n"
"-a, --alternate-colors: Alternates colors on filenames. See below.\n"
"--file-color: Sets the primary filename color\n"
"--file-color-alt: Sets the secondary filename color. Only used if -a is "
"specified\n"
"--file-colors: Requires 2 colors as arguments. Equivalent to specifying both\n"
"               --file-color and --file-color-alt in that order\n"
"--set-bad: Set the color for out-of-range portions of lines\n"
"--set-good: Set the color for in-range portions of lines\n"
"--set-colors: Requires two arguments. Equivalent to specifying both\n"
"              --set-good and --set-bad in that order\n"
"-h, --help: Display this help and exit\n\n"
"Colors: red, green, yellow, blue, magenta, cyan, white\n"
"Return values:\n"
"    Other values indicate other errors (see man page for more details)\n"
"      0 - All lines were within the specified range\n"
"      1 - At least one line was out of the specified range\n";

/* Option prefix */
const char      OPTION         = '-';

/* Used to fill out short lines to maxLen */
const char      REAR_PADDING   = '-';

/* Short options */
const char      MAX             = 'm';
const char      MIN             = 'M';
const char      TABWIDTH        = 't';
const char      MATCHES         = 'P'; /* Uppercase */
const char      OFFENDERS       = 'p'; /* Lowercase */
const char      LINE_NUMS       = 'n';
const char      COLOR           = 'c';
const char      TRUNCATE        = 'r';
const char      LINE_LENGTHS    = 'l';
const char      NEWLINES        = 'N';
const char      INVERT          = 'i';
const char      HELP            = 'h';
const char      ALT             = 'a';

/* Specify this last to read from stdin */
const char      READ_STDIN     = '-';

/* Long options */
const char      *MAX_LONG           = "max";
const char      *MIN_LONG           = "min";
const char      *TABWIDTH_LONG      = "tab-width";
const char      *MATCHES_LONG       = "print-matches";
const char      *OFFENDERS_LONG     = "print-offenders";
const char      *LINE_NUMS_LONG     = "line-numbers";
const char      *COLOR_LONG         = "color";
const char      *HELP_LONG          = "help";
const char      *FLAGS_LONG         = "flags";
const char      *TRUNCATE_LONG      = "truncate";
const char      *NEWLINES_LONG      = "count-newlines";
const char      *LINE_LENGTHS_LONG  = "line-lengths";
const char      *INVERT_LONG        = "invert-colors";
const char      *ALT_LONG           = "alternate-colors";
const char      *SET_FILE_LONG      = "file-color";
const char      *SET_FILE_ALT_LONG  = "file-color-alt";
const char      *FILE_LONG          = "file-colors";
const char      *SET_BAD_LONG       = "set-bad";
const char      *SET_GOOD_LONG      = "set-good";
const char      *SET_COLORS_LONG    = "set-colors";

/* Color strings */
#define red_str     "red"
#define green_str   "green"
#define yellow_str  "yellow"
#define blue_str    "blue"
#define magenta_str "magenta"
#define cyan_str    "cyan"
#define white_str   "white"
#define def_str     "default"

const char      TRUNCATE_CHAR   = '+';
const char      TAB             = '\t';
const char      NULLCHAR        = '\0';

/* Return values */
const int       BAD_COMBINE             = 100;
const int       WHAT_IS_THAT_FLAG       = 101;
const int       MEM_EXCEEDED            = 102;
const int       BAD_FILE                = 103;
const int       BAD_ARGS                = 104;
const int       NO_ARGS                 = 105;

/* Default values */
static unsigned         maxLen          = 80;
static unsigned         minLen          = 1; /* Empty lines are 1 char long */
static unsigned         tabWidth        = 8;

/* Behavior flags controlled by args to program */
static bool             print           = false;
static bool             printAll        = false;
static bool             offenders       = true;   /* No effect without print */
static bool             lineNums        = false;
static bool             color           = false;
static bool             flags           = false;
static bool             truncate        = false;
static bool             newlines        = false; 
static bool             lineLengths     = false;
static bool             inverted        = false;
static bool             alternate       = false;
/* Flags for colors */
typedef const char *COLOR_T;

/* NSI color sequences: bold to make them easier to see */
#define red     "1;31"
#define green   "1;32"
#define yellow  "1;33"
#define blue    "1;34"
#define magenta "1;35"
#define cyan    "1;36"
#define white   "1;37"
#define def     "1;0" 

COLOR_T good_color = green;
COLOR_T bad_color  = red;
COLOR_T file_color = magenta;
COLOR_T file_alt   = cyan;


/****************************************************************************/

#define PRINTING (print || printAll)

/****************************************************************************/

/* WARNING: The following macros depend on non-global variables! Check their */
/*          scope before using them!                                         */

/* Don't forget that the last arg is a filename                          */
/* Used to make sure that the arguments given to flags that require them */
/* are "valid".                                                          */
/* Scope: arg_check */
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

/* Scope: File-level loop */
#define PRINT_FILENAME_HEADER                                                \
                if (numFiles > 1) {                                          \
                        if (PRINTING) {                                      \
                                if (fd != stdin){                            \
                                        term_file();                         \
                                        fprintf(stdout, "--|%d: %s|--\n",    \
                                                numFiles - (argc - i) + 1,   \
                                                argv[i]);                    \
                                        term_default();                      \
                                }                                            \
                                else {                                       \
                                        term_file();                         \
                                        fprintf(stdout, "--|%d: %s|--\n",    \
                                                numFiles - (argc - i) + 1,   \
                                                "Standard Input");           \
                                        term_default();                      \
                                }                                            \
                        }                                                    \
                }

/* Shorthand for matching long and short flags */
#define MATCH_S(I, J, SHORT_FLAG) argv[I][J] == SHORT_FLAG
#define MATCH_L(I, LONG_FLAG) !strcmp(&(argv[I][2]), LONG_FLAG)

/* Print flags for debugging purposes */
/* Printout enabled by the undocumented "--flags" option */
/* Does not interrupt normal execution */
inline static void print_flags(int i, int argc);

/* Functions relating to colors */
inline static void term_default();
inline static void term_color(bool isGood);
inline static void term_file();
static COLOR_T strtocolor(char *str);

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
        
        /* i gives the index of the first filename */
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

        /* Since getline counts newlines, we need to allow for them */
        if (!newlines) {
            ++maxLen;
        }

        /* These must persist and are set for each file examined */
        FILE *fd = NULL;
        char *buf = NULL;
        size_t size = 0;

        if (flags) print_flags(i, argc);

        /* If no file specified but '-' specified as last option, read from */
        /* stdin instead and reduce i so to pretend stdin is a file.        */
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
        /* cause the entire batch to be reported as bad                  */
        bool violated = false;

        /* Track per-file violations for conditional filename header printing */
        bool violatedHere = false;

        /* Process each remaining argument as a filename */
        for (; i < argc; ++i) {
                if (argv[i][0] == READ_STDIN) {
                        if (argv[i][1] == '\0')
                                fd = stdin;
                        else {
                                fd = NULL;
                        }
                }
                else fd = fopen(argv[i], "r");
                if (fd == NULL){
                        fprintf(stderr, "%s %s %s\n", "Could not open file",
                                                      argv[i], "for reading");
                        exit(BAD_FILE);
                }

                violatedHere = false;

                if (PRINTING && color) term_default();


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
                                /* Label files at first violation. Don't call */
                                /* out files completely within tolerance.     */
                                if (!violatedHere) PRINT_FILENAME_HEADER;
                                violated = true;
                                violatedHere = true;
                                if (!offenders && !printAll) continue;
                        } else {
                                if (offenders && !printAll) continue;
                        }

                        /* Line numbers up to 10^7 - 1. If your files are  */
                        /* longer than that, you have bigger problems than */
                        /* the output from this program not lining up      */
                        if (PRINTING && lineNums)
                                fprintf(stdout, "%7lu", (unsigned long) line);

                        /* Line lengths up to 10^3 - 1. If your  lines are */
                        /* longer than that, you have other problems.      */
                        if (PRINTING && lineLengths) {
                                fputc(' ', stdout);
                                if ((color) && (len != 1)){
                                        if (len < minLen || len > maxLen)
                                                term_color(false);
                                        else term_color(true);
                                }
                                /* We may or may not want to count newlines */
                                fprintf(stdout, "[%3lu]", newlines ?
                                                           len : (len - 1));
                                if (color) term_default();
                        }

                        if (PRINTING && (lineNums || lineLengths)) {
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
                                if (!overMaxLen && charCount >= (maxLen - 1)) {
                                        overMaxLen = true;
                                        if (PRINTING && color)
                                                term_color(false);
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
                                        overMinLen = true;
                                        if (PRINTING || !offenders) {
                                                if (color && (minLen != 1))
                                                        if ((len <= maxLen) ||
                                                            printAll)
                                                               term_color(true);
                                        }
                                }

                                if (PRINTING) {
                                        fprintf(stdout, "%c", buf[index]);
                                        ++charCount;
                                }
                        }

                        /* We don't want to rear pad if no minimum length */
                        /* is set.                                        */
                        /* Don't punish empty lines, but don't forget to  */
                        /* account for the newline in nonempty lines      */
                        if (minLen != 1 && len > 1 && charCount < minLen) {
                                if (PRINTING && color) term_color(false);
                                for (; charCount < (minLen - 1); ++charCount) {
                                        if (color) {
                                                fprintf(stdout, "%c",
                                                        REAR_PADDING);
                                        }
                                }
                        }

                        /* The last character should be a newline. We take */
                        /* this opporunity to reset terminal text color.   */
                        if (PRINTING && color) term_default();
                        if (PRINTING) fprintf(stdout, "%c", '\n');
                }

                if (fd != stdin) fclose(fd);
        }

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
                                } else if (MATCH_L(i, NEWLINES_LONG)) {
                                        newlines = true;
                                } else if (MATCH_L(i, INVERT_LONG)) {
                                        inverted = true;
                                } else if (MATCH_L(i, ALT_LONG)) {
                                        alternate = true;
                                } else if (MATCH_L(i, SET_GOOD_LONG)) {
                                        good_color = strtocolor(argv[++i]);
                                } else if (MATCH_L(i, SET_BAD_LONG)) {
                                        bad_color = strtocolor(argv[++i]);
                                } else if (MATCH_L(i, SET_COLORS_LONG)) {
                                        good_color = strtocolor(argv[++i]);
                                        bad_color = strtocolor(argv[++i]);
                                } else if (MATCH_L(i, SET_FILE_LONG)) {
                                        file_color = strtocolor(argv[++i]);
                                } else if (MATCH_L(i, SET_FILE_ALT_LONG)) {
                                        file_alt = strtocolor(argv[++i]);
                                } else if (MATCH_L(i, FILE_LONG)) {
                                        file_color = strtocolor(argv[++i]);
                                        file_alt = strtocolor(argv[++i]);
                                        alternate = true;
                                } else if (MATCH_L(i, HELP_LONG)) {
                                        fprintf(stdout, "%s", HELP_ME);
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
                        } else if (MATCH_S(i, j, NEWLINES)) {
                                newlines = true;
                        } else if (MATCH_S(i, j, INVERT)) {
                                inverted = true;
                        } else if (MATCH_S(i, j, ALT)) {
                                alternate = true;
                        } else if (MATCH_S(i, j, HELP)) {
                                fprintf(stdout, "%s", HELP_ME);
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
        fprintf(stderr, "%s: %s\n", "count newlines?",
                                    newlines ? "true" : "false");
        fprintf(stderr, "%s: %s\n", "alternate", alternate ? "true" : "false");
}

static const char *ESC = "\033[";
static const char *INV = "\033[1;7;";

inline static void term_default()
{
        printf("%s0m", ESC);
}

inline static void term_color(bool isGood)
{
        fprintf(stdout, "%s%sm", inverted ? INV : ESC,
                                 isGood ? good_color : bad_color);
}

inline static void term_file()
{
    static bool alt = false;
    if (color) {
        fprintf(stdout, "%s%sm", inverted ? INV : ESC,
                                 alt ? file_color : file_alt);
    }
    if (alternate) alt = !alt; 
}

static COLOR_T strtocolor(char *str)
{
        if (str == NULL) return def_str;

        if      (strcmp(str, red_str) == 0)     return red;
        else if (strcmp(str, green_str) == 0)   return green;
        else if (strcmp(str, yellow_str) == 0)  return yellow;
        else if (strcmp(str, blue_str) == 0)    return blue;
        else if (strcmp(str, magenta_str) == 0) return magenta;
        else if (strcmp(str, cyan_str) == 0)    return cyan;
        else if (strcmp(str, white_str) == 0)   return white;
        else if (strcmp(str, def_str) == 0)     return def;
        else return def;
}

/* Expanding tabs is controlled by the MY_GETLINE_TABWIDTH define */
/* If MY_GETLINE_TABWIDTH is defined, my_getline() will replace   */
/* \t with however many spaces MY_GETLINE_TABWIDTH evaluates to   */
size_t my_getline(char **buf, size_t *size, FILE *fd)
{
        static const int  INIT_SIZE = 256;
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
                        else ++i;
                }
                else {
                        (*buf)[i] = c;
                        /* UNIX line endings:       \n   */
                        /* OSX line endings:        \r   */
                        /* Windows line endings     \r\n */
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
                                        #if defined(MY_GETLINE_TABSTOPS)
                                        int dst_to_tabstop =
                                                ((i % MY_GETLINE_TABWIDTH) ? 
                                                (i % MY_GETLINE_TABWIDTH) :
                                                MY_GETLINE_TABWIDTH);
                                        for (size_t q = i;
                                             q < i + dst_to_tabstop;
                                             ++q) {
                                                (*buf)[q] = ' ';
                                        }
                                        i += dst_to_tabstop - 1;
                                        #else
                                        for (size_t q = i;
                                             q < i + MY_GETLINE_TABWIDTH;
                                             ++q) {
                                                (*buf)[q] = ' ';
                                        }
                                        i += MY_GETLINE_TABWIDTH - 1;
                                        #endif
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
