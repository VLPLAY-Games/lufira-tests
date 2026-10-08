#!/usr/bin/env python3
"""lufira-tests — build script, independent of LufiraOS-Builder. Builds
every test under asm/ and c/ into a standalone .elf, then packs the
results into a versioned release archive under dist/.

C tests link against LufiraOS's libc (crt0.S + a few libc/src/*.c), so
this script takes --lufira-repo (default: sibling checkout) purely as a
source of that libc — same pattern as LufiraOS-Builder's --lufira-repo
for the kernel/bootloader. .asm tests need only nasm+ld, no --lufira-repo.
"""

import argparse
import shutil
import subprocess
import tarfile
from pathlib import Path

VERSION = "1.0.0"

# Mirrors the "Сборка:" header comment in each .asm file — name of the
# produced .elf doesn't always match the source stem (hello_simple.asm ->
# hello.elf, kept as-is to stay distinct from hello.c -> hello_c.elf).
ASM_TESTS = {
    "busy_loop.asm": "busy_loop.elf",
    "cwd_test.asm": "cwd_test.elf",
    "fork_test.asm": "fork_test.elf",
    "hello_simple.asm": "hello.elf",
    "mmap_test.asm": "mmap_test.elf",
    "pipe_test.asm": "pipe_test.elf",
    "ptr_validation_test.asm": "ptr_validation_test.elf",
}

# Same libc objects every userspace program links against (see the
# "Сборка (см. подробный header-комментарий в libc/crt0.S)" comment at the
# top of each .c test).
LIBC_SOURCES = ["crt0.S", "src/string.c", "src/malloc.c", "src/printf.c", "src/stdlib.c"]

C_TESTS = {
    "argv_test.c": "argv_test.elf",
    "hello.c": "hello_c.elf",
    "malloc_test.c": "malloc_test.elf",
    "mkdir_test.c": "mkdir_test.elf",
    "string_test.c": "string_test.elf",
}

CC_FLAGS = [
    "-m64", "-ffreestanding", "-fno-builtin", "-fno-pic", "-fno-pie",
    "-mgeneral-regs-only", "-mno-red-zone", "-nostdlib", "-static",
    "-Wall", "-Wextra",
]


def _run(cmd, **kw) -> None:
    subprocess.run(cmd, check=True, **kw)


def build_asm_tests(repo_root: Path, out_dir: Path) -> list:
    built = []
    for src_name, elf_name in ASM_TESTS.items():
        src = repo_root / "asm" / src_name
        obj = out_dir / (src_name.replace(".asm", ".o"))
        elf = out_dir / elf_name
        _run(["nasm", "-f", "elf64", str(src), "-o", str(obj)])
        _run(["ld", "-m", "elf_x86_64", "-o", str(elf), str(obj)])
        built.append(elf)
    return built


def build_c_tests(repo_root: Path, lufira_repo: Path, out_dir: Path) -> list:
    libc_dir = lufira_repo / "libc"
    include_dir = libc_dir / "include"
    flags = CC_FLAGS + [f"-I{include_dir}"]

    libc_objs = []
    for rel in LIBC_SOURCES:
        src = libc_dir / rel
        obj = out_dir / (Path(rel).stem + ".o")
        _run(["gcc", *flags, "-c", str(src), "-o", str(obj)])
        libc_objs.append(obj)

    built = []
    for src_name, elf_name in C_TESTS.items():
        src = repo_root / "c" / src_name
        obj = out_dir / (src_name.replace(".c", ".o"))
        elf = out_dir / elf_name
        _run(["gcc", *flags, "-c", str(src), "-o", str(obj)])
        _run(["ld", "-m", "elf_x86_64", "-static", "-nostdlib", "-no-pie",
              "-o", str(elf), str(obj), *[str(o) for o in libc_objs]])
        built.append(elf)
    return built


def package(elves: list, dist_dir: Path, version: str) -> Path:
    dist_dir.mkdir(parents=True, exist_ok=True)
    archive_path = dist_dir / f"lufira-tests-v{version}.tar.gz"
    with tarfile.open(archive_path, "w:gz") as tf:
        for elf in elves:
            tf.add(elf, arcname=elf.name)

    # "latest" is what LufiraOS-Builder resolves today (no real release
    # server yet) — swapping in a real GitHub Releases URL later needs no
    # change downstream of this function.
    latest_path = dist_dir / "lufira-tests-latest.tar.gz"
    shutil.copyfile(archive_path, latest_path)
    return archive_path


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--lufira-repo", type=Path,
                         default=Path(__file__).resolve().parent.parent / "LufiraOS",
                         help="path to the LufiraOS repository, needed only for libc sources "
                              "(default: sibling of this repository)")
    parser.add_argument("--out-dir", type=Path, default=Path("build"))
    parser.add_argument("--dist-dir", type=Path, default=Path("dist"))
    parser.add_argument("--version", default=VERSION)
    args = parser.parse_args()

    repo_root = Path(__file__).resolve().parent
    out_dir = args.out_dir
    if out_dir.exists():
        shutil.rmtree(out_dir)
    out_dir.mkdir(parents=True)

    print("=== Building .asm tests (nasm + ld) ===")
    asm_elves = build_asm_tests(repo_root, out_dir)

    print("=== Building .c tests (gcc + LufiraOS libc) ===")
    c_elves = build_c_tests(repo_root, args.lufira_repo, out_dir)

    print(f"=== Packing {len(asm_elves) + len(c_elves)} test(s) into a release archive ===")
    archive_path = package(asm_elves + c_elves, args.dist_dir, args.version)
    print(f"release archive created: {archive_path}")


if __name__ == "__main__":
    main()
