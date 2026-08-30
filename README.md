# RetroCycles — Ilonium Client

A client for RetroCycles (0.2.9.3.0), built for competitive play: clearer
feedback, a modern interface and a configuration that works out of the box.

## Download

Grab the latest archive from the [Releases](https://github.com/iloniumm/ilonium-client/releases) page.

## Features

- **Mod menu** — every client feature in one searchable panel, reachable from
  the main menu or in game, with key bindings for the ones worth a shortcut.
- **Interactive minimap** — arena zones (Sumo, Fortress) drawn live, with
  rotation, so the safe ground is always where you expect it.
- **Rubber gauges** — your own reserve as a persistent meter, and per player
  indicators showing what everyone is spending.
- **Zone timers** — countdowns for arena contractions.
- **Spectator camera** — free flight with smooth follow, orbit and a clean
  screen mode for recording.
- **Mazing Training** — the training server's physics as a local solo mode:
  no zones, no round restarts, instant respawn near the centre.
- **Configured on arrival** — bindings, HUD layout, camera and graphics are all
  set up on a fresh install, and anything you change afterwards stays changed.

## Authorship

Copyright (c) 2026 Ilona. Licensed under GPL v2 or later, as the original
Armagetron Advanced from which it derives.

The client identifies itself as `Ilonium Client` for tournament
compliance.

## Installation

1. Download and extract the latest release.
2. Replace `armagetronad_main` in your install (under `usr/bin/` in the Steam
   folder).
3. Keep the `config/` directory from the archive: the shipped configuration
   lives there.

## Building from source

Requires SDL3, OpenGL and a C++11 compiler.

```
./configure
make
```

---

*Built for the RetroCycles community by Ilona.*
