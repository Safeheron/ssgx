# Config Secret Sealing — `GetSecretString`

## Overview

`GetSecretString` lets you keep sensitive string fields (database passwords, API
keys, service credentials) in a TOML configuration file **without leaving them as
long-lived plaintext on disk**. The operator deploys the value once in plaintext;
on first access the framework seals it inside the enclave and rewrites the file to
hold only ciphertext. Every later access decrypts entirely within the enclave.

```cpp
TomlConfig toml;
toml.LoadFile("/etc/myapp/config.toml");

auto pwd = toml.GetSecretString("database", "password");
if (!pwd) {
    Printf("Failed: %s\n", toml.GetLastErrorMsg().c_str());
    return false;
}
// use *pwd inside the enclave
```

## How it works

A field is marked with a dotted-key suffix:

- `password.secret = "plaintext"` — what the operator deploys
- `password.sealed = "ssgxcfg.v1:<base64>"` — what the file holds after first access

On each call, `GetSecretString`:

1. If `<field>.sealed` exists → base64-decode and unseal inside the enclave
   (the field path is bound as AAD) and return the plaintext.
2. Otherwise if `<field>.secret` exists → seal the plaintext in the enclave, update
   the in-memory config so `.secret` becomes `.sealed`, and return the plaintext.
   The rewritten file is persisted by `SaveFile()` (called automatically by the
   destructor, or explicitly if you want to detect write errors).
3. Otherwise → return `std::nullopt` (strict mode: the field is not a secret).

## What you get

- **No long-lived plaintext at rest.** After the first access the config file holds
  only ciphertext; the plaintext only ever appears inside the trusted enclave on
  subsequent runs.
- **Hardware- and enclave-bound encryption.** Sealing uses the enclave's
  MRENCLAVE-bound key and the CPU's hardware key, so the sealed file cannot be
  decrypted by another enclave or on another physical machine.
- **Tamper-evident field binding.** The sealed blob is bound to its field path via
  AAD, so a ciphertext copied from one field cannot be unsealed under a different
  field.
- **Format-preserving rewrite.** Comments and key order in the TOML file are
  preserved across the `.secret` → `.sealed` rewrite.
- **Drop-in usage.** Reading a secret is a single call; no separate key management
  or provisioning step is required for the common case.

## Trade-offs and limitations

These are inherent design properties, not defects — weigh them against your threat
model.

**First-access plaintext exposure (`.secret` path only).** Until a field is sealed,
the file still holds `.secret = "plaintext"`, and the plaintext passes through
untrusted host memory and disk. This window closes after the first access; from then
on the file holds only ciphertext and plaintext appears only inside the enclave. The
table below shows what is covered and the action that closes each gap:

| Exposure (first access) | Implementation | Action to close the gap |
|---|---|---|
| Host process memory | Buffers freed without zeroing | Restart process; reboot to also defeat a root attacker scanning freed RAM |
| Disk, logical layer | Zeroed via `mmap`+`bzero`+`msync` (defeats `strings /dev/vda`) | — |
| Disk, SSD physical NAND | Stale page persists until GC | Put initial config on `tmpfs`, or use full-disk encryption (LUKS) |
| VM snapshot | Not covered | Snapshot only after the post-seal restart |

**No anti-rollback.** The sealed blob is bound to its field path (AAD) but has no
version counter, so an attacker who can write the file and holds an older `.sealed`
value (backup, snapshot, replica) can replay it — the next call returns the old
plaintext. Mitigate with file-integrity monitoring, an out-of-band hash of the
expected blob, or periodic secret rotation.

**Enclave upgrade re-provisioning.** Sealing is MRENCLAVE-bound, so an enclave
upgrade makes existing sealed values undecryptable; re-deploy the plaintext `.secret`
and re-seal on the new measurement.

## When to use it

Use it to harden configuration that would otherwise sit in plaintext on disk —
turning previously-plaintext secrets into encrypted-at-rest values that can tolerate
a single, operator-controlled plaintext window during first-boot sealing, such as
database passwords, API keys, and internal service credentials.

## When not to use it

- **Core cryptographic key material** (private signing keys, root secrets) whose
  exposure would be catastrophic and irreversible. Provision such secrets directly
  into the enclave over a remote-attestation secure channel — never via a plaintext
  config file.
- **Credentials that directly govern asset security** — e.g. exchange or custody API
  keys with withdrawal, trading, or signing permissions — whose leak enables direct
  loss of funds. Treat these like core key material above.
