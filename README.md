# Miniscope Zero

Repository for **Miniscope Zero: a fully wireless, single-cell-resolution miniature microscope for imaging neural dynamics in freely behaving animals**.

Miniscope Zero is a miniature one-photon microscope that operates without tethers: power is delivered via quasistatic cavity resonance (QSCR) wireless power transfer, and imaging data is streamed over a high-bandwidth optical link. This repository hosts the project-specific hardware designs — the miniscope, the optical data acquisition hardware, and the QSCR arena — and points to the related software and hardware repositories.

- Preprint: (TODO: bioRxiv link)
- License: [CERN-OHL-S v2](LICENSE)

Co-led by Takuya Sasatani ([@t-sasatani](https://github.com/t-sasatani)) and Marcel Brosch ([@MarcelMB](https://github.com/MarcelMB)).

## Repository contents

```
MiniscopeZero/                      Wireless miniscope
├── Optics/                         Optical system
│   ├── optics_bom.csv              Bill of materials for the optical path
│   ├── mechanical/                 Dimensioned drawing of the optical assembly
│   ├── zemax/                      Zemax OpticStudio sequential design files
│   ├── housing/                    3D-printed housing: STEP, Fusion archive, per-body STLs
│   └── baseplate/                  CNC-machined baseplate and 3D-printed dust cap
└── PCB/                            Electronics
    ├── WLMS-v0_2_1-motherboard/    Main board: PYTHON480 image sensor, MCU, PMU,
    │                               excitation LED, IR receiver
    ├── WLMS-v0_2_9_daughterboard/  Power/uplink board: rectifier, pre-regulator,
    │                               TX LED for the optical data link
    └── WPT_receiver/               Rectangular wireless power receiver coils

OpticalDAQ/                         Receiver-side data acquisition hardware
├── PCB/
│   ├── optical_daq_v0_4/           Photoreceiver front end: VGA board and digitizer board
│   ├── optical_downlink_v0_2/      USB board for the IR downlink
│   └── FPGA_breakout/              Breakout board for the Opal Kelly XEM7310 FPGA module
└── Housing/                        CNC aluminum shield box

QSCR/                               Quasistatic cavity resonance power-transfer arena
├── COMSOL/                         Electromagnetic simulation models of the arena
├── Fusion360/                      QSCR behavioral arena design files
└── PCB/c_mount/                    Capacitor mount PCB

guides/                             Miscellaneous guides (currently: QSCR operation and assembly)
```

## Related repositories (developed by authors)

- [mio (Miniscope-I/O)](https://github.com/miniscope/mio) — data acquisition, synchronization, and preprocessing
- [CaMAP](https://github.com/miniscope/CaMAP) — calcium imaging mapping and analysis pipeline
- [wpt-devkit](https://github.com/SasataniLab/wpt-devkit) — wireless power transmitter
- [FuzzyTrack](https://github.com/miniscope/FuzzyTrack) — diffused LED position tracking

## Hardware modules

Off-the-shelf or reference modules used in the system:

- [Hamamatsu C15522-3010SA](https://www.hamamatsu.com/us/en/product/optical-sensors/mppc/mppc-module/C15522-3010SA.html) — SiPM (MPPC) module, receiver for the optical data link
- [EPC9065](https://epc-co.com/epc/products/evaluation-boards/epc9065) — high-frequency ZVS class-D power amplifier, drives the QSCR cavity
- [Opal Kelly XEM7310](https://www.opalkelly.com/products/fpga-integration/XEM7310/) — Artix-7 FPGA board for data acquisition

## Contributing

This repository primarily archives the design files accompanying the paper, so
pull requests are unlikely to be merged. Questions and issue reports are
welcome via the issue tracker.