#!/bin/bash
# 使い方: ./setup_worker.sh <機能名/エージェント名>
FEATURE_NAME=$1
WORKTREE_DIR=".worktrees/feat-${FEATURE_NAME}"
BRANCH_NAME="feat/${FEATURE_NAME}"
AGENT_NAME="Agent-${FEATURE_NAME^}"
AGENT_EMAIL="${FEATURE_NAME}@agent.local"

if [ -z "$FEATURE_NAME" ]; then
 echo "Error: 機能名を指定してください (例: ./setup_worker.sh inventory)"
 exit 1
fi

# 1. ワークツリーの作成
git worktree add -b "$BRANCH_NAME" "$WORKTREE_DIR" HEAD

# 2. ワークツリー内固有のGit Authorを設定
cd "$WORKTREE_DIR" || exit
git config user.name "$AGENT_NAME"
git config user.email "$AGENT_EMAIL"
cd ../../

# 3. 作業メモ用ファイルの初期化
mkdir -p docs/agent_logs
touch "docs/agent_logs/${FEATURE_NAME}.md"

echo "=== サブエージェント環境が準備できました ==="
echo "作業パス: $WORKTREE_DIR"
echo "Git Author: $AGENT_NAME <$AGENT_EMAIL>"
echo "作業ログ: docs/agent_logs/${FEATURE_NAME}.md"
