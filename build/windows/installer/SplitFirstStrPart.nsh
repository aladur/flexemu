Function SplitFirstStrPart
  Exch $R0
  Exch
  Exch $R1
  Push $R2
  Push $R3
  StrCpy $R3 $R1
  StrLen $R1 $R0
  IntOp $R1 $R1 + 1
  loop:
    IntOp $R1 $R1 - 1
    StrCpy $R2 $R0 1 -$R1
    StrCmp $R1 0 exit0
    StrCmp $R2 $R3 exit1 loop
  exit0:
  StrCpy $R1 ""
  Goto exit2
  exit1:
    IntOp $R1 $R1 - 1
    StrCmp $R1 0 0 +3
     StrCpy $R2 ""
     Goto +2
    StrCpy $R2 $R0 "" -$R1
    IntOp $R1 $R1 + 1
    StrCpy $R0 $R0 -$R1
    StrCpy $R1 $R2
  exit2:
  Pop $R3
  Pop $R2
  Exch $R1 ;rest
  Exch
  Exch $R0 ;first
FunctionEnd