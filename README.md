# len
Line length utility

<samp>len</samp> is a small utility that counts line lengths from text files. Its behavior on other file types is undefined and probably not very safe.

<samp>len</samp> returns <samp>0</samp> if all lines inspected had lengths within a user-specifiable range, and <samp>1</samp> if any line had a length outside of that range. len does not otherwise produce output unless asked to.

Feel free to glance at the spaghetti code that is the source code. A short description of the options that <samp>len</samp> recognizes is near the top of <samp>len.c</samp>.

<hr>
<h4>Compiling</h4>
<samp>make len</samp> should do the trick.
</br>
<samp>len</samp> is written to <samp>-std=c99 -pedantic</samp> under <samp>gcc 4.8.0.</samp>

<hr>
<h4>Other Capabilities</h4>
len is capable of displaying lines from all files it processed and some information about those lines.
<ul>
    <li>Filenames</li>
    <li>Line numbers</li>
    <li>Lines with lengths outside a certain range</li>
    <li>Lines with lengths within a certain range</li>
    <li>All lines from all files inspected</li>
    <li>Truncated lines</li>
    <li>Which parts of lines are outside of a certain range</li>
</ul>
