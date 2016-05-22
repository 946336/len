
"""
Grabbed from http://svn.python.org/projects/python/branches/py3k/Lib/idlelib/WidgetRedirector.py
"""
from Tkinter import *

class WidgetRedirector:

    """Support for redirecting arbitrary widget subcommands.

    Some Tk operations don't normally pass through Tkinter.  For example, if a
    character is inserted into a Text widget by pressing a key, a default Tk
    binding to the widget's 'insert' operation is activated, and the Tk library
    processes the insert without calling back into Tkinter.

    Although a binding to <Key> could be made via Tkinter, what we really want
    to do is to hook the Tk 'insert' operation itself.

    When a widget is instantiated, a Tcl command is created whose name is the
    same as the pathname widget._w.  This command is used to invoke the various
    widget operations, e.g. insert (for a Text widget). We are going to hook
    this command and provide a facility ('register') to intercept the widget
    operation.

    In IDLE, the function being registered provides access to the top of a
    Percolator chain.  At the bottom of the chain is a call to the original
    Tk widget operation.

    """
    def __init__(self, widget):
        self._operations = {}
        self.widget = widget            # widget instance
        self.tk = tk = widget.tk        # widget's root
        w = widget._w                   # widget's (full) Tk pathname
        self.orig = w + "_orig"
        # Rename the Tcl command within Tcl:
        tk.call("rename", w, self.orig)
        # Create a new Tcl command whose name is the widget's pathname, and
        # whose action is to dispatch on the operation passed to the widget:
        tk.createcommand(w, self.dispatch)

    def __repr__(self):
        return "WidgetRedirector(%s<%s>)" % (self.widget.__class__.__name__,
                                             self.widget._w)

    def close(self):
        for operation in list(self._operations):
            self.unregister(operation)
        widget = self.widget; del self.widget
        orig = self.orig; del self.orig
        tk = widget.tk
        w = widget._w
        tk.deletecommand(w)
        # restore the original widget Tcl command:
        tk.call("rename", orig, w)

    def register(self, operation, function):
        self._operations[operation] = function
        setattr(self.widget, operation, function)
        return OriginalCommand(self, operation)

    def unregister(self, operation):
        if operation in self._operations:
            function = self._operations[operation]
            del self._operations[operation]
            if hasattr(self.widget, operation):
                delattr(self.widget, operation)
            return function
        else:
            return None

    def dispatch(self, operation, *args):
        '''Callback from Tcl which runs when the widget is referenced.

        If an operation has been registered in self._operations, apply the
        associated function to the args passed into Tcl. Otherwise, pass the
        operation through to Tk via the original Tcl function.

        Note that if a registered function is called, the operation is not
        passed through to Tk.  Apply the function returned by self.register()
        to *args to accomplish that.  For an example, see ColorDelegator.py.

        '''
        m = self._operations.get(operation)
        try:
            if m:
                return m(*args)
            else:
                return self.tk.call((self.orig, operation) + args)
        except TclError:
            return ""


class OriginalCommand:

    def __init__(self, redir, operation):
        self.redir = redir
        self.operation = operation
        self.tk = redir.tk
        self.orig = redir.orig
        self.tk_call = self.tk.call
        self.orig_and_operation = (self.orig, self.operation)

    def __repr__(self):
        return "OriginalCommand(%r, %r)" % (self.redir, self.operation)

    def __call__(self, *args):
        return self.tk_call(self.orig_and_operation + args)


def main():
    root = Tk()
    root.wm_protocol("WM_DELETE_WINDOW", root.quit)
    text = Text()
    text.pack()
    text.focus_set()
    redir = WidgetRedirector(text)
    global previous_tcl_fcn
    def my_insert(*args):
        print("insert", args)
        previous_tcl_fcn(*args)
    previous_tcl_fcn = redir.register("insert", my_insert)
    root.mainloop()
    redir.unregister("insert")  # runs after first 'close window'
    redir.close()
    root.mainloop()
    root.destroy()

if __name__ == "__main__":
    main()


# author: stefaan.himpe@gmail.com
# license: MIT
from Tkinter import *
import re

class AnsiColorText(Text):
  """
  class to convert text with ansi color codes to 
  text with tkinter color tags
 
  for now we ignore all but the simplest color directives
  see http://www.termsys.demon.co.uk/vtansi.htm for a list of
  other directives
 
  it has not been thoroughly tested, but it works well enough for demonstration purposes
  """
  foreground_colors = {
      'bright' : {
                  '30' : 'Black',
                  '31' : 'Red',
                  '32' : 'Green',
                  '33' : 'Brown',
                  '34' : 'Blue',
                  '35' : 'Purple',
                  '36' : 'Cyan',
                  '37' : 'White'
                  },
      'dim'    :  {
                  '30' : 'DarkGray',
                  '31' : 'LightRed',
                  '32' : 'LightGreen',
                  '33' : 'Yellow',
                  '34' : 'LightBlue',
                  '35' : 'Magenta',
                  '36' : 'Pink',
                  '37' : 'White'
                  }
  }
 
  background_colors= {
      'bright' : {
                  '40' : 'Black',
                  '41' : 'Red',
                  '42' : 'Green',
                  '43' : 'Brown',
                  '44' : 'Blue',
                  '45' : 'Purple',
                  '46' : 'Cyan',
                  '47' : 'White'
                  },
      'dim'    :  {
                  '40' : 'DarkGray',
                  '41' : 'LightRed',
                  '42' : 'LightGreen',
                  '43' : 'Yellow',
                  '44' : 'LightBlue',
                  '45' : 'Magenta',
                  '46' : 'Pink',
                  '47' : 'White'
                  }
  }
 
  # define some regexes which will come in handy in filtering
  # out the ansi color codes
  color_pat = re.compile('\x01?\x1b\[([\d+;]*?)m\x02?')
  inner_color_pat = re.compile("^(\d+;?)+$")
 
  def __init__(self, parent):
    """
    initialize our specialized tkinter Text widget
    """
    Text.__init__(self, parent)
    self.redirector = WidgetRedirector(self)
    self.insert = self.redirector.register("insert", lambda *args, **kw: "break")
    self.delete = self.redirector.register("delete", lambda *args, **kw: "break")
    self.known_tags = set([])
    # register a default color tag
    self.register_tag("30", "Black", "White")
    self.reset_to_default_attribs()
 
  def reset_to_default_attribs(self):
    self.tag = '30'
    self.bright = 'bright'
    self.foregroundcolor = 'Black'
    self.backgroundcolor = 'White'
 
  def colored_write(self, txt, color):
    self.tag = str(30+color)
    txt += '\n'
    self.color_set(self.tag)
    self.insert(END,txt,self.tag)
    
  def register_tag(self, txt, foreground, background):
    """
    register a tag with name txt and with given
    foreground and background color
    """
    self.tag_config(txt, foreground=foreground, background=background)
    self.known_tags.add(txt)
     
  def color_set(self, tag):
    if tag not in self.known_tags:
        # if tag not yet registered, 
        # extract the foreground and background color
        # and ignore the other things
        parts = tag.split(";")
        for part in parts:
          if part in AnsiColorText.foreground_colors[self.bright]:
            self.foregroundcolor = AnsiColorText.foreground_colors[self.bright][part]
          elif part in AnsiColorText.background_colors[self.bright]:
            self.backgroundcolor = AnsiColorText.background_colors[self.bright][part]
          else:
            for ch in part:
              if ch == '0' :
                # reset all attributes
                self.reset_to_default_attribs()
              if ch == '1' :
                # define bright colors
                self.bright = 'bright'
              if ch == '2' :
                # define dim colors
                self.bright = 'dim'

        self.register_tag(tag, 
            foreground=self.foregroundcolor, 
            background=self.backgroundcolor)
    # remember that we switched to this tag
    self.tag = tag
          
  def write(self, text, is_editable=False):
    """
    add text to the text widget
    """
    # first split the text at color codes, stripping stuff like the <ESC> 
    # and \[ characters and keeping only the inner "0;23"-like codes
    segments = AnsiColorText.color_pat.split(text)
    if segments:
      for text in segments:
        # a segment can be regular text, or it can be a color pattern
        if AnsiColorText.inner_color_pat.match(text):
          # if it's a color pattern, check if we already have
          # registered a tag for it
         self.color_set(text)
        elif text == '':
          # reset tag to black
          self.tag = '30' # black
        else:
          # no color pattern, insert text with the currently selected
          # tag
          self.insert(END,text,self.tag)