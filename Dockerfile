# ESP-IDF Docker Image for Matter Development
# Minimal image - ESP-Matter is mounted from host

FROM espressif/esp-matter:release-v1.5_idf_v5.4.1

# Install additional dependencies needed for Matter
RUN apt-get update && apt-get install -y \
    git \
    python3-pip \
    python3-venv \
    && rm -rf /var/lib/apt/lists/*

# Set working directory
WORKDIR /workspace

# Default command
CMD ["/bin/bash"]
