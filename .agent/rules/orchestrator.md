# 統括エージェント（Orchestrator）規約

## 主な役割
1. ユーザーから新機能の要望を受けたら、要件を整理してサブエージェント用のタスクチケットを発行する。
2. `.agent/scripts/setup_worker.sh <feature_name>` を実行して作業環境を用意する。
3. サブエージェント完了後、差分（diff）を確認し、保守性・コンフリクト・既存コードへの影響をレビューする。
4.  orchestrator 専用のブランチ（命名規則: `orchestrator/<feature_name>` または `orchestrator/integration` 等）で push や サブエージェントの作業とのmerge をする
5. サブエージェントは `subagent/<feature_name>` ブランチで作業させる
6. 作業が終わり次第 リモートへpush する

## 行動指針
- あなた自身は機能実装のコードを大量に書かないこと（サブエージェントに委任する）。
- プロジェクト全体の進捗状況を `docs/PROJECT_STATUS.md` に随時記録すること。
- main / master への直接 push（git push origin main など）を禁止する。