#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
CLI="${ROOT_DIR}/.arduino/bin/arduino-cli"
SKETCH_DIR="${ROOT_DIR}/ESP32RemoteForVictron_Touch_NL_Dashboard_V4"

if [[ ! -x "${CLI}" ]]; then
  echo "arduino-cli not found. Run scripts/setup_arduino_cli.sh first."
  exit 1
fi

export ARDUINO_DATA_DIR="${ROOT_DIR}/.arduino/data"
export ARDUINO_DOWNLOADS_DIR="${ROOT_DIR}/.arduino/downloads"
export ARDUINO_USER_DIR="${ROOT_DIR}/.arduino/user"

if ! "${CLI}" core list | awk '$1 == "esp32:esp32" { found=1 } END { exit(found ? 0 : 1) }'; then
  echo "esp32:esp32 core not found. Run scripts/setup_arduino_cli.sh first."
  exit 1
fi

"${CLI}" compile \
  --fqbn esp32:esp32:esp32s3 \
  --warnings all \
  "${SKETCH_DIR}"
