# psy

psy is a terminal text editor first written in 1990. Its features may show their
age, but familiar keystrokes and a fondness for the editor have kept it in daily
use across generations of machines and operating systems.

## History

As the author remembers it, psy originally ran on MS-DOS. It was later ported to
BSD and Mac OS X. Before the current work, the last build still in use was an
Intel binary. The source retains creation dates from 1990 and the original
designer name, `Kenji Psytoe`.

In 2026, a macOS upgrade prompted a native arm64 port for Apple Silicon, developed
with Codex. A version preserving the legacy behavior was saved first, followed by
work on longstanding buffer capacity and character encoding limitations. The new
features were developed using test-driven development, including support for
terminal resizing.

The current version includes:

- Dynamic character and line storage, removing the old limit of roughly 64k characters.
- UTF-8 by default, with automatic detection of Shift-JIS input files.
- Editing and saving of characters outside Shift-JIS, including additional kanji and emoji.
- Terminal resizing, including rearrangement of split windows.

The current build and test target is macOS on Apple Silicon. MS-DOS and BSD are
part of the project's history; the current version has not been verified on those
platforms.

## Building and running

```sh
make
PSYHEAD=./ ./psy filename.txt
```

The build uses the macOS Command Line Tools, ncurses, and iconv. On Apple Silicon,
it produces a native arm64 executable. To rebuild from scratch, run
`make clean && make`.

The example above explicitly loads the bundled `custom.psy` configuration.

## Character encodings

- **Reading:** Validate the entire file as UTF-8 first. If it is not valid UTF-8,
  validate and read it as Shift-JIS (CP932).
- **Saving:** Use UTF-8 by default, regardless of the input encoding.
- **Internal representation:** Unicode code points. Terminal input and output also use UTF-8.

Kanji and emoji that cannot be represented in Shift-JIS can be edited and saved.
When inserting a file into the current text, its encoding is detected independently.
As in earlier versions, CRLF and CR line endings are normalized to LF on input,
and files are saved with LF line endings.

Override the defaults at startup when needed:

```sh
./psy --read-encoding=shift-jis old-file.txt
./psy --read-encoding=utf-8 filename.txt
./psy --write-encoding=shift-jis filename.txt
./psy --help
```

Read and write settings are independent. `--read-encoding` also applies to files
read after startup. `--write-encoding` applies to subsequent saves and block
exports.

If a byte sequence is valid in both encodings, UTF-8 takes precedence. Use
`--read-encoding=shift-jis` when automatic detection chooses the wrong encoding.

## Read and save failures

If a read fails because of invalid encoding, embedded NUL bytes, insufficient
memory, or another error, partially read content is not inserted into the buffer.
Saving from a buffer whose file failed to open is disabled. Resolve the problem
and reopen the file before saving.

Saving writes to a temporary file in the same directory. Once conversion and
writing succeed, the original file is moved to its usual `.bak` name and replaced
with the temporary file.

When Shift-JIS output is explicitly requested, saving fails if the text contains
characters that cannot be represented in that encoding. A conversion failure
leaves both the original file and any existing backup unchanged.

## Buffer capacity

Character links use 32-bit indices. Character storage, line storage, and cursor
stacks grow as needed. `buf.size` and `buf.line` specify initial capacities, so
large files can be handled with the original settings of `65535` and `4096`.
The old 16-bit limits on line numbers and column positions, and the fixed stack
limit of 2,048 characters for long lines, have also been removed.

The internal limits are 1,073,741,823 character cells and 1,073,741,823 line cells.
Practical capacity depends on available memory. Storage is shared by all text
buffers and the kill buffer.

The capacity command (`Ctrl-X Ctrl-L Ctrl-P` with the bundled configuration)
displays “自動拡張” (automatic growth). The text size is a count of Unicode code
points in the current text, including newlines. “確保済み空き” (free allocated
storage) reports unused cells in the storage already allocated; it is not a limit
on how much more text can be added. More storage is allocated as needed.

The legacy `msg.info.size` setting is still accepted but is no longer used for
this display. Use `msg.info.capacity` to customize it. Omitting the setting uses
the new default display. Its three `%ld` placeholders are, in order, the text's
character count, free allocated character cells, and free allocated line cells.

Storage for regular expression replacement results also grows as needed.
Search expression input retains its existing limit of 256 bytes.

## Configuration

Configuration is loaded from `PSYHEAD` followed by `custom.psy`. `PSYHEAD` must
include a trailing `/`. If it is unset, psy looks in the current directory.
Existing Shift-JIS configurations are automatically detected and converted to
UTF-8 internally.

The bundled `custom.psy` is a UTF-8 configuration retaining the traditional key
bindings and Japanese messages. To explicitly use it:

```sh
PSYHEAD=./ ./psy filename.txt
```

## Unicode behavior

Cursor movement and deletion operate on code points. Display widths use macOS's
`wcwidth`, including widths for common full-width, half-width, and combining
characters. Treating sequences such as emoji composed of multiple code points as
a single unit for movement or deletion is not yet supported. Their UTF-8 sequences
are preserved.

## Terminal resizing

psy reads the terminal's rows and columns at startup and automatically responds
to later size changes. It redraws even while waiting for input and rearranges
split windows while preserving their relative heights where possible.

Cursor position, search input, save confirmations, and repeat arguments survive
resizing. Filenames and messages are clipped to the terminal width.

If the terminal is narrower than 20 columns or has too few rows for the current
window layout, input pauses until the terminal is enlarged. A single window
requires at least four rows.

## Tests

Running the tests also requires Python 3.

```sh
make test
make test-sanitize
```

`tests/test_editor.py` sends actual keystrokes through a pseudo-terminal and checks
saved files. Coverage includes files exceeding one million characters, 40,000
lines, individual lines of 100,000 characters, Japanese and emoji input/output
and deletion, combining character widths, Japanese search and replacement, large
regular expression replacements, block copying, Shift-JIS detection and UTF-8
saving, insertion of files with different encodings, preservation of originals
on conversion failure, and loading legacy configurations.

Capacity display tests cover legacy configurations, growth beyond the initial
capacity, and custom message templates.

Resize tests change the terminal dimensions and send `SIGWINCH`. They cover
startup dimensions, redraws while idle or entering a search or confirming a save,
cursor preservation after scrolling, split windows, recovery from very small
terminal sizes, and resizing in the middle of UTF-8 input.

`tests/test_memory.c` checks allocation beyond the 16-bit boundary, preservation
of links and stacks during growth, failure at capacity limits, and reuse of freed
cells.

Memory checks use AddressSanitizer and UndefinedBehaviorSanitizer. Leak detection
alone is disabled because some persistent buffers are reclaimed by the operating
system at process exit.

## Historical sources

Two source snapshots document the transition to the current version:

| Snapshot | Contents |
| --- | --- |
| [Before the arm64 port](history/pre-arm64/) | Sources and makefile preserved immediately before the 2026 port, from the working tree used with the Intel executable. |
| [Legacy behavior on arm64](history/legacy-arm64/) | The minimal Apple Silicon port, before the buffer, Unicode, and terminal resizing changes. It retains Shift-JIS and the roughly 64k character limit. |

These are snapshots preserved on October 11, 2026, not an untouched 1990 release
or a reconstruction of the project's full revision history. The older snapshot
already includes changes made during the BSD and Mac OS X years. The original
MS-DOS compiler and linker option files (`psy.cc` and `psy.ld`) and DOS support
source (`psydos.dosc`) also remain at the repository root.

The archived sources and makefiles are byte-for-byte copies, preserving their
original encodings and line endings. `.gitattributes` disables line-ending
conversion under `history/`. Each snapshot has a `SHA256SUMS` manifest; verify it
by running `shasum -a 256 -c SHA256SUMS` inside that directory.

The pre-arm64 snapshot is provided as historical material and is not expected to
build unchanged with current tools. The legacy arm64 snapshot can be built with
`make` in its directory. It requires a legacy Shift-JIS `custom.psy`, selected
through `PSYHEAD` with a trailing `/`; the current UTF-8 configuration is not
intended for that version. Use the repository root for current development.

The original local backups, including the Intel and legacy arm64 executables,
remain in `backup-before-arm64/` and `releases/psy-legacy-arm64/`. Those directories
and personal configuration backups are excluded from Git tracking.
