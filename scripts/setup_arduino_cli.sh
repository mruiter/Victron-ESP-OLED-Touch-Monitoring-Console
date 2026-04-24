#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BIN_DIR="${ROOT_DIR}/.arduino/bin"
ARDUINO_DATA_DIR="${ROOT_DIR}/.arduino/data"
ARDUINO_DOWNLOADS_DIR="${ROOT_DIR}/.arduino/downloads"
ARDUINO_USER_DIR="${ROOT_DIR}/.arduino/user"
ESP32_INDEX_URL="https://espressif.github.io/arduino-esp32/package_esp32_index.json"

mkdir -p "${BIN_DIR}" "${ARDUINO_DATA_DIR}" "${ARDUINO_DOWNLOADS_DIR}" "${ARDUINO_USER_DIR}"

if [[ ! -x "${BIN_DIR}/arduino-cli" ]]; then
  echo "Installing arduino-cli into ${BIN_DIR} ..."
  curl -fsSL https://raw.githubusercontent.com/arduino/arduino-cli/master/install.sh | BINDIR="${BIN_DIR}" sh
fi

export ARDUINO_DATA_DIR
export ARDUINO_DOWNLOADS_DIR
export ARDUINO_USER_DIR

"${BIN_DIR}/arduino-cli" config init --overwrite
"${BIN_DIR}/arduino-cli" config add board_manager.additional_urls "${ESP32_INDEX_URL}"
"${BIN_DIR}/arduino-cli" core update-index --additional-urls "${ESP32_INDEX_URL}"
"${BIN_DIR}/arduino-cli" core install esp32:esp32 --additional-urls "${ESP32_INDEX_URL}"

# Core library dependencies for this sketch.
"${BIN_DIR}/arduino-cli" lib install TFT_eSPI EspMQTTClient ArduinoJson

# LilyGo AMOLED library is not always available via index; install from GitHub.
"${BIN_DIR}/arduino-cli" lib install --git-url https://github.com/Xinyuan-LilyGO/LilyGo-AMOLED-Series.git

echo "arduino-cli setup complete."
