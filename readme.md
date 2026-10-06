# SafeStore documentation

SafeStore is a local command-line backup and restore tool written in C. It saves timestamped copies of files and directories, lists their backup history, and lets you select a snapshot to restore with `fzf`. Backups can optionally be compressed with LZ4, encrypted with OpenSSL, or both.

This guide covers building the program, using its commands, understanding the storage layout, and working with the source code. The current implementation is a small prototype: review the restore behavior and limitations before using it with important data.

## Requirements

Use a Linux or compatible Unix environment with a C compiler and Make. The code uses POSIX APIs and GNU-style utilities; portability to other operating systems has not been established.

- Build tools: `cc` and `make`.
- Basic backups: `date`, `sha1sum`, `cp`, and `mv`.
- Restore: `fzf` and `rm`, plus tools required by the selected snapshot format.
- Archived backups: `tar`.
- Compressed backups: `lz4`.
- Encrypted backups: `openssl` supporting `enc -aes-256-cbc -pbkdf2`.

Runtime utilities must be available on `PATH`. Set `HOME` to your user home directory; SafeStore uses it to locate its backup store. Keep enough free space in the store and `/tmp` for archive creation and decryption.

## Build

Run these commands from the repository root:

```sh
make
./safestore --list
```

The Makefile compiles the eight C source files with `cc -Wall -Wextra -O2` and produces `./safestore`. It does not link external compression or cryptography libraries: those operations invoke command-line utilities.

```sh
make clean
make
```

`make clean` removes the executable and generated object files. There is no install target; run the executable from the repository or place a compiled copy in a directory on your `PATH`.

## Command reference

```text
./safestore -b|--backup <file_or_folder> [-c|--compress] [-e|--encrypt]
./safestore -l|--list
./safestore -r|--restore <file_or_folder>
```

### Back up a file or directory

```sh
./safestore --backup ./notes.txt
./safestore --backup ./documents
./safestore --backup ./documents --compress
./safestore --backup ./documents --encrypt
./safestore --backup ./documents --compress --encrypt
```

The source must exist. SafeStore resolves it to an absolute path, computes a SHA-1 identifier from that path, creates a timestamped snapshot directory, saves the data, and appends a record to the CSV history.

Without options, the source is copied recursively. Compression creates a `.tar.lz4` archive. Encryption alone creates a `.tar.enc` archive; compression plus encryption creates `.tar.lz4.enc`. Each archive contains the source basename rather than its complete original path.

Encrypted backups prompt for a password and confirmation. Passwords must be nonempty and match. Input echo is disabled when standard input is a terminal. Keep the password available for recovery; SafeStore does not store it or provide password recovery.

### List backup history

```sh
./safestore --list
```

Entries are grouped by original absolute path. Each entry shows a timestamp and the path identifier. An empty or missing history file produces `No backups yet.` when the file cannot be opened.

The SHA-1 value identifies the source path. It is not a checksum of the backed-up contents and does not verify data integrity.

### Restore a snapshot

```sh
./safestore --restore /absolute/path/to/notes.txt
```

Use the original absolute path, preferably copied from `--list`. SafeStore lists snapshot timestamps in `fzf`, with newer timestamps first. Select an entry and press Enter; cancel the selector to leave the target untouched. The implementation selects timestamps and does not provide a content preview.

SafeStore detects the selected snapshot's format from its filename and restores to the original location. Encrypted snapshots prompt once for the password. A deleted source can be restored, but the restore argument must resolve to the exact path used at backup time. Missing relative paths containing `.` or `..`, or paths through symlinked parents, may produce a different identifier; use the recorded absolute path.

**Restore replaces the target without a confirmation prompt.** After selection, the implementation executes `rm -rf` on the original path before copying, decrypting, or extracting the snapshot. A wrong password or extraction failure can therefore leave the original data deleted. Save any current data elsewhere and verify the backup and password before restoring. Do not restore filesystem roots or broad system directories.

## Storage layout

SafeStore stores all backups under `$HOME/.local/share/safestore`:

```text
~/.local/share/safestore/
  file.csv
  Backup/
    <sha1_of_absolute_source_path>/
      <YYYYMMDD-HHMMSS>/
        <source_basename>              # plain copy
        <source_basename>.tar.lz4      # compression
        <source_basename>.tar.enc      # encryption
        <source_basename>.tar.lz4.enc  # compression and encryption
```

The tree shows alternative snapshot formats; a normal backup creates one of them. Snapshot names use local time with one-second precision. History records use this CSV header:

```csv
timestamp,path,hash
```

The timestamp has the form `YYYY-MM-DD HH:MM:SS`; the path is quoted and the identifier is 40 hexadecimal characters. The CSV contains plaintext original paths and timestamps even when snapshot contents are encrypted. Listing reads the CSV; restore discovers timestamp directories directly under the path identifier.

Archive creation, restore selection, and decryption use temporary files under `/tmp`. Successful operations remove their temporary files, but interrupted or failed operations can leave files behind. Encryption uses AES-256-CBC with PBKDF2 and a salt through OpenSSL. It does not provide authenticated encryption, and temporary archive contents are unencrypted before encryption or after decryption.

## Source structure

- `src/main.c`: parses backup, list, and restore arguments.
- `src/backup.c`: resolves paths, creates snapshots, and appends CSV records.
- `src/list.c`: reads CSV records and groups history by source path.
- `src/restore.c`: discovers snapshots, invokes `fzf`, detects formats, and restores data.
- `src/hash.c`: invokes `sha1sum` to identify absolute source paths.
- `src/fsutil.c`: creates directories and invokes recursive copying.
- `src/compress.c`: creates and extracts tar archives, optionally using LZ4 pipelines.
- `src/encrypt.c`: reads passwords and invokes OpenSSL encryption or decryption.
- `src/common.h`: shared includes and the store path suffix.
- Other `src/*.h` files declare their corresponding module interfaces.
- `Makefile`: build and cleanup targets.
- `Presentations/`: project presentation decks.

## Limitations

- Backups are full copies or full archives. There is no incremental backup, deduplication, scheduling, remote storage, retention policy, or automatic pruning.
- The store is local. A failure affecting the same disk can affect both originals and backups; maintain an independent copy for recovery.
- Multiple backups of the same path within one second share a snapshot directory. Avoid concurrent backups and repeated backups within the same second.
- External copy, move, archive, extraction, and deletion commands often have unchecked exit statuses. A success message or CSV record alone does not establish that the data was saved or restored correctly.
- Paths are interpolated into shell commands using single quotes. Embedded single quotes are not escaped, and leading hyphens can be interpreted as command options in some operations. Use trusted, ordinary filenames; special characters also expose CSV parser limitations.
- Restore considers at most 1,024 timestamp directories and has no transactional rollback. Plain-copy restore may fail if the destination parent directory is missing.
- Symlinks and filesystem metadata follow `realpath`, `cp -r`, and tar behavior. Do not assume complete preservation of ownership, ACLs, extended attributes, or every special file type.
- There is no automated test target or license file in this repository.

## Troubleshooting and verification

If the build fails, check that a C compiler and Make are installed. If a runtime command is missing, check its availability with `command -v`, for example `command -v fzf` or `command -v lz4`.

`Couldn't find` means the backup source could not be resolved. `No backups found` means no snapshot directory was found for the resolved restore path; compare it with the path in `--list` and confirm the same `HOME` is in use. `Restore cancelled.` means no snapshot was selected; also check that `fzf` is available and usable in the current terminal. `Decryption failed` can indicate a wrong password, a damaged archive, or an OpenSSL failure.

Before relying on the tool, use disposable files to exercise backup and restore. Back up a small file, inspect its stored contents and history, change the file, restore the snapshot, and compare the recovered contents. Repeat for directories and any compression or encryption mode you intend to use. Preserve current data separately before every restore.
