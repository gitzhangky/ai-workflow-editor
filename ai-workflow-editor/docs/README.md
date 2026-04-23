# AI Workflow Editor Docs Index

## 如果你是使用者

先看：

1. `user-guide.md`
   - 界面说明
   - 第一个工作流怎么搭
   - Inspector 怎么填
   - 校验提示怎么看
   - 保存 / 打开 / Undo / Redo 怎么用
   - 快捷键速查
   - 常见问题

## 如果你是继续开发的编码代理

建议按这个顺序读：

1. `user-guide.md`
   - 当前用户实际能怎么用
   - 当前需要维护的操作流程文档

2. `2026-04-22-claude-code-handoff.md`
   - current status
   - code map
   - build/test commands
   - compatibility notes
   - rules for the next coding agent

3. `plans/2026-04-23-beta-usability-priority-plan.md`
   - 当前推荐的后续开发顺序
   - 可靠性优先的阶段划分

4. `2026-04-21-ai-workflow-editor-design.md`
   - original design intent
   - architecture direction
   - scope boundaries

5. `plans/2026-04-21-ai-workflow-editor-implementation-plan.md`
   - first-wave implementation history
   - useful only when tracing initial scaffolding decisions

6. `plans/2026-04-22-next-development-plan.md`
   - 历史计划
   - 仅在追溯早期开发顺序时参考

## 文档维护约定

从现在开始，任何用户可见的新功能，都应该同步更新：

- `docs/user-guide.md`
- 本页入口说明

如果一个功能改变了用户操作路径，但文档没有更新，这次开发就还不算真正完成。
