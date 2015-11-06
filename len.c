#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include <ctype.h>

#include <stdbool.h>
#include <string.h>

/*******************************CONSTANTS************************************/

/* Error messages */
const char      *USAGE                  = "Usage: len [OPTIONS] {filename}";
const char      *BAD_ARG                = "Bad argument:";
const char      *INCOMPLETE_ARG         = "Incomplete arguments to a flag";
const char      *NO_FILE                = "No file specified";
const char      *BAD_OPTION             = "Unrecognized option:";
const char      *NO_COMBINE             = "Cannot combine option:";

/* Help text */
const char *HELP_ME = 
"len version 1.0: Line length checker\n\n"
"Usage: len [OPTIONS] {filename}\n\n"
"--NOTE-- len's behavior on non plaintext files is undefined\n"
"--NOTE-- len does not handle lines with nothing but whitespace\n"
"         particularly elegantly\n"
"--NOTE-- len does not support regular expressions\n\n"
"Options:\n"
"--NOTE-- Short options not requiring arguments can be combined\n"
"         i.e. -npr is equivalent to -n -p -r, but -nm is invalid\n"
"-l, -m, --max: Specify a maximum line length(default: 80)\n"
"-M, --min: Specify a minimum line length (default: 1)\n"
"           --NOTE-- The newline character counts as a character\n"
"-t, --tab-width: Specify how many characters a tab counts as (default: 8)\n"
"-P, --print_matches: Print all lines that fall within the specified range\n"
"-p, --print-offenders: Print all lines that do no fall within the\n"
"                       specified range\n"
"-n, --line-numbers: Print line numbers as well (Off by default)\n"
"-c --color: Enable coloring parts of lines that are outside of the\n"
"            specified range. Has no effect unless -P or -p is specified\n"
"-r, --truncate: Truncate long lines, indicated with \"+\"\n"
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
const char      REAR_PADDING    = '-';

/* Short options */
const char      MAX1           = 'l';
const char      MAX2           = 'm';
const char      MIN            = 'M';
const char      TABWIDTH       = 't';
const char      MATCHES        = 'P'; /* Uppercase */
const char      OFFENDERS      = 'p'; /* Lowercase */
const char      LINE_NUMS      = 'n';
const char      COLOR          = 'c';
const char      HELP           = 'h';
const char      TRUNCATE       = 'r';

const char      TAB             = '\t';

const char      TRUNCATE_CHAR   = '+';

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

/* Return values */
const int       NO_ARGS                 = 255;
const int       BAD_ARGS                = 254;
const int       BAD_FILE                = 253;
const int       MEM_EXCEEDED            = 252;
const int       WHAT_IS_THAT_FLAG       = 251;
const int       BAD_COMBINE             = 250;

/* Behavior flags controlled by args to program */
unsigned        maxLen          = 80;
unsigned        minLen          = 1; /* Empty lines are 1 character long */
unsigned        tabWidth        = 8;
bool            print           = false;
bool            printAll        = false;
bool            offenders       = true;   /* Has no effect without print */
bool            lineNums        = false;
bool            color           = false;
bool            flags           = false;
bool            truncate        = false;

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
#define MATCH_L(I, LONG_FLAG)  !strcmp(&(argv[I][2]), LONG_FLAG)

/* Print flags for debugging purposes */
/* Printout enabled by the "flags" option */
inline static void print_flags(int i, int argc);

/* Changes the text color of the terminal */
inline static void term_red();
inline static void term_green();
inline static void term_default();

/* I won't use the GNU getline() here. */
size_t my_getline(char **buf, size_t *size, FILE *fd);

int main(int argc, char **argv)
{
        /* Suppress unused function warning */
        (void) print_flags;

        if (argc == 1) {
                fprintf(stderr, "%s\n", USAGE);
                exit(NO_ARGS);
        }
        
        /* Parse arguments */
        int i;
        for (i = 1; argv[i] != NULL; ++i) {
                if (MATCH_S(i, 0, OPTION)){
                        if (MATCH_S(i, 1, OPTION)){ /* Long form option */
                                if (MATCH_L(i, MAX_LONG)) {
                                        ARG_CHECK(i);
                                        maxLen = strtol(argv[i],
                                                        (char **)NULL, 10);
                                } else if (MATCH_L(i, MIN_LONG)) {
                                        ARG_CHECK(i);
                                        minLen = strtol(argv[i],
                                                 (char **)NULL, 10);
                                } else if (MATCH_L(i, TABWIDTH_LONG)) {
                                        ARG_CHECK(i);
                                        tabWidth = strtol(argv[i],
                                                         (char **)NULL, 10);
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
                                } else if (MATCH_L(i, HELP_LONG)) {
                                        fprintf(stdout, "%s\n", HELP_ME);
                                        exit(EXIT_SUCCESS);
                                /* Unrecognized option */
                                } else {
                                        fprintf(stderr, "%s %s\n", BAD_OPTION,
                                                argv[i]);
                                        exit(WHAT_IS_THAT_FLAG);
                                }
                        } /* Loop through for short form options */
                        else for (int j = 1; j < (int) strlen(argv[i]); ++j) {
                        /* max cannot be combined with other options */
                        if (MATCH_S(i, j, MAX1) || MATCH_S(i, j, MAX2)) {
                                if ((j == 1) && (argv[i][j + 1] == '\0')) {
                                        ARG_CHECK(i);
                                        maxLen = strtol(argv[i],
                                                 (char **)NULL, 10);
                                }
                                else {
                                        fprintf(stderr, "%s %c\n", NO_COMBINE,
                                                argv[i][j]);
                                        exit(BAD_COMBINE);
                                }
                        /* min cannot be combined with other options */
                        } else if (MATCH_S(i, j, MIN)) {
                                if ((j == 1) && (argv[i][j + 1] == '\0')) {
                                        ARG_CHECK(i);
                                        minLen = strtol(argv[i],
                                                 (char **)NULL, 10);
                                }
                                else {
                                        fprintf(stderr, "%s %c\n", NO_COMBINE,
                                                argv[i][j]);
                                        exit(BAD_COMBINE);
                                }
                        /* tab-width cannot be combined with other options */
                        } else if (MATCH_S(i, j, TABWIDTH)) {
                                if ((j == 1) && (argv[i][j + 1] == '\0')) {
                                        ARG_CHECK(i);
                                        tabWidth = strtol(argv[i],
                                                   (char **)NULL, 10);
                                }
                                else {
                                        fprintf(stderr, "%s %c\n", NO_COMBINE,
                                                argv[i][j]);
                                        exit(BAD_COMBINE);
                                }
                        /* Yes, this is terrible. I know */
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
                        } else if (MATCH_S(i, j, HELP)) {
                                fprintf(stdout, "%s\n", HELP_ME);
                                exit(EXIT_SUCCESS);
                        /* Unrecognized option */
                        } else {
                                fprintf(stderr, "%s %s\n", BAD_OPTION,
                                        argv[i]);
                                exit(WHAT_IS_THAT_FLAG);
                        }
                /* I still have no idea which set of braces I didn't close */
                /* properly                                                */
                } } else break;
        }

        /* Sanity check: minLen must not be greater than maxLen */
        /*               Both must also be nonnegative          */
        if (maxLen <= minLen) {
                fprintf(stderr, "%s\n", "Maximum length must be greater "
                                        "than minimum length!\n");
                exit(BAD_ARGS);
        }

        if (flags) print_flags(i, argc);

        /* If there isn't exactly one argument left unprocessed, fail */
        /* I don't think this is strictly necessary, but it's probably good */
        /* Form to have it anyway */
        if (argc - i < 1) {
                fprintf(stderr, "%s\n", NO_FILE);
                fprintf(stderr, "%s\n", USAGE);
                exit(BAD_ARGS);
        }

        /* These must persist and are set per file examined */
        FILE *fd = NULL;
        char *buf = NULL;
        int numFiles = argc - i;
        /* violated is tracked cumulatively. A violation in any file will */
        /* cause the enntire batch to be reported as bad                  */
        bool violated = false;

        for (; i < argc; ++i) {
                fd = fopen(argv[i], "r");
                if (fd == NULL){
                        fprintf(stderr, "%s %s %s\n", "Could not open file",
                                                      argv[argc - 1],
                                                      "for reading");
                        exit(BAD_FILE);
                }
                /* In the case where more than one file is examined, we */
                /* label the relevant contents of each file as such     */
                if (numFiles > 1) {
                        if ((print || printAll)) {
                                fprintf(stderr, "--|%d: %s|--\n",
                                        numFiles - (argc - i) + 1,
                                        argv[i]);
                        }
                }

                size_t size = 0;
                size_t line = 0;
                size_t len = -1;
                size_t index = 0;
                size_t charCount = 0;

                /* This is a weird trick. Change if weird things happen */
                /* Assignment evaluates to the value assigned           */
                while((len = my_getline(&buf, &size, fd)) !=
                                        (unsigned long) -1) {
                        ++line;

                        /* Don't consider blank lines */
                        if (len == 1 && !printAll) continue;

                        /* Print lines that fit none, either, or any */
                        /* condition. Track violations of the range  */ 
                        if ((len <= minLen || len >= maxLen)) {
                                violated = true;
                                if (!offenders && !printAll) continue;
                        } else {
                                if (offenders && !printAll) continue;
                        }

                        /* Line numbers up to 9999999 */
                        if ((print || printAll) && lineNums) fprintf(stderr,
                                                             "%7lu: ",
                                                             (unsigned long)
                                                             line);

                        bool overMaxLen = false;
                        bool overMinLen = false;
                        index = 0;

                        /* Yes, I know this is stupid. Trust me. */
                        for (charCount = 0; charCount < (len - 1) &&
                                            index < (len - 1); ++index) {
                                /* Only turn red once we pass maxLen*/
                                if (!overMaxLen && charCount > maxLen){
                                        overMaxLen = true;
                                        if ((print || printAll) && color)
                                                term_red();
                                        if (truncate) {
                                                index = charCount = len;
                                                fputc(TRUNCATE_CHAR, stdout);
                                        }
                                }
                                /* Only turn green once we pass minLen */
                                if (!overMinLen && (charCount > minLen)){
                                        if (print || printAll || !offenders) {
                                                overMinLen = true;
                                                if ((print || printAll)
                                                    && color && (minLen != 1))
                                                        term_green();
                                        }
                                }
                                /* Tabs count as tabWidth chars */
                                if (buf[index] == TAB){
                                        if ((print || printAll))
                                                for (size_t q = 0;
                                                     q < tabWidth; ++q)
                                                        fputc(' ', stdout);
                                        charCount += tabWidth;
                                } else if ((print || printAll)) {
                                        fputc(buf[index], stdout);
                                        ++charCount;
                                }
                        }
                        /* We don't want to rear pad if no minimum length */
                        /* is set.                                        */
                        /* Don't punish empty lines, but don't forget to  */
                        /* account for the newline in nonempty lines      */
                        if (minLen != 1 && len > 1 && charCount < minLen) {
                                if ((print || printAll) && color) term_red();
                                for (; charCount <= minLen; ++charCount) {
                                        if ((print || printAll) && color)
                                                fputc(REAR_PADDING, stdout);
                                }
                        }
                        // The last character should be a newline
                        if (print || printAll) term_default();
                        if (print || printAll) fputc(buf[len - 1], stdout);
                }
                /* Extra newline at end of output for visual clarity */
                if ((print || printAll) && violated) fputc('\n', stdout);

                fclose(fd);
        }

        free(buf);
        return violated ? EXIT_FAILURE : EXIT_SUCCESS;
}

 /* Print all flags to check if they're set correctly */
inline static void print_flags(int i, int argc)
{
        fprintf(stderr, "argc: [%d], i:[%d]\n", argc, i);
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

size_t my_getline(char **buf, size_t *size, FILE *fd)
{
        static const int INIT_SIZE = 256;
        static const char DELIM = '\n';

        if (size == NULL) return (size_t) -1;

        if (*buf == NULL) {
                *buf = malloc(INIT_SIZE * sizeof(**buf));
                if (*buf == NULL) return (size_t) -1;
                *size = INIT_SIZE;
        }

        int c;
        size_t i = 0;
        do {
                c = fgetc(fd);
                if (c < 0){
                        if (i == 0){
                                *size = -1;
                                return (size_t) -1;
                        }
                }
                else {
                        (*buf)[i] = c;
                        if ((*buf)[i] == DELIM) {
                                (*buf)[++i] = '\0';
                                return i;
                        }
                }
                if ((i++ + 1) == *size){
                        *buf = realloc(*buf, 2 * (*size) + 1);
                        *size += 2;
                        if (buf == NULL) {
                                return (size_t) -1;
                        }
                }
        } while(c > 0); /* EOF returns a negative value */

        /* It turns out we'll never get here */
        (*buf)[i] = '\0';
        return ++i;
}
