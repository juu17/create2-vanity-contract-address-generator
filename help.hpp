#ifndef HPP_HELP
#define HPP_HELP

#include <string>
#include "version.hpp"

const std::string g_strHelp = R"(
usage: ./ERADICATE2 [OPTIONS]

  ERADICATE2 mines a CREATE2 salt that makes a contract deploy to a vanity
  address. A CREATE2 address is computed as:

      keccak256( 0xff ++ deployer(20) ++ salt(32) ++ keccak256(initCode) )[12:]

  You fix the deployer and the init code; ERADICATE2 brute-forces the 32-byte
  salt on the GPU and reports salts whose resulting address scores well.

  Input:
    -A, --address <hex>
        REQUIRED. The CREATE2 deployer (factory) address — the 20-byte, 0x-prefixed
        address that executes the CREATE2. For factory-based deploys this is the
        factory contract, e.g. the canonical deterministic-deployment-proxy
        <factory address>.

    Provide the init code in ONE of three ways (all set the same CREATE2 digest):

    -I, --init-code <hex>
        The RAW init code = the contract's CREATION bytecode, 0x-prefixed hex.
        ERADICATE2 computes keccak256(initCode) itself. Do NOT pass an already
        hashed value here — it would be hashed a second time (the classic mistake).
    -i, --init-code-file <path>
        Read the RAW init code (0x-prefixed hex) from a file. Identical meaning to
        -I; preferred for large bytecode. Surrounding whitespace is trimmed.
    -H, --init-code-hash <hex>
        The init-code HASH supplied DIRECTLY: the 32-byte keccak256(initCode) as
        0x-prefixed hex (exactly 64 hex chars). It is used VERBATIM as the CREATE2
        digest — the internal re-hash is SKIPPED. Use this to keep the argument
        short and to avoid the double-hash trap. Overrides -I / -i if both appear.

    With none of the above, the init code is the empty string.

  Basic modes (how a candidate address is scored):
    --benchmark            No scoring; measure raw throughput only.
    --zeros                Score on '0' nibbles anywhere in the address.
    --zero-bytes           Score on 0x00 bytes anywhere in the address.
    --letters              Score on letter nibbles (a-f) anywhere.
    --numbers              Score on digit nibbles (0-9) anywhere.
    --mirror               Score on the address being a palindrome about its center.
    --leading-doubles      Score on leading repeated hex pairs (00, 11, ... ff).

  Modes that take an argument:
    --leading <single hex>
        Score on the address LEADING with the given hex character (e.g. 0, f).
    --matching <hex pattern>
        Score on the address MATCHING the given hex pattern; use 'X' as a wildcard
        nibble. Example: --matching 123456789XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX pins the
        first 9 nibbles and leaves the rest free. The pattern is at most 40
        characters (20 bytes).

  Advanced modes (use with the Range options below):
    --leading-range        Score on LEADING nibbles whose value is within [min, max].
    --range                Score on nibbles ANYWHERE whose value is within [min, max].

  Range (only for --leading-range / --range):
    -m, --min <0-15>       Range minimum, inclusive (0 = '0' ... 15 = 'f').
    -M, --max <0-15>       Range maximum, inclusive.

  Device control:
    -s, --skip <index>     Skip the OpenCL device at this index. Repeatable.

  Tweaking:
    -w, --work <size>      OpenCL local work size.            [default = 128]
    -W, --work-max <size>  OpenCL maximum (global) work size. [default = --size]
    -S, --size <size>      Salts tried per round.             [default = 16777216]

  Help:
    -h, --help             Show this help and exit.

  Output:
    As the best score improves, one line is printed per improvement:
      Time: <s>  Score: <n>  Salt: 0x<32-byte salt>  Address: 0x<20-byte address>
    A live hash-rate (Speed: <n> H/s, and per GPU) is printed while searching.
    Feed the printed Salt to your CREATE2 deployer to get the shown Address; verify
    independently before committing GPU hours or funds.

  Examples:
    # raw init code inline
    ./ERADICATE2 -A <factory address> -I 0x6080604052... --leading 0

    # raw init code from a file (large bytecode)
    ./ERADICATE2 -A <factory address> -i initcode.hex \
                 --matching 123456789XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX

    # init-code HASH directly (short, no re-hash)
    ./ERADICATE2 -A <factory address> \
                 -H 0xdeef8815788705d3a57fdc3d5736c8988c92b4f6b58469b2d5ba49391995a36b \
                 --matching 123456789XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX

  About:
    ERADICATE2 is a vanity address generator for CREATE2 addresses that
    utilizes computing power from GPUs using OpenCL.

    A fork of https://github.com/johguse/ERADICATE2 (original project by
    Johan Gustafsson).

    Version:         )" ERADICATE2_VERSION R"( (this fork)
    Fork maintainer: Juu17
    Original author: Johan Gustafsson <johan@johgu.se>
)";

#endif /* HPP_HELP */
