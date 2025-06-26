# PROTO DUMP
Proto dump is a powerful CLI dump tool designed to dump protocol buffers for the android game Swordigo in a simple and readable format.

It is written in raw c++ and provides detailed analysis of the files in a color-coded wire-types,values, offsets and ASCII representations.

## FEATURES

- Parsing support for :
    - `Varint` (000)
    - `len` (010)
    - `I32` (101)
- ASCII and hexadecimal output.
- Offset tracking for easy debugging.
- Save the dumped File.
- Example files.

## Installation

The installation is simple, the tool comes with a `build.bat` for compiling the tool and can easily be customised with added flags.

Install the latest release and your to go!

### Optional post installation guide

1. Goto Advanced System Properties and Click the `Environment Variables` button.
2. Once done, In system variables click the option `Path`, if it doesnt exist you can create it.
3. Then click on `new` and paste the `path` to the dir that the proto.exe file is in, then click `ok` and save the changes.
4. Done now you can easily use Proto Dump without having to be in the same `dir` that the `.exe` is in!

## Usage

proto <filename> `flags`

### Flags

1. ``-o`` : Save the Dump in a file.
USAGE :

```
proto <YourFile> -o OutPutFile
```
Saves the Dump in a file


2. ``-s`` : Stop at a specific offset.
USAGE :

```
proto <YourFile> -s 0xFF
```

Stops at offset `0xFF` AFTER THE END OF DUMPING THE CONTENTS OF VARINTS,LEN,I32.
