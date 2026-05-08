set "SRC=%CD%\..\WLMS-v0_2_1-motherboard.kicad_pcb"
set "DST=%CD%\..\panel\WLMS-v0_2_1-motherboard_panel.kicad_pcb"

python -m kikit.ui panelize ^
  --layout "grid; rows:2; cols:3; space:3mm" ^
  --tabs "fixed; width:3mm; vcount:2; hcount:1" ^
  --cuts "mousebites; drill:0.3mm; spacing:0.6mm; offset:-0.4mm; prolong:0.8mm" ^
  --framing "railstb; width: 5mm; space: 3mm; fillet: 1mm" ^
  --tooling "4hole; hoffset:2.5mm; voffset:2.5mm; size:1.5mm" ^
  --fiducials "4fid; hoffset:5mm; voffset:2.5mm; coppersize:2mm; opening:1mm" ^
  "%SRC%" "%DST%"
