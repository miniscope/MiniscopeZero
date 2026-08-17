# Optics

Optical system of the Miniscope Zero head-mounted assembly: bill of materials,
mechanical drawing, optical design files, and housing/baseplate CAD.

## Contents

```
Optics/
├── optics_bom.csv    Bill of materials for the optical path
├── mechanical/       Dimensioned drawing of the optical assembly (Fusion 360 export)
├── zemax/            Ansys Zemax OpticStudio sequential-mode design files
├── housing/          3D-printed optical housing CAD
└── baseplate/        3D-printed baseplate CAD
```

The housing holds the optical stack; the baseplate is cemented to the skull and
receives it. A set-screw mechanism sets the focal plane relative to the implanted
GRIN lens during baseplating, then locks it for subsequent recordings.

## Bill of materials

`optics_bom.csv` lists every element in the optical path. GitHub renders it as a
table in the web UI. The `ref` column keys each part to the mechanical drawing:

| ref | Element |
| --- | --- |
| `LED` | Excitation LED |
| `F_ex` | Excitation filter |
| `DM` | Dichroic beamsplitter |
| `L1`, `L2` | Objective (L2 is closest to the sample) |
| `L3` | Tube lens |
| `M1` | Fold mirror |
| `F_em` | Emission filter |
| `IS` | Image sensor |

Light path: `LED → F_ex → DM (transmitted) → L1 → L2 → sample`, then
`sample → L2 → L1 → DM (reflected 90°) → L3 → M1 (folded) → F_em → IS`.

Folding the path twice distributes the elements laterally rather than
vertically, keeping the assembly height low.

The `notes` column records where a part is **modified from stock** — the filters,
dichroic, and mirror are all diced down from larger catalog parts. Ordering the
stock part number alone will not give you a part that fits the housing.

Links in the `link` column point to the vendor part page, or to a vendor search
for the stock number where no stable product URL exists.

## Mechanical drawing

`mechanical/drawing_Miniscope_Wireless_emission_v_0_2_10.pdf` — dimensioned
layout of the optical stack, exported from Fusion 360 (design v0.2.10), with
element spacings in mm.

## Optical design

`zemax/` contains the sequential-mode OpticStudio models:

- `Miniscope_Wireless_emission_v_0_2_6_mirror_as_prism_2ndaspheric_14572.zmx`
  — emission path (the fold mirror is modelled as a prism).
- `Miniscope_Wireless_excitation_v_0_2_2.zmx` — excitation path.
- `emission_prescription_data.txt` — prescription dump of the emission model
  (surface data, NA, working F/#, track length), readable without OpticStudio.

`.ZDA` files are the matching OpticStudio session/analysis files.
