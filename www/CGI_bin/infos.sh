#!/bin/bash

printf "Content-Type: text/plain\r\n\r\n"
echo ""

# Colors for output
CYAN='\033[0;36m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "================================"
echo -e "   SYSTEM INFORMATION"
echo -e "================================"
echo ""

# Time & Date
echo -e "[TIME & DATE]"
echo "Current Time: $(date '+%H:%M:%S')"
echo "Current Date: $(date '+%A, %B %d, %Y')"
echo "Timezone: $(date '+%Z %z')"
echo ""

# System Information
echo -e "[SYSTEM]"
echo "Hostname: $(hostname)"
echo "Kernel: $(uname -r)"
echo "OS: $(uname -s)"
echo "Architecture: $(uname -m)"
echo "Shell: $SHELL"
echo ""

echo -e "================================"