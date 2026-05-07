# SecureDrop (C++ TCP encrypted chat + file transfer)

A small Windows-only demo app that runs as a **TCP server** and **TCP client** on `localhost:8080`, with a simple key-exchange and encrypted message framing.

> Note: the crypto in this repository is educational/demo code (fixed keys, no authentication, no IVs/nonces). Do not treat this as production security.

## Project layout

- `main.cpp` — entrypoint (`server` / `client` mode)
- `net/`
  - `server.cpp`, `server.h` — TCP server loop + file receive/send
  - `client.cpp`, `client.h` — TCP client loop + file receive/send
- `crypto/`
  - `AES.*` — AES-like block encryption with padding
  - `DES.*` — DES with padding
  - `RSA.*` — fixed RSA keypair + basic encrypt/decrypt
  - `SHA512.*` — SHA-512 implementation (currently not used by net code)

There is also a duplicate copy of the same source tree under `SecureDrop/`.

## Requirements

- Windows (uses Winsock2)
- A GCC/MinGW toolchain is the easiest path (code includes `bits/stdc++.h`)
- Linker must link Winsock: `-lws2_32`

## Build (MinGW g++)

From the repo root:

```bash
g++ -std=gnu++14 -O2 -Wall -Wextra -I. \
  main.cpp \
  net/client.cpp net/server.cpp \
  crypto/AES.cpp crypto/DES.cpp crypto/RSA.cpp crypto/SHA512.cpp \
  -lws2_32 -o main.exe
```

If you prefer building the duplicate copy, run the same command from `SecureDrop/` and adjust paths accordingly.

## Run (step-by-step)

### 1) Start the server

Open Terminal 1 in the repo root:

```bash
./main.exe server
```

You should see:

- `Listening on port 8080...`
- then `Client connected!` after the client starts

### 2) Start the client

Open Terminal 2 in the repo root:

```bash
./main.exe client
```

### 3) Send messages

In either terminal, type a line and press Enter.

- Client shows responses as `Server: ...`
- Server prints incoming messages as `Client: ...`

Type `exit` to end the session.

## Commands

### Switch encryption mode

From **client** or **server**, type one of:

- `switch aes` — AES only
- `switch des` — DES only
- `switch both` — DES then AES

The side that receives the switch request responds with `MODE_OK`, then both sides update their mode.

### Send a file

From **client**:

- `file C:\path\to\something.bin`

From **server**:

- `file C:\path\to\something.bin`

The sender packages the file as:

- `FILE:<filename>:<raw-bytes>`

The receiver saves it to:

- `SecureDrop_Downloads\<filename>`

## Notes / limitations

- RSA uses **fixed parameters** and is used only to encrypt the session keys.
- AES/DES key generators return fixed example keys.
- Message encryption provides confidentiality only; there is no MAC/signature, so tampering is not detected.
- `SHA512` exists but is not currently wired into the networking protocol.
