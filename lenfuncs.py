#!/usr/bin/env python2

import subprocess
import sys

class lenArgs:
    # No arguments
    PRINT_OFFENDERS = "--print-offenders"
    PRINT_MATCHES = "--print-matches"
    LINE_NUMBERS = "--line-numbers"
    COLOR = "--color"
    TRUNCATE = "--truncate"
    LINE_LENGTHS = "--line-lengths"
    COUNT_NEWLINES = "--count-newlines"
    INVERT_COLOR = "--invert-colors"
    ALTERNATE_COLORS = "--alternate-colors"
    HELP = "--help"

    # Short versions
    PRINT_OFFENDERS_SHORT = "-p"
    PRINT_MATCHES_SHORT = "-P"
    LINE_NUMBERS_SHORT = "-n"
    COLOR_SHORT = "-c"
    TRUNCATE_SHORT = "-r"
    LINE_LENGTHS_SHORT = "-l"
    COUNT_NEWLINES_SHORT = "-N"
    INVERT_COLOR_SHORT = "-i"
    ALTERNATE_COLORS_SHORT = "-a"

    # Requires one argument
    MAX = "--max"
    MIN = "--min"
    TAB_WIDTH = "--tab-width"
    FILE_COLOR = "--file-color"
    FILE_COLOR_ALT = "--file-color-alt"
    SET_BAD = "--set-bad"
    SET_GOOD = "--set-good"

    # Short versions
    MAX_SHORT = "-m"
    MIN_SHORT = "-M"
    TAB_WIDTH_SHORT = "-t"

    # Path to binary
    NAME = "PATH_TO_EXECUTABLE/len"

    # Colors
    RED = "red"
    GREEN = "green"
    YELLOW = "yellow"
    BLUE = "blue"
    MAGENTA = "magenta"
    CYAN = "cyan"
    WHITE = "white"

def header(cmdlist):
    header = "Command: \n"
    header += lenArgs.NAME + " " + " ".join(cmdlist) + "\n\n"
    header += "Flags:\n"
    for e in range(1, len(cmdlist)):
        if cmdlist[e] == lenArgs.PRINT_OFFENDERS:
            header += ("\t" + lenArgs.PRINT_OFFENDERS + "\n")
        elif cmdlist[e] == lenArgs.PRINT_MATCHES:
            header += ("\t" + lenArgs.PRINT_MATCHES + "\n")
        elif cmdlist[e] == lenArgs.LINE_NUMBERS:
            header+= ("\t" + lenArgs.LINE_NUMBERS + "\n")
        elif cmdlist[e] == lenArgs.COLOR:
            header += ("\t" + lenArgs.COLOR + "\n")
        elif cmdlist[e] == lenArgs.TRUNCATE:
            header += ("\t" + lenArgs.TRUNCATE + "\n")
        elif cmdlist[e] == lenArgs.LINE_LENGTHS:
            header += ("\t" + lenArgs.LINE_LENGTHS + "\n")
        elif cmdlist[e] == lenArgs.COUNT_NEWLINES:
            header += ("\t" + lenArgs.COUNT_NEWLINES + "\n")
        elif cmdlist[e] == lenArgs.INVERT_COLOR:
            header += ("\t" + lenArgs.INVERT_COLOR + "\n")
        elif cmdlist[e] == lenArgs.ALTERNATE_COLORS:
            header += ("\t" + lenArgs.ALTERNATE_COLORS + "\n")
        elif cmdlist[e] == lenArgs.MAX:
            header += ("\t" + lenArgs.MAX + " " + cmdlist[e + 1] + "\n")
        elif cmdlist[e] == lenArgs.MIN:
            header += ("\t" + lenArgs.MIN + " " + cmdlist[e + 1] + "\n")
        elif cmdlist[e] == lenArgs.TAB_WIDTH:
            header += ("\t" + lenArgs.TAB_WIDTH + " " + cmdlist[e + 1] + "\n")
        elif cmdlist[e] == lenArgs.FILE_COLOR:
            header += ("\t" + lenArgs.FILE_COLOR + " " + cmdlist[e + 1] + "\n")
        elif cmdlist[e] == lenArgs.FILE_COLOR_ALT:
            header += ("\t" + lenArgs.FILE_COLOR_ALT + " " + cmdlist[e + 1] +
                       "\n")
        elif cmdlist[e] == lenArgs.SET_BAD:
            header += ("\t" + lenArgs.SET_BAD + " " + cmdlist[e + 1] + "\n")
        elif cmdlist[e] == lenArgs.SET_GOOD:
            header += ("\t" + lenArgs.SET_GOOD + " " + cmdlist[e + 1] + "\n")
    return header + "\n"

def errStr(errno):
    if errno == 0:
        return "Check passed!"
    elif errno == 1:
        return "Check failed!"
    elif errno == 100:
        return "Cannot combine options that require arguments"
    elif errno == 101:
        return "Unrecognized option"
    elif errno == 102:
        return "Unable to allocate sufficient memory"
    elif errno == 103:
        return "Could not open specified file"
    elif errno == 104:
        return "Missing argument to a flag"
    elif errno == 105:
        return "No options or arguments given to program"
    else:
        return ""

def take_nonempty(strings):
    res = []
    for s in strings:
        if s != "":
            res += [s]
    return res

def runLen(files, args):
    try:
        output = subprocess.check_output([lenArgs.NAME] + take_nonempty(args) +
                                         files, stderr = subprocess.STDOUT)
    except subprocess.CalledProcessError as e:
        # e has the following things:
        # returncode
        # cmd
        # output
        msg = "(" + str(e.returncode) + ") " + errStr(e.returncode)
        return (e.output, msg)
    else:
        # (Output, Message)
        return (output, "")
