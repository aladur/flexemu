import gdb

__all__ = "getMonthString getBigEndianWord getCString".split()

monthStrings = [ "JAN", "FEB", "MAR", "APR", "MAY", "JUN",
                 "JUL", "AUG", "SEP", "OCT", "NOV", "DEC" ]

def getMonthString(monthIndex):
    monthIndex = max(monthIndex, 0)
    monthIndex = min(monthIndex, 11)
    return monthStrings[monthIndex]

def getDateString(day, month, year):
    if year < 75:
        year = year + 2000
    else:
        year = year + 1900
    monthString = getMonthString(month - 1)
    return "{0:02d}-{1:s}-{2:04d}".format(day, monthString, year) 

def getTimeString(hour, minute):
    return "{0:02d}-{1:02d}".format(hour, minute)

def getBigEndianWord(gdbArray):
    return int(gdbArray[0]) * 256 + int(gdbArray[1])

def getCString(gdbArray, max_size):
    string = str("")
    i = 0
    while (i < max_size) & (gdbArray[i] != 0):
        string = string + chr(gdbArray[i] & 127)
        i = i + 1
    return string

def getAttributes(value):
    string = ""
    if (value & 128) != 0:
        string = string + "W"
    if (value & 64) != 0:
        string = string + "D"
    if (value & 32) != 0:
        string = string + "R"
    if (value & 16) != 0:
        string = string + "C"
    return string

def getSectorTypeString(value):
    if value == 0:
        return "Unknown"
    if value == 1:
        return "Boot"
    elif value == 2:
        return "SystemInfo"
    elif value == 3:
        return "Directory"
    elif value == 4:
        return "FreeChain"
    elif value == 5:
        return "File"
    elif value == 6:
        return "NewFile"
    else:
        return "Unknown (" + str(value) + ")"

