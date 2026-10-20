# Changelog

## 1.1.4

### Added
- `-H, --init-code-hash <hex>`: supply the 32-byte `keccak256(init code)` digest
  **directly**. It is used verbatim as the CREATE2 digest, with the internal
  re-hash **skipped** — keeping the argument short (64 hex chars) and avoiding the
  double-hash pitfall of passing an already-hashed value to `-I`/`-i`. Validated to
  be exactly 32 bytes; overrides `-I`/`-i` when both are supplied.
- Version banner: the version is now printed as the first line at startup.
- `version.hpp`: single source of truth for the version string (used by the
  startup banner and the `-h` output).
- `.gitignore` for build outputs (`*.o`, `ERADICATE2.x64`, `.DS_Store`, logs).
- `CHANGELOG.md` (this file).

### Changed
- `-h` help and `README.md` rewritten with a detailed description of every
  parameter, the CREATE2 address formula, the output format, and worked examples
  (including `-H`).

### Fixed
- `-A/--address` is now validated to be exactly 20 bytes. Previously a missing or
  wrong-length address was read out of bounds and silently baked into the CREATE2
  preimage, so every "found" salt was mined against a garbage deployer and would
  not reproduce the advertised address on-chain.
- `--matching` patterns longer than 40 characters (20 bytes) are now rejected, and
  the match-buffer fill is bounded, instead of overflowing the fixed `cl_uchar[20]`
  arrays (a stack buffer overflow).
- A kernel source file (`keccak.cl` / `eradicate2.cl`) that cannot be opened now
  fails with a clear "failed to open file" error instead of an opaque OpenCL build
  failure. (The `.cl` files are read relative to the working directory.)
- Numeric arguments (`-w`/`-W`/`-S`/`-m`/`-M`/`-s`) reject malformed values
  (e.g. `16M`, empty) instead of silently parsing a partial / zero / uninitialized
  result; string arguments are taken verbatim (no whitespace truncation).
- Explicit standard-library includes added so the build does not rely on transitive
  includes: `<algorithm>` in `ModeFactory.cpp` (`std::fill`) and `<cstdint>` in
  `hexadecimal.hpp` (`uint8_t`).

### Notes
- CREATE2 salt generation is unchanged: the per-attempt increments stay within the
  salt words (`h.d[6]`/`h.d[7]`/`h.d[8]`), so the init-code digest is never touched.
