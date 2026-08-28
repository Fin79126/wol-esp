#!/bin/bash
FEATURE_NAME=$1
if [ -z "$FEATURE_NAME" ]; then
 echo "Error: 機能名を指定してください (例: ./cleanup_worker.sh inventory)"
 exit 1
fi
WORKTREE_DIR=".worktrees/feat-${FEATURE_NAME}"
BRANCH_NAME="feat/${FEATURE_NAME}"

git worktree remove "$WORKTREE_DIR"
git branch -d "$BRANCH_NAME"
echo "Cleaned up worktree and branch for $FEATURE_NAME"
