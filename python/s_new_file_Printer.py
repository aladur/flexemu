import gdb

#struct s_new_file
#{
#    std::string filename;
#    st_t first; /* track and sector of first first first sector */
#    st_t next; /* track and sector of next sector to be written */
#};

class s_new_file_Printer:
  def __init__(self, val):
    self.val = val

  def to_string(self):
    filename = str(self.val["filename"])
    first = str(self.val["first"])
    next = str(self.val["next"])
    return "filename={0:s} first={1:s} next={2:s}".format(filename, first, next)

def pretty_print_s_new_file(val):
    if str(val.type) == 'NafsDirectoryContainer::s_new_file': return s_new_file_Printer(val)
    return None

gdb.pretty_printers.append(pretty_print_s_new_file)

