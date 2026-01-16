#!/bin/bash
# Monitor ESP32-C6 from Docker using RFC2217 remote serial port

set -e

# Colors
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m'

# RFC2217 connection string for Docker
SERIAL_PORT="rfc2217://host.docker.internal:4000"

echo -e "${GREEN}Starting monitor via RFC2217...${NC}"
echo -e "${YELLOW}Make sure ser2net is running: ./scripts/ser2net-start.sh${NC}"
echo -e "${YELLOW}Press Ctrl+] to exit monitor${NC}"
echo ""

# Check if ser2net is accessible
if ! nc -z host.docker.internal 4000 2>/dev/null; then
    if ! nc -z localhost 4000 2>/dev/null; then
        echo -e "${RED}Error: Cannot connect to ser2net on port 4000${NC}"
        echo "Start ser2net with: ./scripts/ser2net-start.sh"
        exit 1
    fi
fi

# Run idf.py monitor with RFC2217
docker compose run --rm esp-idf \
    idf.py -p "$SERIAL_PORT" monitor "$@"
