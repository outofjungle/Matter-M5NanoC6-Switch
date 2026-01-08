#!/bin/bash
set -e

export ESP_MATTER_PATH="/Users/ven/Workspace/ESP/esp-matter"
export IDF_PATH="/Users/ven/Workspace/ESP/esp-idf"
export _PW_ACTUAL_ENVIRONMENT_ROOT="$ESP_MATTER_PATH/connectedhomeip/connectedhomeip/.environment"

# Add pigweed tools to PATH
export PATH="$_PW_ACTUAL_ENVIRONMENT_ROOT/cipd/packages/pigweed:$PATH"
export PATH="$_PW_ACTUAL_ENVIRONMENT_ROOT/cipd/packages/pigweed/bin:$PATH"

# Source ESP-IDF tools
source "$IDF_PATH/export.sh"

cd /Users/ven/Workspace/Matter-M5NanoC6-Switch
idf.py -D IDF_TARGET=esp32c6 build
