# Create2 Vanity Contract Address Generator

Vanity contract-address generator using CREATE2 — runs on Linux and macOS, GPU-accelerated via OpenCL with CPU fallback.

It brute-forces the 32-byte CREATE2 salt so a contract deploys to an address
matching a chosen pattern.

A CREATE2 address is:

```
keccak256( 0xff ++ deployer(20) ++ salt(32) ++ keccak256(initCode) )[12:]
```

You fix the **deployer** (the factory/contract performing the CREATE2) and the
**init code** (the contract's creation bytecode); ERADICATE2 searches the salt.

## The factory (`<factory address>`)

`<factory address>` — the contract that
executes the `CREATE2` opcode. You pass it to ERADICATE2 with `-A`, and the mined
salt is valid **only** for that exact deployer and the exact init code you hash;
change either and the resulting address changes.

Deploy your own minimal CREATE2 factory and use its deployed address as
`<factory address>`:

```solidity
// SPDX-License-Identifier: MIT
pragma solidity ^0.8.0;

/// Minimal CREATE2 factory. The deployed contract's address is
///   keccak256(0xff ++ address(this) ++ salt ++ keccak256(initCode))[12:]
/// so `address(this)` is the <factory address> you pass to ERADICATE2 (-A),
/// and `initCode` is exactly what you hash for -I / -i / -H.
contract Create2Factory {
    event Deployed(address addr, bytes32 salt);

    function deploy(bytes32 salt, bytes memory initCode)
        external
        returns (address addr)
    {
        assembly {
            addr := create2(0, add(initCode, 0x20), mload(initCode), salt)
            if iszero(addr) { revert(0, 0) }
        }
        emit Deployed(addr, salt);
    }
}
```

Call `deploy(salt, initCode)` with the salt ERADICATE2 printed. Alternatively,
reuse a pre-deployed cross-chain CREATE2 proxy (the deterministic-deployment-proxy,
available at a fixed address on most EVM chains) and pass its address as `-A`
instead of deploying your own.

## Build

```
make
```

Produces `ERADICATE2.x64`. A bare `make` works on both Linux (x86_64) and macOS
(Apple Silicon or Intel): the Makefile links OpenCL as a framework on macOS and
via `-lOpenCL` elsewhere, and selects the right compiler flags per platform.
Override flags with `CXXFLAGS="..."` if you need to.

The OpenCL kernels (`eradicate2.cl`, `keccak.cl`) are loaded at runtime from the
working directory, so kernel tweaks take effect on the next run without a
rebuild — run ERADICATE2 from the directory that contains the `.cl` files.

## Usage

```
./ERADICATE2 -A <factory address> (-I | -i | -H <init code / hash>) <mode> [options]
```

Run `./ERADICATE2 -h` for the same reference on the command line.

## Options

### Input

| Option                | Description                                                                                                                                                                                                 |
| --------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `-A, --address <hex>` | **Required.** The CREATE2 deployer (factory) address — 20-byte, 0x-prefixed. For factory-based deploys this is the factory contract, e.g. the canonical deterministic-deployment-proxy `<factory address>`. |

Provide the init code in **one** of three ways (all set the same CREATE2 digest):

| Option                        | Description                                                                                                                                                                                                                      |
| ----------------------------- | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `-I, --init-code <hex>`       | RAW creation bytecode, 0x-prefixed. ERADICATE2 computes `keccak256(initCode)` itself — do **not** pass an already-hashed value (it would be hashed twice).                                                                       |
| `-i, --init-code-file <path>` | Read the RAW creation bytecode (0x-prefixed hex) from a file. Same meaning as `-I`; preferred for large bytecode. Surrounding whitespace is trimmed.                                                                             |
| `-H, --init-code-hash <hex>`  | The 32-byte `keccak256(initCode)` **directly** (exactly 64 hex chars). Used verbatim as the CREATE2 digest — the internal re-hash is **skipped**. Keeps the argument short and avoids the double-hash trap. Overrides `-I`/`-i`. |

With none of the above, the init code is the empty string.

### Scoring modes

Select exactly one mode; it decides how each candidate address is scored.

**Basic**

| Mode                | Description                                               |
| ------------------- | --------------------------------------------------------- |
| `--benchmark`       | No scoring; measure raw throughput only.                  |
| `--zeros`           | Score on `0` nibbles anywhere in the address.             |
| `--zero-bytes`      | Score on `0x00` bytes anywhere in the address.            |
| `--letters`         | Score on letter nibbles (`a`–`f`) anywhere.               |
| `--numbers`         | Score on digit nibbles (`0`–`9`) anywhere.                |
| `--mirror`          | Score on the address being a palindrome about its center. |
| `--leading-doubles` | Score on leading repeated hex pairs (`00`, `11`, … `ff`). |

**With an argument**

| Mode                       | Description                                                                                                                                                                                                                                |
| -------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| `--leading <single hex>`   | Score on the address **leading** with the given hex character (e.g. `0`, `f`).                                                                                                                                                             |
| `--matching <hex pattern>` | Score on the address **matching** the given hex pattern; use `X` as a wildcard nibble. At most 40 characters (20 bytes). Example: `--matching 123456789XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX` pins the leading nibbles and leaves the rest free. |

**Advanced** (use with the Range options below)

| Mode              | Description                                                       |
| ----------------- | ----------------------------------------------------------------- |
| `--leading-range` | Score on **leading** nibbles whose value is within `[min, max]`.  |
| `--range`         | Score on nibbles **anywhere** whose value is within `[min, max]`. |

### Range (only for `--leading-range` / `--range`)

| Option             | Description                                        |
| ------------------ | -------------------------------------------------- |
| `-m, --min <0-15>` | Range minimum, inclusive (`0` = `0` … `15` = `f`). |
| `-M, --max <0-15>` | Range maximum, inclusive.                          |

### Device control

| Option               | Description                                       |
| -------------------- | ------------------------------------------------- |
| `-s, --skip <index>` | Skip the OpenCL device at this index. Repeatable. |

### Tuning

| Option                  | Description                        | Default    |
| ----------------------- | ---------------------------------- | ---------- |
| `-w, --work <size>`     | OpenCL local work size.            | `128`      |
| `-W, --work-max <size>` | OpenCL maximum (global) work size. | `--size`   |
| `-S, --size <size>`     | Salts tried per round.             | `16777216` |

### Help

| Option       | Description             |
| ------------ | ----------------------- |
| `-h, --help` | Show the help and exit. |

## Examples

```
# raw init code from a file
./ERADICATE2 -A <factory address> -i initcode.hex \
             --matching 123456789XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX

# init-code hash directly (short; no re-hash)
./ERADICATE2 -A <factory address> \
             -H 0x1a36b7d4939195d3a6b58469b2dc92b4fdeb8c5795af88536c8853d57ef88709 \
             --matching 123456789XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
```

Output is one line per improved score:

```
Time: <s>  Score: <n>  Salt: 0x<32-byte salt>  Address: 0x<20-byte address>
```

plus a live hash-rate. Feed the printed `Salt` to your CREATE2 deployer to get
the shown `Address`; always verify the resulting address independently before
committing GPU hours or funds.

## About

Vanity address generator for CREATE2 addresses, using GPU compute via OpenCL.

A fork of [johguse/ERADICATE2](https://github.com/johguse/ERADICATE2) — the original project by Johan Gustafsson.

```
Version:         1.1.4 (this fork)
Fork maintainer: Juu17
Original author: Johan Gustafsson <johan@johgu.se>
```
