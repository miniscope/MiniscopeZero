# Miniscope Zero

Repository for **Miniscope Zero: a fully wireless, single-cell-resolution miniature microscope for imaging neural dynamics in freely behaving animals**.

Miniscope Zero is a miniature one-photon microscope that operates without tethers: power is delivered via quasistatic cavity resonance (QSCR) wireless power transfer, and imaging data is streamed over a high-bandwidth optical link. This repository hosts the project-specific hardware designs — the miniscope, the optical data acquisition hardware, and the QSCR arena — and points to the related software and hardware repositories.

- Preprint: (TODO: bioRxiv link)
- License: (TODO: decide before release)

Co-led by Takuya Sasatani ([@t-sasatani](https://github.com/t-sasatani)) and Marcel Brosch ([@MarcelMB](https://github.com/MarcelMB)).

## Repository contents

```
MiniscopeZero/                      Wireless miniscope
├── Optics/                         Optical system: 3D-printed housing and BOM (TODO)
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
│   └── FPGA_breakout/              Breakout board for the Opal Kelly XEM7010 FPGA module
└── Housing/                        CNC aluminum shield box

QSCR/                               Quasistatic cavity resonance power-transfer arena
├── COMSOL/                         Electromagnetic simulation models of the arena
├── Fusion360/                      QSCR behavioral arena design files
└── PCB/c_mount/                    Capacitor mount PCB

guides/                             Miscellaneous guides related to the system (TODO)
```

## Related repositories (developed by authors)

- [mio (Miniscope-I/O)](https://github.com/miniscope/mio) — data acquisition, synchronization, and preprocessing
- [CaMAP](https://github.com/miniscope/CaMAP) — calcium imaging mapping and analysis pipeline
- [wpt-devkit](https://github.com/SasataniLab/wpt-devkit) — wireless power transmitter
- [FuzzyTrack](https://github.com/miniscope/FuzzyTrack) — Diffused LED position tracking

## Contributing

This repository primarily archives the design files accompanying the paper, so
pull requests are unlikely to be merged. Questions and issue reports are
welcome via the issue tracker.

## Hardware modules (TODO)

Off-the-shelf modules used in the system (TODO: add part numbers and links):

- Hamamatsu SiPM module
- Wireless power reference design
- Opal Kelly board