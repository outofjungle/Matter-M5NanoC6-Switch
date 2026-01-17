# ESP-IDF Docker Image for Matter Development
# Minimal image - ESP-Matter is mounted from host

FROM espressif/esp-matter:release-v1.5_idf_v5.4.1

# Install additional dependencies needed for Matter
RUN apt-get update && apt-get install -y \
    git \
    python3-pip \
    python3-venv \
    && rm -rf /var/lib/apt/lists/*

# Install Python packages for pairing code generation (ecdsa already installed)
# Install to ESP-IDF virtual environment so packages are available after activation
RUN export IDF_PATH=/opt/espressif/esp-idf IDF_PATH_FORCE=1 && \
    . ${IDF_PATH}/export.sh && \
    pip install --no-cache-dir qrcode pillow

# Set working directory
WORKDIR /workspace

# Default command
CMD ["/bin/bash"]
