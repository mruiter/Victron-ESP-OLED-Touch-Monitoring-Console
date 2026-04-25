#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BIN_DIR="${ROOT_DIR}/.arduino/bin"
ARDUINO_DATA_DIR="${ROOT_DIR}/.arduino/data"
ARDUINO_DOWNLOADS_DIR="${ROOT_DIR}/.arduino/downloads"
ARDUINO_USER_DIR="${ROOT_DIR}/.arduino/user"
ESP32_INDEX_URL="https://espressif.github.io/arduino-esp32/package_esp32_index.json"
ARDUINO_INDEX_URL="https://downloads.arduino.cc/packages/package_index.tar.bz2"
ARDUINO_LIBRARY_INDEX_URL="https://downloads.arduino.cc/libraries/library_index.tar.bz2"

mkdir -p "${BIN_DIR}" "${ARDUINO_DATA_DIR}" "${ARDUINO_DOWNLOADS_DIR}" "${ARDUINO_USER_DIR}"

if [[ ! -x "${BIN_DIR}/arduino-cli" ]]; then
  echo "Installing arduino-cli into ${BIN_DIR} ..."
  curl -fsSL https://raw.githubusercontent.com/arduino/arduino-cli/master/install.sh | BINDIR="${BIN_DIR}" sh
fi

check_url_reachable() {
  local url="$1"
  if ! curl --head --silent --fail --connect-timeout 8 --max-time 15 "${url}" >/dev/null; then
    echo "ERROR: Cannot reach ${url}"
    echo "This environment appears to be offline or blocked from Arduino package servers."
    echo "Retry when network access is available."
    exit 2
  fi
}

check_index_file() {
  local file_path="$1"
  if [[ ! -s "${file_path}" ]]; then
    echo "ERROR: Missing or empty index file: ${file_path}"
    echo "Arduino index update appears to have failed due network/connectivity restrictions."
    exit 2
  fi
}

check_url_reachable "${ARDUINO_INDEX_URL}"
check_url_reachable "${ARDUINO_LIBRARY_INDEX_URL}"
check_url_reachable "${ESP32_INDEX_URL}"

export ARDUINO_DATA_DIR
export ARDUINO_DOWNLOADS_DIR
export ARDUINO_USER_DIR

"${BIN_DIR}/arduino-cli" config init --overwrite
"${BIN_DIR}/arduino-cli" config add board_manager.additional_urls "${ESP32_INDEX_URL}"
"${BIN_DIR}/arduino-cli" core update-index --additional-urls "${ESP32_INDEX_URL}"
check_index_file "${ARDUINO_DATA_DIR}/package_index.json"
check_index_file "${ARDUINO_DATA_DIR}/package_esp32_index.json"
check_index_file "${ARDUINO_DATA_DIR}/library_index.json"
"${BIN_DIR}/arduino-cli" core install esp32:esp32 --additional-urls "${ESP32_INDEX_URL}"

# Core library dependencies for this sketch.
"${BIN_DIR}/arduino-cli" lib install TFT_eSPI EspMQTTClient ArduinoJson

# LilyGo AMOLED library is not always available via index; install from GitHub.
"${BIN_DIR}/arduino-cli" lib install --git-url https://github.com/Xinyuan-LilyGO/LilyGo-AMOLED-Series.git

if ! "${BIN_DIR}/arduino-cli" core list | awk '$1 == "esp32:esp32" { found=1 } END { exit(found ? 0 : 1) }'; then
  echo "ERROR: esp32:esp32 core was not installed. Check your network and rerun this script."
  exit 1
fi

for required_lib in TFT_eSPI EspMQTTClient ArduinoJson LilyGo; do
  if ! "${BIN_DIR}/arduino-cli" lib list | grep -Fq "${required_lib}"; then
    echo "ERROR: Required library '${required_lib}' was not found after installation."
    exit 1
  fi
done

echo "arduino-cli setup complete."
