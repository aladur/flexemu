import gdb
import flex

#struct s_sys_info_sector
#{
#    Byte unused1[16]; // To be initialized with 0
#    s_sys_info_record sir; // System information record
#    Byte unused2[216]; // To be initialized with 0
#};

class s_sys_info_sector_Printer:
  def __init__(self, val):
    self.val = val

  def to_string(self):
    return str(self.val["sir"])

def pretty_print_s_sys_info_sector(val):
  if str(val.type) == 's_sys_info_sector': return s_sys_info_sector_Printer(val)
  return None

gdb.pretty_printers.append(pretty_print_s_sys_info_sector)

