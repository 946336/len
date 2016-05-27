#!/usr/bin/env python2

"""
Basic GUI for len
"""

# Help me
from Tkinter import *
import tkFileDialog as fd

import sys
sys.path.append("PATH_TO_EXECUTABLE/len")

import lenfuncs as lf
import ansicolortext as act

from os.path import basename

# ------------------------------------------------------------------------------

# Top-level window
top = Tk()
top.wm_title("Glen")

# ------------------------------------------------------------------------------
# Variables to track "state"

gcolor = StringVar()
bcolor = StringVar()
fcolor_1 = StringVar()
fcolor_2 = StringVar()

matchvar = StringVar()
offendvar = StringVar()
colorvar = StringVar()
linenumvar = StringVar()
linelenvar = StringVar()
truncatevar = StringVar()
newlinevar = StringVar()
invertvar = StringVar()
alternatevar = StringVar()

def_min = StringVar()
def_min.set("0")
def_tab = StringVar()
def_tab.set("8")
def_max = StringVar()
def_max.set("80")

# ------------------------------------------------------------------------------

# Major frames: display, configuration, and file selection
dispFrame = Frame(top)
fileFrame = Frame(top)
confFrame = Frame(top)

# Scrollbar, if possible
scrollbar = Scrollbar(dispFrame)
scrollbar.pack(side = RIGHT, fill = Y)

# Display contents
textbox = act.AnsiColorText(dispFrame)
textbox.configure(yscrollcommand = scrollbar.set)
statusLabel = Label(dispFrame, text = "[Verdict]")

# ???
scrollbar.configure(command = textbox.yview)

# ------------------------------------------------------------------------------

# Set up display
statusLabel.pack()
textbox.pack(fill = BOTH, expand = 1)

# ------------------------------------------------------------------------------
# Set up conf frame for options to len

# Number options - spinboxes
numbers = Frame(confFrame)

minlabel = Label(numbers, text = "Minimum length")
minspinbox = Spinbox(numbers, from_ = 0, to = 9999999999999, width = 5,
                     textvariable = def_min)

tablabel = Label(numbers, text = "Tab width")
tabspinbox = Spinbox(numbers, from_ = 0, to = 9999999999999, width = 5,
                    textvariable = def_tab)

maxlabel = Label(numbers, text = "Maximum length")
maxspinbox = Spinbox(numbers, from_ = 0, to = 9999999999999, width = 5,
                     textvariable = def_max)

# Pack numbers
minlabel.pack(side = LEFT)
minspinbox.pack(side = LEFT)

tablabel.pack(side = LEFT)
tabspinbox.pack(side = LEFT)

maxlabel.pack(side = LEFT)
maxspinbox.pack(side = LEFT)
numbers.pack()

# ------------------------------------------------------------------------------
# Yes/No options
yesno1 = Frame(confFrame)
yesno2 = Frame(confFrame)

# print matches/offenders, color, line numbers, line lengths, truncate,
# count newlines, invert colors, alternate colors
check_matches = Checkbutton(yesno1, onvalue = lf.lenArgs.PRINT_MATCHES,
                               offvalue = "",  text = "Print Matches",
                               variable = matchvar)
check_offenders = Checkbutton(yesno1, onvalue = lf.lenArgs.PRINT_OFFENDERS,
                                 offvalue = "", text = "Print Offenders",
                                 variable = offendvar)

check_color = Checkbutton(yesno1, onvalue = lf.lenArgs.COLOR,
                             offvalue = "", text = "Color", variable = colorvar)
check_linenum = Checkbutton(yesno1, onvalue = lf.lenArgs.LINE_NUMBERS,
                               offvalue = "",  text = "Line Numbers",
                               variable = linenumvar)

check_linelen = Checkbutton(yesno1, onvalue = lf.lenArgs.LINE_LENGTHS,
                               offvalue = "",  text = "Line Lengths",
                               variable = linelenvar)
check_truncate = Checkbutton(yesno2, onvalue = lf.lenArgs.TRUNCATE,
                                offvalue = "", text = "Truncate",
                                variable = truncatevar)
check_newline = Checkbutton(yesno2, onvalue = lf.lenArgs.COUNT_NEWLINES,
                               offvalue = "", text = "Count Newlines",
                               variable = newlinevar)
check_invert = Checkbutton(yesno2, onvalue = lf.lenArgs.INVERT_COLOR,
                              offvalue = "", text = "Invert Colors",
                              variable = invertvar)
check_alternate = Checkbutton(yesno2, onvalue = lf.lenArgs.ALTERNATE_COLORS,
                                 offvalue = "", text = "Alternate Colors",
                                 variable = alternatevar)

# Pack checkboxes
check_matches.pack(side = LEFT)
check_offenders.pack(side = LEFT)
check_color.pack(side = LEFT)
check_linenum.pack(side = LEFT)
check_linelen.pack(side = LEFT)
check_truncate.pack(side = LEFT)
check_newline.pack(side = LEFT)
check_invert.pack(side = LEFT)
check_alternate.pack(side = LEFT)

# Pack yes/no options
yesno1.pack()
yesno2.pack()

# ------------------------------------------------------------------------------
# Global data. I should probably collect these into some class in order to not
# pollute the namespace, but meh.
files = []
output = ""
msg = ""
cmd = []

def runFiles():
    global output
    global msg
    global cmd

    if not files:
        textbox.write("No previous run!")
        return

    top.wm_title("Glen - " + str(map(basename, files)))
    cmd = [matchvar.get(), offendvar.get(), colorvar.get(), linenumvar.get(),
           truncatevar.get(), newlinevar.get(), invertvar.get(),
           alternatevar.get(), lf.lenArgs.MAX, maxspinbox.get(),
           lf.lenArgs.MIN, minspinbox.get(), lf.lenArgs.TAB_WIDTH,
           tabspinbox.get(), linelenvar.get(), lf.lenArgs.SET_BAD, bcolor.get(),
           lf.lenArgs.SET_GOOD, gcolor.get(), lf.lenArgs.FILE_COLOR,
           fcolor_1.get(), lf.lenArgs.FILE_COLOR_ALT, fcolor_2.get()]
    output, msg = lf.runLen(list(files), cmd)
    textbox.delete("1.0", END)    
    textbox.write(output)
    if msg == "":
        statusLabel.configure(text = lf.errStr(0))
    else:
        statusLabel.configure(text = msg)

def selectThenDo():
    global files
    files = fd.askopenfilenames(title = "Select file(s)")
    textbox.delete("1.0", END)
    textbox.write("Checking file(s)...")
    textbox.delete("1.0", END)    
    if files:
        runFiles()

def clear():
    textbox.delete("1.0", END)
    top.wm_title("Glen")
    statusLabel.configure(text = "[Verdict]")

def quickSave():
    outfile = open("-".join(map(basename, files)) + ".len_report", "w")
    outfile.write(output)

def quickSaveWithHeader():
    outfile = open("-".join(map(basename, files)) + ".len_report", "w")
    outfile.write(lf.header(cmd))
    outfile.write(output)

def save():
    outfile = fd.asksaveasfile(mode = "w",
                               initialfile = "-".join(map(basename, files)) +
                                             ".len_report")
    outfile.write(output)

def saveWithHeader():
    outfile = fd.asksaveasfile(mode = "w",
                               initialfile = "-".join(map(basename, files)) +
                                             ".len_report")
    outfile.write(lf.header(cmd))
    outfile.write(output)

# ------------------------------------------------------------------------------
# Select and run files
selectFileButton = Button(fileFrame, text = "Select file(s)",
                             command = selectThenDo)
selectFileButton.pack(side = LEFT)

# Rerun files from previous invocation
rerunButton = Button(fileFrame, text = "Rerun", command = runFiles)
rerunButton.pack(side = LEFT)

# Button to clear text
Button(fileFrame, text = "Clear",
          command = clear, anchor = SE).pack(side = RIGHT)

# ------------------------------------------------------------------------------

dispFrame.pack(fill = BOTH, expand = 1)
fileFrame.pack(fill = X)
confFrame.pack(fill = X)

# ------------------------------------------------------------------------------
# Menus - wheeeeeeee
menubar = Menu()

# File menu. Provides the following:
#   Select File(s)
#   Quicksave
#   Quicksave with header
#   Save output
#   Save output with header
#   Quit
filemenu = Menu(menubar, tearoff = 1)
filemenu.add_command(label = "Select File(s)", command = selectThenDo)
filemenu.add_command(label = "Quick Save", command = quickSave)
filemenu.add_command(label = "Quick Save (With Header)",
                     command = quickSaveWithHeader)
filemenu.add_command(label = "Save output", command = save)
filemenu.add_command(label = "Save output (With Header)",
                     command = saveWithHeader)
filemenu.add_command(label = "Quit", command = top.quit)
menubar.add_cascade(label = "File", menu = filemenu)

# ------------------------------------------------------------------------------
# Color menus

# Groups color options
#   Good
#   Bad
#   File Primary
#   File Secondary
colormenu = Menu(menubar, tearoff = 1)

# Good color
gcolormenu = Menu(colormenu, tearoff = 1)
gcolormenu.add_radiobutton(label = "Red", variable = gcolor, value = "red")
gcolormenu.add_radiobutton(label = "Green", variable = gcolor, value = "green")
gcolormenu.add_radiobutton(label = "Yellow", variable = gcolor,
                           value = "yellow")
gcolormenu.add_radiobutton(label = "Blue", variable = gcolor, value = "blue")
gcolormenu.add_radiobutton(label = "Magenta", variable = gcolor,
                           value = "magenta")
gcolormenu.add_radiobutton(label = "Cyan", variable = gcolor, value = "cyan")
colormenu.add_cascade(label = "Color (Bad)", menu = gcolormenu)

# Bad color
bcolormenu = Menu(colormenu, tearoff = 1)
bcolormenu.add_radiobutton(label = "Red", variable = bcolor, value = "red")
bcolormenu.add_radiobutton(label = "Green", variable = bcolor, value = "green")
bcolormenu.add_radiobutton(label = "Yellow", variable = bcolor,
                           value = "yellow")
bcolormenu.add_radiobutton(label = "Blue", variable = bcolor, value = "blue")
bcolormenu.add_radiobutton(label = "Magenta", variable = bcolor,
                           value = "magenta")
bcolormenu.add_radiobutton(label = "Cyan", variable = bcolor, value = "cyan")
colormenu.add_cascade(label = "Color (Good)", menu = bcolormenu)

# File color 1
fcolormenu_1 = Menu(colormenu, tearoff = 1)
fcolormenu_1.add_radiobutton(label = "Red", variable = fcolor_1, value = "red")
fcolormenu_1.add_radiobutton(label = "Green", variable = fcolor_1,
                             value = "green")
fcolormenu_1.add_radiobutton(label = "Yellow", variable = fcolor_1,
                             value = "yellow")
fcolormenu_1.add_radiobutton(label = "Blue", variable = fcolor_1,
                             value = "blue")
fcolormenu_1.add_radiobutton(label = "Magenta", variable = fcolor_1,
                           value = "magenta")
fcolormenu_1.add_radiobutton(label = "Cyan", variable = fcolor_1,
                             value = "cyan")
colormenu.add_cascade(label = "File Color (Primary)", menu = fcolormenu_1)

# File color 2
fcolormenu_2 = Menu(colormenu, tearoff = 1)
fcolormenu_2.add_radiobutton(label = "Red", variable = fcolor_2, value = "red")
fcolormenu_2.add_radiobutton(label = "Green", variable = fcolor_2,
                             value = "green")
fcolormenu_2.add_radiobutton(label = "Yellow", variable = fcolor_2,
                             value = "yellow")
fcolormenu_2.add_radiobutton(label = "Blue", variable = fcolor_2,
                             value = "blue")
fcolormenu_2.add_radiobutton(label = "Magenta", variable = fcolor_2,
                           value = "magenta")
fcolormenu_2.add_radiobutton(label = "Cyan", variable = fcolor_2,
                             value = "cyan")
colormenu.add_cascade(label = "File Color (Secondary)", menu = fcolormenu_2)

menubar.add_cascade(label = "Colors", menu = colormenu)

# ------------------------------------------------------------------------------
# Argument processing. Help. I don't understand argparser

for i in range(1, len(sys.argv)):
    if (sys.argv[i] == lf.lenArgs.PRINT_OFFENDERS or
            sys.argv[i] == lf.lenArgs.PRINT_OFFENDERS_SHORT):
        offendvar.set(lf.lenArgs.PRINT_OFFENDERS)
    elif (sys.argv[i] == lf.lenArgs.PRINT_MATCHES or
            sys.argv[i] == lf.lenArgs.PRINT_MATCHES_SHORT):
        matchvar.set(lf.lenArgs.PRINT_MATCHES)
    elif (sys.argv[i] == lf.lenArgs.LINE_NUMBERS or
            sys.argv[i] == lf.lenArgs.LINE_NUMBERS_SHORT):
        linenumvar.set(lf.lenArgs.LINE_NUMBERS)
    elif (sys.argv[i] == lf.lenArgs.COLOR or
            sys.argv[i] == lf.lenArgs.COLOR_SHORT):
        colorvar.set(lf.lenArgs.COLOR)
    elif (sys.argv[i] == lf.lenArgs.TRUNCATE or
            sys.argv[i] == lf.lenArgs.TRUNCATE_SHORT):
        truncatevar.set(lf.lenArgs.TRUNCATE)
    elif (sys.argv[i] == lf.lenArgs.LINE_LENGTHS or
            sys.argv[i] == lf.lenArgs.LINE_LENGTHS_SHORT):
        linelenvar.set(lf.lenArgs.LINE_LENGTHS)
    elif (sys.argv[i] == lf.lenArgs.COUNT_NEWLINES or
            sys.argv[i] == lf.lenArgs.COUNT_NEWLINES_SHORT):
        newlinevar.set(lf.lenArgs.COUNT_NEWLINES)
    elif (sys.argv[i] == lf.lenArgs.INVERT_COLOR or
            sys.argv[i] == lf.lenArgs.INVERT_COLOR_SHORT):
        invertvar.set(lf.lenArgs.INVERT_COLOR)
    elif (sys.argv[i] == lf.lenArgs.ALTERNATE_COLORS or
            sys.argv[i] == lf.lenArgs.ALTERNATE_COLORS_SHORT):
        alternatevar.set(lf.lenArgs.ALTERNATE_COLORS)
    elif (sys.argv[i] == lf.lenArgs.MAX or
            sys.argv[i] == lf.lenArgs.MAX_SHORT):
        def_max.set(sys.argv[i + 1])
    elif (sys.argv[i] == lf.lenArgs.MIN or
            sys.argv[i] == lf.lenArgs.MIN_SHORT):
        def_min.set(sys.argv[i + 1])
    elif (sys.argv[i] == lf.lenArgs.TAB_WIDTH or
            sys.argv[i] == lf.lenArgs.TAB_WIDTH_SHORT):
        def_tab.set(sys.argv[i + 1])
    elif sys.argv[i] == lf.lenArgs.FILE_COLOR:
        fcolor_1.set(sys.argv[i + 1])
    elif sys.argv[i] == lf.lenArgs.FILE_COLOR_ALT:
        fcolor_2.set(sys.argv[i + 1])
    elif sys.argv[i] == lf.lenArgs.SET_BAD:
        bcolor.set(sys.argv[1])
    elif sys.argv[i] == lf.lenArgs.SET_GOOD:
        gcolor.set(sys.argv[1])
    else:
        try:
            int(sys.argv[i])
        except ValueError, e:
            # Done with args. This and rest are files
            files = list(sys.argv[i:])
            runFiles()
            break
        else:
            # No exception, this was a number
            # If this does not follow an option, then we're done too
            if not sys.argv[i - 1].startswith("-"):
                files = list(sys.argv[i:])
                runFiles()
                break

# ------------------------------------------------------------------------------

# Set --print-offenders, since ostensibly this is the preferred default option
check_offenders.select()

# Set default colors
bcolor.set("red")
gcolor.set("green")
fcolor_1.set("magenta")
fcolor_2.set("cyan")

top.config(menu = menubar)
top.mainloop()
