# Gargantua 2.0

Gargantua is a UCI chess engine with **NNUE** (Efficiently Updatable Neural Network) evaluation, derived from [Chess0](https://github.com/mkd/chess0), and inspired by Code Monkey King's [BBC](https://github.com/maksimKorzh/bbc).

**Version 2.0** introduces a major architectural overhaul, significantly improving strength and stability.

## Key Features in v2.0
- **New NNUE Architecture**: Shifted to a robust Stockfish-based probe implementation (`src/stockfish_probe`) for state-of-the-art evaluation.
- **Syzygy Tablebase Support**: precise endgame play with support for 3-4-5-6-7 piece tablebases.
- **Crash Recovery System**: Implemented a dedicated signal handling mechanism to ensure maximum stability even during intensive tablebase probes.
- **Simplified Build**: A clean and standardized build process.

## Files
- [src](src): Source code (C++17).
- [nn-b1a57edbea57.nnue](src/nn-b1a57edbea57.nnue): Primary neural network.
- [tests](tests): Python-based testing framework.

## Building Gargantua
To build the engine, simply run:
```bash
cd src
make
```
This produces the `gargantua` binary.

## Using Gargantua
Gargantua supports the [UCI protocol](http://wbec-ridderkerk.nl/html/UCIProtocol.html). You can use it with any compatible GUI (Scid, CuteChess, Arena, etc.).

### UCI Options
- **Hash**: Size of the transposition table in MB.
- **SyzygyPath**: Absolute path to your Syzygy tablebase files (`.rtbw`, `.rtbz`).
  - Example: `SyzygyPath=/path/to/syzygy/3-4-5`

## Testing Framework
Gargantua includes a powerful testing framework in `tests/manager.py` for automated matches and Elo measurement using `cutechess-cli`.

Example: Run a 100-game match against a previous version:
```bash
python3 tests/manager.py --match --base ./gargantua-v1 --games 100 --tc 10+0.1
```

## Credits
- **Claudio M. Camacho**: Author
- **Stockfish Team**: For the NNUE and Tablebase probe implementation.
- **Daniel Shawul**: For the original NNUE-probe inspiration.
