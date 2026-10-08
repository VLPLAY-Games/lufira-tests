# lufira-tests

![License](https://img.shields.io/badge/license-Apache--2.0-green)
![Status](https://img.shields.io/badge/status-alpha-orange)

**lufira-tests** is a collection of standalone test programs for [LufiraOS](https://github.com/VLPLAY-Games/LufiraOS) — raw x86_64 assembly and freestanding C, each exercising a specific syscall or kernel subsystem (fork/exec, pipes, mmap, argv/envp, pointer-validation edge cases, malloc, string routines, filesystem ops, and more) directly against the real syscall ABI, independent of the shell or any userspace package.

It builds independently of [LufiraOS-Builder](https://github.com/VLPLAY-Games/LufiraOS-Builder): the `.asm` tests need only `nasm`+`ld`, and the `.c` tests link against a sibling [LufiraOS](https://github.com/VLPLAY-Games/LufiraOS) checkout purely as a source of its minimal libc (`crt0.S` + a few `libc/src/*.c`), the same `--lufira-repo` pattern used throughout this project's other repositories.

## Usage

```bash
# Expects a sibling ../LufiraOS checkout by default (override with --lufira-repo).
python3 build.py
```

Every test under `asm/` and `c/` is built into a standalone `.elf`, then the results are packed into a versioned release archive under `dist/`. `LufiraOS-Builder`'s `debug` subcommand stages that payload onto `/tests` on the disk image so the tests can be run from the shell (`run /tests/<name>.elf`) inside a live LufiraOS instance.

```bash
python3 build.py --help
```

## Project Structure

```
lufira-tests/
├── asm/            # raw x86_64 assembly tests (nasm) — no libc dependency
├── c/               # freestanding C tests — link against LufiraOS's minimal libc
├── dist/            # versioned release archives produced by build.py
└── build.py         # build entry point
```

## License

This project is licensed under the Apache License 2.0. See the [LICENSE](LICENSE) file for details.
