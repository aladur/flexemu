import gdb
import flex

#struct s_dir_entry
#{
#    char    filename[FLEX_BASEFILENAME_LENGTH]; // Name of file
#    char    file_ext[FLEX_FILEEXT_LENGTH]; // Extension of file
#    Byte    file_attr; // File attributes, see flexFileAttributes
#    Byte    hour; // FLEX extension: hour of creation. Default: 0
#    st_t    start; // Track/secor of first sector of the file
#    st_t    end; // Track/sector of last sector of the file
#    Byte    records[2]; // Number of records (= sectors) the file has
#    Byte    sector_map; // Indicates a random access file, see IS_RANDOM_FILE
#    Byte    minute; // FLEX extension: minute of creation. Default: 0
#    Byte    month; // Month when the file was created, range 1 - 12
#    Byte    day; // Day when the file was created, range 1 - 31
#    Byte    year; // Year when the file was created, range 0 - 99
#};

class s_dir_entry_Printer:
  def __init__(self, val):
    self.val = val

  def to_string(self):
    if self.val["filename"][0] == 0:
        return "<empty>"
    if self.val["filename"][0] == -1:
        return "<deleted>"
    random = ""
    name = flex.getCString(self.val["filename"], 8)
    ext = flex.getCString(self.val["file_ext"], 3)
    if len(ext) > 0:
        name = name + "." + ext

    attrString = flex.getAttributes(int(self.val["file_attr"]))
    start = str(self.val["start"])
    end = str(self.val["end"])
    if start == end:
        sectors = start
    else:
        sectors = start + ".." + end
    records = flex.getBigEndianWord(self.val["records"])
    is_random = int(self.val["sector_map"])
    if is_random == 2:
        random = "random"
    hour = int(self.val["hour"])
    minute = int(self.val["minute"])
    month = int(self.val["month"])
    day = int(self.val["day"])
    year = int(self.val["year"])
    date = flex.getDateString(day, month, year)
    time = flex.getTimeString(hour, minute)
    return "file={0:s} sectors={1:s} records={2:d} date={3:s} time={4:s} attr={5:s} {6:s}".format(name, sectors, records, date, time, attrString, random)

def pretty_print_s_dir_entry(val):
  if str(val.type) == 's_dir_entry': return s_dir_entry_Printer(val)
  return None

gdb.pretty_printers.append(pretty_print_s_dir_entry)

