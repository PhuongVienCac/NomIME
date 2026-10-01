#!/bin/bash
# Sign a NomIME installer for WinSparkle (EdDSA/ed25519 scheme) and print the
# exact appcast.xml enclosure attributes (length + sparkle:edSignature).
#
# Usage: update/sign-update.sh <path-to-installer.exe> [path-to-eddsa_priv.pem]
#
# The private key never lives in this repository; pass its path explicitly
# or keep it at the default location below.

set -euo pipefail

INSTALLER="${1:?Usage: sign-update.sh <installer.exe> [eddsa_priv.pem]}"
# v2 is the key whose public half is compiled into NomIMEServer; the older
# eddsa_priv.pem signs updates that every installed client rejects.
KEY="${2:-$HOME/NomIME-signing-keys/eddsa_priv_v2.pem}"
TOOL="$(dirname "$0")/winsparkle-tool.exe"
SERVER_APP="$(dirname "$0")/../NomIMEServer/NomIMEServerApp.cpp"

if [[ ! -f "$INSTALLER" ]]; then
  echo "error: installer not found: $INSTALLER" >&2
  exit 1
fi
if [[ ! -f "$KEY" ]]; then
  echo "error: EdDSA private key not found: $KEY" >&2
  exit 1
fi

# A signature from the wrong key still "succeeds" here and only fails on the
# users' machines, so refuse any key whose public half the clients don't trust.
TRUSTED=$(sed -n 's/.*kUpdateEdDSAPubKey\[\] = "\([^"]*\)".*/\1/p' "$SERVER_APP")
ACTUAL=$("$TOOL" public-key --private-key-file "$KEY" | grep -oE '[A-Za-z0-9+/]{43}=' | head -n1)
if [[ -z "$TRUSTED" ]]; then
  echo "error: cannot find kUpdateEdDSAPubKey in $SERVER_APP" >&2
  exit 1
fi
if [[ "$ACTUAL" != "$TRUSTED" ]]; then
  echo "error: $KEY does not match the public key clients trust" >&2
  echo "       key's public half: ${ACTUAL:-<unreadable>}" >&2
  echo "       clients trust:     $TRUSTED" >&2
  exit 1
fi

LENGTH=$(stat -c%s "$INSTALLER")
SIGNATURE=$("$TOOL" sign --private-key-file "$KEY" "$INSTALLER")

echo "length=\"$LENGTH\""
echo "sparkle:edSignature=\"$SIGNATURE\""
