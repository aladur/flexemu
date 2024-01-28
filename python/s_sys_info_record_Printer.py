import gdb
import flex

#struct s_sys_info_record
#{
#    char    disk_name[FLEX_DISKNAME_LENGTH]; // Name of this disk image
#    char    disk_ext[FLEX_DISKEXT_LENGTH]; // Extension of this disk image
#    Byte    disk_number[2]; // Number of this disk image
#    st_t    fc_start; // Start track/sector of free chain
#    st_t    fc_end; // End track/sector of free chain
#    Byte    free[2]; // Number of sectors in free chain
#    Byte    month; // Month when the disk was created, range 1 - 12
#    Byte    day; // Day when the disk was created, range 1 - 31
#    Byte    year; // Year when the disk was created, range 0 - 99
#    st_t    last; // Maximum track/sector number this disk image supports
#};

class s_sys_info_record_Printer:
  def __init__(self, val):
    self.val = val

  def to_string(self):
    name = flex.getCString(self.val["disk_name"], 8)
    ext = flex.getCString(self.val["disk_ext"], 3)
    if len(ext) > 0:
        name = name + "." + ext
    number = flex.getBigEndianWord(self.val["disk_number"])
    fc = str(self.val["fc_start"]) + ".." + str(self.val["fc_end"])
    free = flex.getBigEndianWord(self.val["free"])
    month = int(self.val["month"])
    day = int(self.val["day"])
    year = int(self.val["year"])
    last = str(self.val["last"])
    date = flex.getDateString(day, month, year)
    return "disk={0:s} #{1:d} free_chain={2:s} free={3:d} date={4:s} last={5:s}".format(name, number, fc, free, date, last)

def pretty_print_s_sys_info_record(val):
  if str(val.type) == 's_sys_info_record': return s_sys_info_record_Printer(val)
  return None

gdb.pretty_printers.append(pretty_print_s_sys_info_record)

#def build_pretty_printer():
#    fpp = gdb.printing.RegexpCollectionPrettyPrinter("flexemu_pretty_printer")

#    fpp.add_printer('st_t', '^st_t$', st_t_Printer)
#    fpp.add_printer('s_sys_info_record', '^s_sys_info_record$', s_sys_info_record_Printer)

#gdb.printing.register_pretty_printer(
#    gdb.current_objfile(), build_pretty_printer())

