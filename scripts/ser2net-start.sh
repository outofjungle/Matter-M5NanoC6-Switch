#!/bin/bash
# Start ser2net for ESP32-C6 remote serial access

set -e

# Colors for output
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m' # No Color

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
CONFIG_FILE="$PROJECT_DIR/ser2net.yaml"

echo -e "${GREEN}Starting ser2net for ESP32-C6...${NC}"

# Check if ser2net is installed
if ! command -v ser2net &> /dev/null; then
    echo -e "${RED}Error: ser2net is not installed${NC}"
    echo "Install with: brew install ser2net"
    exit 1
fi

# Check if config file exists
if [ ! -f "$CONFIG_FILE" ]; then
    echo -e "${RED}Error: Configuration file not found: $CONFIG_FILE${NC}"
    exit 1
fi

# Find ESP32-C6 device
echo -e "${YELLOW}Looking for ESP32-C6 device...${NC}"
DEVICE=$(ls /dev/cu.usb* 2>/dev/null | head -n 1)

if [ -z "$DEVICE" ]; then
    echo -e "${RED}Error: No USB serial device found${NC}"
    echo "Please connect your ESP32-C6 device"
    echo "Available devices:"
    ls -la /dev/cu.* 2>/dev/null || echo "  None"
    exit 1
fi

echo -e "${GREEN}Found device: $DEVICE${NC}"

# Update configuration file with actual device path
if [[ "$OSTYPE" == "darwin"* ]]; then
    sed -i '' "s|esp32_device:.*|esp32_device: $DEVICE|" "$CONFIG_FILE"
else
    sed -i "s|esp32_device:.*|esp32_device: $DEVICE|" "$CONFIG_FILE"
fi

echo -e "${GREEN}Updated configuration: $CONFIG_FILE${NC}"

# Check if ser2net is already running
if pgrep -f "ser2net.*$CONFIG_FILE" > /dev/null; then
    echo -e "${YELLOW}ser2net is already running. Stopping it first...${NC}"
    pkill -f "ser2net.*$CONFIG_FILE" || true
    sleep 1
fi

# Start ser2net in foreground mode
echo -e "${GREEN}Starting ser2net on port 4000...${NC}"
echo -e "${YELLOW}Press Ctrl+C to stop${NC}"
echo ""
echo "Docker connection string:"
echo -e "${GREEN}  rfc2217://host.docker.internal:4000${NC}"
echo ""

exec ser2net -c "$CONFIG_FILE" -d
