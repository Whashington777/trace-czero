# TRACE — Condition Zero

Linux overlay for Counter-Strike: Condition Zero with a Dear ImGui interface and a graphical launcher.

## Features

- Head-hitbox targeting with adjustable FOV and smoothing
- Optional recoil control with strength and starting bullet settings (1–25)
- ESP boxes, names, distance, tracers, team and visibility filters
- Crosshair, FOV circle and watermark
- GUI launcher with a configurable game directory

## Run

Download the release archive, extract the entire folder and open `Start`. Select the Half-Life directory containing `hl_linux` and `czero`, then click **Запустить Condition Zero**. Steam should be running. Choose the map and bots in the game.

| Key | Action |
| --- | --- |
| Insert | Toggle the menu |
| Escape | Close the menu |
| End | Detach the overlay |

Aim and ESP start disabled. Recoil control has its own switch and is disabled by default. The launcher leaves the game's video settings and map selection alone.

## Build

Requires CMake 3.20+, a C++20 compiler with i386 support, SDL2 headers, and 32-bit FreeType, OpenGL and SDL2 libraries.

```sh
PKG_CONFIG_LIBDIR=/usr/lib32/pkgconfig:/usr/lib/pkgconfig:/usr/share/pkgconfig \
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release \
  -DSDL2_RUNTIME=/path/to/32-bit/libSDL2-2.0.so.0
cmake --build build --target tracecs16 trace_launcher -j
```

The SDL2 headers must match the linked SDL2 runtime. On other distributions, adjust the pkg-config paths for their multilib layout.

Outputs:

- `build/trace_launcher`
- `build/libtracecs16.so`
- `build/gui/libportfolio_gui.so`

## Supported build

Native Linux GoldSrc, i386. The adapter checks the engine and client GNU build IDs before attaching:

```
hw.so     2eb3ff7acb804319d453335feed3be5ed21faab3
client.so 3d9e527b99dd5455cb39693752401bedf6790f6b
```

Different binaries require a matching profile. The release needs i386 support, a compatible glibc and a 32-bit OpenGL driver. It does not remove weapon spread.

## Libraries

Dear ImGui 1.93.0, SDL2, FreeType and stb. Third-party license notices are retained with their sources and in the release archive. The interface is based on the supplied ImGui Portfolio #8 project.
