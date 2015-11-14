# len
Line length utility

<samp>len</samp> is a small utility that counts line lengths from text files. Its behavior on other file types is undefined and probably not very safe.

By default, <samp>len</samp> does not produce any output on stdout or stderr.

When piping output from <samp>len</samp> to <samp>less</samp>, be sure to specify <samp>-r</samp> with <samp>less</samp> if you want colors to be displayed properly.

Feel free to glance at the spaghetti code that is the source code. A short description of the options that <samp>len</samp> recognizes is near the top of <samp>len.c</samp>.

<hr>
<h4>Compiling</h4>
<samp>make len</samp> should do the trick.
</br>
<samp>len</samp> is written to <samp>-std=c99 -pedantic</samp> under <samp>gcc 4.8.0.</samp>
<hr>

<h4>Return Value</h4>
<samp>len</samp> returns 0 when all lines inspected had lengths within the specified/default range.
<samp>len</samp> returns 1 when at least one line inspected had a length outside the specified/default range.

<hr>
<h4>Other Capabilities</h4>
len is capable of displaying lines from all files it processed and some information about those lines.
<ul>
    <li>Filenames</li>
    <li>Line numbers</li>
    <li>Line lengths</li>
    <li>Lines with lengths outside a certain range</li>
    <li>Lines with lengths within a certain range</li>
    <li>All lines from all files inspected</li>
    <li>Truncated lines</li>
    <li>Which parts of lines are outside of a certain range</li>
</ul>
