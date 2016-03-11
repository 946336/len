# len
Line length utility

`len` is a small utility that counts line lengths from text files. Its behavior on other file types is undefined and probably not very nice.

By default, `len` does not produce any output on stdout or stderr unless examining multiple files.

When piping output from `len` to `less`, be sure to specify `-r` with `less` if you want colors to be displayed properly.

Feel free to glance at the spaghetti code that is the source code - I've tried to leave helpful comments, but those can only go so far.

<hr>
<h4>Options</h4>
**-m, --max** *`MAX_LINE_LENGTH`*<br>
Set the upper limit of line lengths. This cannot be negative or less than the lower limit, described below.<br>
Default: 80

**-M, --min** *`MIN_LINE_LENGTH`* <br>
Set the lower limit for line lengths. This cannot exceed the upper limit. Newlines are counted as one character.<br>
Default: 1

**-t, --tab-width** *`TAB_WIDTH`*<br>
Set the number of spaces a tab character is interpreted as, respecting tab stops.<br>
Default: 8

**-p, --print-offenders**<br>
Print lines that do not meet line length criteria. When multiple files are specified, lines are reported in sections with filename headers in the format `--|FILENAME|--`. Can be specified with `-P` to print all lines from all files.

**-P, --print-matches**<br>
Print lines that meet line length criteria. When multiple files are specified, lines are reported with filename headers in the format `--|FILENAME|--`. Can be specified with `-p` to print all lines from all files.

**-n, --line-numbers**<br>
Prefix lines with line numbers.

**-c, --color**<br>
Turns on color.

**-r, --truncate**<br>
Truncate longlines longer than *`MAX_LINE_LENGTH`* to a length of *`MAX_LINE_LENGTH`* and postfixes them with `+`.

**-l, --line-lengths**<br>
Prefix lines with their length.

**-N, --count-newlines**<br>
Include newlines when calculating line length. Note that all newline sequences (\n, \r, \r\n) are converted to a single newline character \n. When specified with `-c` or `--color`, colors change one character earlier, so that the correct number of characters are colored. For example, if the newline is the 81st character in the line and *`MAX_LINE_LENGTH`* is 80, the 80th character will be colored. Off by default.

**-h, --help**<br>
Display help and exit

<hr>
<h4>Tab Handling</h4>
`len` will expand tabs as far as the next tabstop, where tabstops are defined as places where the number of characters processed is a nonzero multiple of the tab width.

<hr>
<h4>Return Value</h4>
`len` returns 0 when all lines inspected had lengths within the specified/default range.
`len` returns 1 when at least one line inspected had a length outside the specified/default range.

Other return values are as follows:
* **100**: Cannot combine options that require arguments
* **101**: Unrecognized option
* **102**: Unable to allocate sufficient memory
* **103**: Could not open specified file
* **104**: Missing an argument to an option
* **105**: No options or arguments given
