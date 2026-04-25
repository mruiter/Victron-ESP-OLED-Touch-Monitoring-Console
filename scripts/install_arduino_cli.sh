#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BIN_DIR="${ROOT_DIR}/.arduino/bin"

mkdir -p "${BIN_DIR}"

if [[ -x "${BIN_DIR}/arduino-cli" ]]; then
  echo "arduino-cli already present at ${BIN_DIR}/arduino-cli"
else
  echo "Installing arduino-cli into ${BIN_DIR} ..."
  curl -fsSL https://raw.githubusercontent.com/arduino/arduino-cli/master/install.sh | BINDIR="${BIN_DIR}" sh
fi

"${BIN_DIR}/arduino-cli" version

echo "arduino-cli is installed in ${BIN_DIR}."
