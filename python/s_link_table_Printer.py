import gdb
import flex

#struct s_link_table
#{
#    st_t        next;       // Track and sector number of next sector
#    Byte        record_nr[2]; // FLEX logical record number
#    Word        f_record;   // Relative position in file / 252
#    SDWord      file_id;
#    SectorType  type; // The sector type.
#};

class s_link_table_Printer:
  def __init__(self, val):
    self.val = val

  def to_string(self):
    next = str(self.val["next"])
    record_nr = flex.getBigEndianWord(self.val["record_nr"])
    f_record = int(self.val["f_record"])
    file_id = int(self.val["file_id"])
    if file_id == 2147483647:
        file_id = "max"
    else:
        file_id = str(file_id)
    typeString = flex.getSectorTypeString(self.val["type"])
    return "next={0:s} record_nr={1:d} f_record={2:d} file_id={3:s} type={4:s}".format(next, record_nr, f_record, file_id, typeString)

def pretty_print_s_link_table(val):
    if str(val.type) == 'NafsDirectoryContainer::s_link_table': return s_link_table_Printer(val)
    return None

gdb.pretty_printers.append(pretty_print_s_link_table)

