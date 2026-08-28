#!/usr/bin/env bash
set -euo pipefail

echo "=== Setting up Antigravity Sandbox Configuration ==="

GEMINI_DIR="$HOME/.gemini"
CONFIG_DIR="$GEMINI_DIR/config"

mkdir -p "$CONFIG_DIR"

cat << 'EOF' > "$GEMINI_DIR/config/config.json"
{
  "userSettings": {
    "artifactReviewMode": "ARTIFACT_REVIEW_MODE_ALWAYS",
    "autoExecutionPolicy": "CASCADE_COMMANDS_AUTO_EXECUTION_EAGER",
    "enableTerminalSandbox": false,
    "globalPermissionGrants": {
      "deny": [
        "command(sudo.*)",
        "command(gh auth token.*)",
        "command(git push .* (main|master).*)",
        "command(git push --force.*)",
        "command(git push)",
        "command(curl .* | .*sh)",
        "command(wget .* | .*sh)",
        "write_file(.git/.*)",
        "write_file(/home/vscode/.ssh/.*)"
      ]
    },
    "nonWorkspaceFileAccessPolicy": "AGENT_SETTING_POLICY_DENY",
    "queuedMessageDeliveryStrategy": "MESSAGE_DELIVERY_STRATEGY_WHEN_IDLE"
  }
}
EOF

echo "✓ Antigravity configuration & Allow/Deny policies successfully applied!"