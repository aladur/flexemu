import gdb


class st_t_Printer:
  def __init__(self, val):
    self.val = val

  def to_string(self):
    track = int(self.val["trk"])
    sector = int(self.val["sec"])
    return "{0:02X}-{1:02X}".format(track, sector)

def pretty_print_st_t(val):
  if str(val.type) == 'st_t': return st_t_Printer(val)
  return None

gdb.pretty_printers.append(pretty_print_st_t)

