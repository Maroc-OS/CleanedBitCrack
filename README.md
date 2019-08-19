# BitCrack  (macOS Compatible)

## Build Status

[![Codacy Badge](https://api.codacy.com/project/badge/Grade/7d7d6b6a74b04571bd855c7786e947b7)](https://www.codacy.com/app/merruk-company/CleanedBitCrack?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=Maroc-OS/CleanedBitCrack&amp;utm_campaign=Badge_Grade)

[![Build status](https://ci.appveyor.com/api/projects/status/r6chsmy618smn3on?svg=true)](https://ci.appveyor.com/project/Maroc-OS/cleanedbitcrack)

## NOTE

This version has been modified and partially fixed by me (Maroc-OS @MerrukTechnolog) except for most of CUDA files it's not tested, only OpenCL has been tested. the main goal is to fix maximum possible issues, style, security and performance. then make it a pool style compatible tool.

## Description

A tool for brute-forcing Bitcoin private keys. The main purpose of this project is to contribute to the effort of solving the [Bitcoin puzzle transaction](https://blockchain.info/tx/08389f34c98c606322740c0be6a7125d9860bb8d5cb182c02f98461e5fa6cd15): A transaction with 32 addresses that become increasingly difficult to crack.

## Using BitCrack

### Usage

Use `cuBitCrack.exe` for CUDA devices and `clBitCrack.exe` for OpenCL devices.

#### Note

**clBitCrack.exe is still EXPERIMENTAL**, as users have reported critial bugs when running on some AMD and Intel devices.

**Note for Intel users:**

There is bug in Intel's OpenCL implementation which affects BitCrack. Details here: <https://github.com/brichard19/BitCrack/issues/123>

```note
xxBitCrack.exe [OPTIONS] [TARGETS]

Where [TARGETS] are one or more Bitcoin address

Options:

-i, --in FILE
    Read addresses from FILE, one address per line. If FILE is "-" then stdin is read

-o, --out FILE
    Append private keys to FILE, one per line

-d, --device N
    Use device with ID equal to N

-b, --blocks BLOCKS
    The number of CUDA blocks

-t, --threads THREADS
    Threads per block

-p, --points NUMBER
    Each thread will process NUMBER keys at a time

-r, --random
    Use random values from keyspace.

--keyspace KEYSPACE
    Specify the range of keys to search, where KEYSPACE is in the format,

    START:END start at key START, end at key END
    START:+COUNT start at key START and end at key START + COUNT
    :END start at key 1 and end at key END
    :+COUNT start at key 1 and end at key 1 + COUNT

-c, --compressed
    Search for compressed keys (default). Can be used with -u to also search uncompressed keys

-u, --uncompressed
    Search for uncompressed keys, can be used with -c to search compressed keys

--compression MODE
    Specify the compression mode, where MODE is 'compressed' or 'uncompressed' or 'both'

--list-devices
    List available devices

--stride NUMBER
    Increment by NUMBER

--share M/N
    Divide the keyspace into N equal sized shares, process the Mth share

--continue FILE
    Save/load progress from FILE
```

### Examples

The simplest usage, the keyspace will begin at 0, and the CUDA parameters will be chosen automatically

```console
xxBitCrack.exe 1FshYsUh3mqgsG29XpZ23eLjWV8Ur3VwH
```

Multiple keys can be searched at once with minimal impact to performance. Provide the keys on the command line, or in a file with one address per line

```console
xxBitCrack.exe 1FshYsUh3mqgsG29XpZ23eLjWV8Ur3VwH 15JhYXn6Mx3oF4Y7PcTAv2wVVAuCFFQNiP 19EEC52krRUK1RkUAEZmQdjTyHT7Gp1TYT
```

To start the search at a specific private key, use the `--keyspace` option:

```console
xxBitCrack.exe --keyspace 766519C977831678F0000000000 1FshYsUh3mqgsG29XpZ23eLjWV8Ur3VwH
```

The `--keyspace` option can also be used to search a specific range:

```console
xxBitCrack.exe --keyspace 80000000:ffffffff 1FshYsUh3mqgsG29XpZ23eLjWV8Ur3VwH
```

To start the search randomly, use the `--random` option:

```console
xxBitCrack.exe --random 1FshYsUh3mqgsG29XpZ23eLjWV8Ur3VwH
```

The `--keyspace` & `--random` option can also be used to search randomly in a specific range:

```console
xxBitCrack.exe --keyspace 80000000:ffffffff --random 1FshYsUh3mqgsG29XpZ23eLjWV8Ur3VwH
```

To periodically save progress, the `--continue` option can be used. This is useful for recovering
after an unexpected interruption:

```console
xxBitCrack.exe --keyspace 80000000:ffffffff 1FshYsUh3mqgsG29XpZ23eLjWV8Ur3VwH
...
GeForce GT 640   224/1024MB | 1 target 10.33 MKey/s (1,244,659,712 total) [00:01:58]
^C
xxBitCrack.exe --keyspace 80000000:ffffffff 1FshYsUh3mqgsG29XpZ23eLjWV8Ur3VwH
...
GeForce GT 640   224/1024MB | 1 target 10.33 MKey/s (1,357,905,920 total) [00:02:12]
```

Use the `-b,` `-t` and `-p` options to specify the number of blocks, threads per block, and keys per thread.

```console
xxBitCrack.exe -b 32 -t 256 -p 16 1FshYsUh3mqgsG29XpZ23eLjWV8Ur3VwH
```

## Choosing the right parameters for your device

GPUs have many cores. Work for the cores is divided into blocks. Each block contains threads.

There are 3 parameters that affect performance: blocks, threads per block, and keys per thread.

`blocks:` Should be a multiple of the number of compute units on the device. The default is 32.

`threads:` The number of threads in a block. This must be a multiple of 32. The default is 256.

`Keys per thread:` The number of keys each thread will process. The performance (keys per second)
increases asymptotically with this value. The default is256. Increasing this value will cause the
kernel to run longer, but more keys will be processed.

### Building

## Build dependencies

Visual Studio 2017 (if on Windows)

For CUDA: CUDA Toolkit 9.2

For OpenCL: An OpenCL SDK

## Building in Windows

Open the Visual Studio solution.

Build the `clKeyFinder` project for an OpenCL build.

Build the `cuKeyFinder` project for a CUDA build.

Note: By default the NVIDIA OpenCL headers are used. You can set the header and library path for
OpenCL in the `BitCrack.props` property sheet.

Note: CUDA may give the build error,

```text
unsupported Microsoft Visual Studio version! Only the versions 2012, 2013, 2015 and 2017 are supported!
```

To fix this, edit `C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v9.2\include\crt\host_config.h` as
Administrator and change

```cpp
#if _MSC_VER < 1600 || _MSC_VER > 1911
```

to

```cpp
#if _MSC_VER < 1600 || _MSC_VER > 1916
```

## Building in macOS & Linux

Using `make`:

Build CUDA:

```console
make BUILD_CUDA=1
```

Build OpenCL:

```console
make BUILD_OPENCL=1
```

Or just use:

```console
make
```

Or build both:

```console
make BUILD_CUDA=1 BUILD_OPENCL=1
```

Debug Version:

```console
make BUILD_DEBUG=1
```

### Supporting this project

If you find this project useful and would like to support it, consider making a donation. Your support is greatly appreciated!

**BTC**: `1LqJ9cHPKxPXDRia4tteTJdLXnisnfHsof`

**LTC**: `LfwqkJY7YDYQWqgR26cg2T1F38YyojD67J`

**ETH**: `0xd28082CD48E1B279425346E8f6C651C45A9023c5`

### Contact

Send any questions or comments to bitcrack.project@gmail.com
