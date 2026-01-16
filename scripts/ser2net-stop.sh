#!/bin/bash
# Stop ser2net server

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
CONFIG_FILE="$PROJECT_DIR/ser2net.yaml"

echo "Stopping ser2net..."

if pgrep -f "ser2net.*$CONFIG_FILE" > /dev/null; then
    pkill -f "ser2net.*$CONFIG_FILE"
    echo "ser2net stopped"
else
    echo "ser2net is not running"
fi
