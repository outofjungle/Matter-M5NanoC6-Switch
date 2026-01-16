# ESP-IDF Docker Image for Matter Development
# Minimal image - ESP-Matter is mounted from host

FROM espressif/idf:v5.4.1

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
