FROM mcr.microsoft.com/devcontainers/base:ubuntu-24.04

# 必要なパッケージのインストール
RUN apt-get update && apt-get install -y --no-install-recommends \
    curl \
    git \
    jq \
    ca-certificates \
    && apt-get clean && rm -rf /var/lib/apt/lists/*

ENV PATH="/home/vscode/.gemini/bin:$PATH"

USER vscode
WORKDIR /workspace