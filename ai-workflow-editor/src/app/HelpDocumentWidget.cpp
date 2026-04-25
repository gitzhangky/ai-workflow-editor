#include "app/HelpDocumentWidget.hpp"

#include <QImage>
#include <QPainter>
#include <QPainterPath>
#include <QTextBrowser>
#include <QTextDocument>
#include <QUrl>
#include <QVBoxLayout>

namespace
{
QString helpCss()
{
    return QStringLiteral(
        "body { color: #3d3529; background: #faf6ef; padding: 26px 42px; line-height: 1.72; }"
        "h1 { font-size: 24px; color: #4c433b; border-bottom: 2px solid #d8cfbf; padding-bottom: 10px; margin-top: 0; }"
        "h2 { font-size: 18px; color: #5f5142; margin-top: 30px; border-bottom: 1px solid #e8e0d4; padding-bottom: 5px; }"
        "h3 { font-size: 14px; color: #715f4f; margin-top: 20px; }"
        "p, li { font-size: 13px; }"
        "code { background: #f0e8dc; padding: 2px 5px; border-radius: 3px; font-size: 12px; }"
        "table { border-collapse: collapse; width: 100%; margin: 12px 0; }"
        "th { background: #f0e8dc; color: #5b4f3e; text-align: left; padding: 8px 12px; font-size: 13px; }"
        "td { border-bottom: 1px solid #e8e0d4; padding: 7px 12px; font-size: 13px; vertical-align: top; }"
        "tr:hover td { background: #f5efe5; }"
        "kbd { background: #e8e0d4; border: 1px solid #d0c8b8; border-radius: 3px; padding: 1px 6px; font-size: 12px; }"
        ".hero { background: #fffaf1; border: 1px solid #e1d2bd; border-radius: 14px; padding: 16px 18px; margin: 14px 0 18px; }"
        ".tip { background: #eef6e8; border-left: 3px solid #8cb878; padding: 10px 14px; margin: 12px 0; border-radius: 0 4px 4px 0; font-size: 13px; }"
        ".warning { background: #fff2df; border-left: 3px solid #d69545; padding: 10px 14px; margin: 12px 0; border-radius: 0 4px 4px 0; font-size: 13px; }"
        ".help-figure { background: #fffdf9; border: 1px solid #ddd4c6; border-radius: 14px; padding: 14px; margin: 12px 0 18px; }"
        ".caption { color: #7b6b5a; font-size: 12px; margin-top: 8px; }"
        ".layout-diagram { display: grid; grid-template-columns: 1fr 2fr 1fr; gap: 8px; }"
        ".diagram-box { background: #f2eadf; border: 1px solid #d8cfbf; border-radius: 9px; padding: 10px; text-align: center; font-weight: 700; color: #5a4d40; }"
        ".diagram-wide { grid-column: 1 / span 3; background: #edf3fb; border-color: #b8cbe8; }"
        ".workflow-diagram { white-space: nowrap; }"
        ".flow-node { display: inline-block; background: #f7f1e7; border: 1px solid #d8c7ac; border-radius: 10px; padding: 8px 12px; margin: 4px 2px; font-weight: 700; }"
        ".flow-arrow { color: #4b84d9; font-weight: 700; margin: 0 4px; }"
        ".node-card-demo { width: 260px; border: 1px solid #d8c7ac; border-radius: 12px; background: #fffaf1; overflow: hidden; }"
        ".node-card-head { background: #e9dcc8; padding: 8px 12px; font-weight: 700; }"
        ".node-card-body { padding: 10px 12px; font-size: 12px; }"
        ".badge { display: inline-block; border-radius: 12px; padding: 3px 9px; margin: 3px; font-size: 12px; font-weight: 700; }"
        ".ok { background: #e5f3dd; color: #4f7a35; }"
        ".warn { background: #fff1d6; color: #9b6a20; }"
        ".err { background: #ffe2dd; color: #9c3b2d; }");
}

QString wrap(QString const &body)
{
    return QStringLiteral("<html><head><style>%1</style></head><body>%2</body></html>").arg(helpCss(), body);
}

void drawRoundedBox(QPainter &painter,
                    QRectF const &rect,
                    QString const &text,
                    QColor const &fill,
                    QColor const &stroke,
                    QColor const &textColor = QColor(QStringLiteral("#4c433b")))
{
    QPainterPath path;
    path.addRoundedRect(rect, 12.0, 12.0);
    painter.fillPath(path, fill);
    painter.setPen(QPen(stroke, 2.0));
    painter.drawPath(path);
    painter.setPen(textColor);
    painter.drawText(rect.adjusted(10, 8, -10, -8), Qt::AlignCenter | Qt::TextWordWrap, text);
}

QImage makeLayoutOverviewImage(bool chinese)
{
    QImage image(860, 430, QImage::Format_ARGB32_Premultiplied);
    image.fill(QColor(QStringLiteral("#fffdf9")));

    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing);

    QFont titleFont = painter.font();
    titleFont.setPointSize(17);
    titleFont.setBold(true);
    painter.setFont(titleFont);
    painter.setPen(QColor(QStringLiteral("#4c433b")));
    painter.drawText(QRectF(24, 18, 812, 36),
                     Qt::AlignLeft | Qt::AlignVCenter,
                     chinese ? QString::fromUtf8("界面布局图") : QStringLiteral("Interface Layout"));

    QFont bodyFont = painter.font();
    bodyFont.setPointSize(12);
    bodyFont.setBold(true);
    painter.setFont(bodyFont);

    drawRoundedBox(painter,
                   QRectF(30, 70, 800, 54),
                   chinese ? QString::fromUtf8("顶部工具栏和菜单\n新建 / 保存 / 撤销 / 导出 / 帮助")
                           : QStringLiteral("Toolbar and menus\nNew / Save / Undo / Export / Help"),
                   QColor(QStringLiteral("#edf3fb")),
                   QColor(QStringLiteral("#b8cbe8")));
    drawRoundedBox(painter,
                   QRectF(30, 146, 190, 160),
                   chinese ? QString::fromUtf8("左侧节点库\n搜索、分组、拖拽节点")
                           : QStringLiteral("Node Library\nSearch, categories,\ndrag nodes"),
                   QColor(QStringLiteral("#f2eadf")),
                   QColor(QStringLiteral("#d8cfbf")));
    drawRoundedBox(painter,
                   QRectF(244, 146, 372, 160),
                   chinese ? QString::fromUtf8("中间工作流画布\n摆放节点并连接端口\n帮助页也在这里打开为新页签")
                           : QStringLiteral("Workflow Canvas\nPlace nodes and connect ports\nHelp opens here as a tab"),
                   QColor(QStringLiteral("#fff6e7")),
                   QColor(QStringLiteral("#d8c7ac")));
    drawRoundedBox(painter,
                   QRectF(640, 146, 190, 160),
                   chinese ? QString::fromUtf8("右侧 Inspector\n编辑属性并查看校验提示")
                           : QStringLiteral("Inspector\nEdit properties and\nvalidation hints"),
                   QColor(QStringLiteral("#f2eadf")),
                   QColor(QStringLiteral("#d8cfbf")));
    drawRoundedBox(painter,
                   QRectF(30, 330, 800, 54),
                   chinese ? QString::fromUtf8("底部状态栏\n校验摘要 / 连线反馈 / 缩放比例")
                           : QStringLiteral("Status bar\nValidation summary / connection feedback / zoom"),
                   QColor(QStringLiteral("#edf3fb")),
                   QColor(QStringLiteral("#b8cbe8")));

    return image;
}

QImage makeMinimalWorkflowImage(bool chinese)
{
    QImage image(860, 260, QImage::Format_ARGB32_Premultiplied);
    image.fill(QColor(QStringLiteral("#fffdf9")));

    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing);

    QFont titleFont = painter.font();
    titleFont.setPointSize(17);
    titleFont.setBold(true);
    painter.setFont(titleFont);
    painter.setPen(QColor(QStringLiteral("#4c433b")));
    painter.drawText(QRectF(24, 16, 812, 34),
                     Qt::AlignLeft | Qt::AlignVCenter,
                     chinese ? QString::fromUtf8("最小工作流") : QStringLiteral("Minimal Workflow"));

    QFont nodeFont = painter.font();
    nodeFont.setPointSize(12);
    nodeFont.setBold(true);
    painter.setFont(nodeFont);

    const QStringList labels = chinese
                                   ? QStringList{QString::fromUtf8("开始"), QString::fromUtf8("提示词"),
                                                 QString::fromUtf8("大模型"), QString::fromUtf8("输出")}
                                   : QStringList{QStringLiteral("Start"), QStringLiteral("Prompt"),
                                                 QStringLiteral("LLM"), QStringLiteral("Output")};

    QVector<QRectF> rects;
    rects << QRectF(60, 104, 140, 70) << QRectF(260, 104, 140, 70) << QRectF(460, 104, 140, 70)
          << QRectF(660, 104, 140, 70);

    for (int i = 0; i < rects.size(); ++i) {
        drawRoundedBox(painter,
                       rects.at(i),
                       labels.at(i),
                       QColor(QStringLiteral("#f7f1e7")),
                       QColor(QStringLiteral("#d8c7ac")));

        if (i + 1 < rects.size()) {
            const QPointF start(rects.at(i).right() + 16, rects.at(i).center().y());
            const QPointF end(rects.at(i + 1).left() - 16, rects.at(i + 1).center().y());
            painter.setPen(QPen(QColor(QStringLiteral("#4b84d9")), 3.0, Qt::SolidLine, Qt::RoundCap));
            painter.drawLine(start, end);
            QPolygonF arrow;
            arrow << QPointF(end.x(), end.y()) << QPointF(end.x() - 10, end.y() - 6)
                  << QPointF(end.x() - 10, end.y() + 6);
            painter.setBrush(QColor(QStringLiteral("#4b84d9")));
            painter.drawPolygon(arrow);
        }
    }

    painter.setPen(QColor(QStringLiteral("#7b6b5a")));
    QFont captionFont = painter.font();
    captionFont.setPointSize(11);
    captionFont.setBold(false);
    painter.setFont(captionFont);
    painter.drawText(QRectF(60, 198, 740, 32),
                     Qt::AlignCenter,
                     chinese ? QString::fromUtf8("开始 → 提示词 → 大模型 → 输出")
                             : QStringLiteral("Start → Prompt → LLM → Output"));

    return image;
}

QImage makeValidationCardImage(bool chinese)
{
    QImage image(860, 310, QImage::Format_ARGB32_Premultiplied);
    image.fill(QColor(QStringLiteral("#fffdf9")));

    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing);

    QFont titleFont = painter.font();
    titleFont.setPointSize(17);
    titleFont.setBold(true);
    painter.setFont(titleFont);
    painter.setPen(QColor(QStringLiteral("#4c433b")));
    painter.drawText(QRectF(24, 16, 812, 34),
                     Qt::AlignLeft | Qt::AlignVCenter,
                     chinese ? QString::fromUtf8("校验提示如何读") : QStringLiteral("Reading Validation Feedback"));

    drawRoundedBox(painter,
                   QRectF(72, 78, 250, 150),
                   chinese ? QString::fromUtf8("提示词\n用户提示模板：空\nwarning")
                           : QStringLiteral("Prompt\nUser template: empty\nwarning"),
                   QColor(QStringLiteral("#fff6e7")),
                   QColor(QStringLiteral("#d8c7ac")));

    drawRoundedBox(painter,
                   QRectF(372, 78, 210, 64),
                   chinese ? QString::fromUtf8("Inspector\n高亮问题字段")
                           : QStringLiteral("Inspector\nHighlights field"),
                   QColor(QStringLiteral("#eef6e8")),
                   QColor(QStringLiteral("#8cb878")));

    drawRoundedBox(painter,
                   QRectF(372, 164, 360, 64),
                   chinese ? QString::fromUtf8("状态栏\n当前选中节点的校验摘要")
                           : QStringLiteral("Status Bar\nValidation summary for selection"),
                   QColor(QStringLiteral("#edf3fb")),
                   QColor(QStringLiteral("#b8cbe8")));

    painter.setPen(QPen(QColor(QStringLiteral("#4b84d9")), 3.0, Qt::SolidLine, Qt::RoundCap));
    painter.drawLine(QPointF(322, 124), QPointF(368, 110));
    painter.drawLine(QPointF(322, 158), QPointF(368, 196));

    painter.setPen(QColor(QStringLiteral("#9b6a20")));
    QFont badgeFont = painter.font();
    badgeFont.setPointSize(12);
    badgeFont.setBold(true);
    painter.setFont(badgeFont);
    drawRoundedBox(painter,
                   QRectF(92, 238, 132, 38),
                   chinese ? QString::fromUtf8("warning") : QStringLiteral("warning"),
                   QColor(QStringLiteral("#fff1d6")),
                   QColor(QStringLiteral("#d69545")),
                   QColor(QStringLiteral("#9b6a20")));

    return image;
}
}

HelpDocumentWidget::HelpDocumentWidget(QWidget *parent)
    : QWidget(parent)
    , _browser(new QTextBrowser(this))
{
    setObjectName("helpDocumentWidget");

    _browser->setObjectName("helpDocumentBrowser");
    _browser->setOpenExternalLinks(true);
    _browser->setReadOnly(true);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(_browser);

    retranslateUi();
}

void HelpDocumentWidget::retranslateUi()
{
    const bool chinese = tr("Overview") != QStringLiteral("Overview");
    registerIllustrationResources(chinese);
    _browser->setHtml(buildHelpContent());
}

void HelpDocumentWidget::registerIllustrationResources(bool chinese)
{
    _browser->document()->addResource(QTextDocument::ImageResource,
                                      QUrl(QStringLiteral("help://layout-overview")),
                                      makeLayoutOverviewImage(chinese));
    _browser->document()->addResource(QTextDocument::ImageResource,
                                      QUrl(QStringLiteral("help://minimal-workflow")),
                                      makeMinimalWorkflowImage(chinese));
    _browser->document()->addResource(QTextDocument::ImageResource,
                                      QUrl(QStringLiteral("help://validation-card")),
                                      makeValidationCardImage(chinese));
}

QString HelpDocumentWidget::buildHelpContent() const
{
    const bool chinese = tr("Overview") != QStringLiteral("Overview");

    if (!chinese) {
        QString html;
        html += QStringLiteral("<h1>AI Workflow Editor — User Guide</h1>");
        html += QStringLiteral(
            "<div class='hero'><b>What this is:</b> a visual authoring tool for AI workflow structure. "
            "It helps you design, validate, save, reopen, and export workflows. It is not a chat shell, "
            "not an LLM runtime, and not a generic BPM/low-code system.</div>");

        html += QStringLiteral("<h2>Illustrated Tour</h2>");
        html += QStringLiteral(
            "<div class='help-figure'>"
            "<img src='help://layout-overview' width='720'/>"
            "<div class='caption'>Toolbar and menus, Node Library, Workflow Canvas, Inspector, and Status Bar. "
            "The canvas is the first tab in the central workspace. The user guide opens as another tab.</div>"
            "</div>");

        html += QStringLiteral("<h2>5-Minute Route</h2>");
        html += QStringLiteral(
            "<ol>"
            "<li>Add <b>Start</b>, <b>Prompt</b>, <b>LLM</b>, and <b>Output</b>.</li>"
            "<li>Connect them as <code>Start → Prompt → LLM → Output</code>.</li>"
            "<li>Select Prompt and fill the user template.</li>"
            "<li>Select LLM and fill the model name.</li>"
            "<li>Save the workflow, reopen it, then try exporting Python code.</li>"
            "</ol>");
        html += QStringLiteral(
            "<div class='help-figure'>"
            "<img src='help://minimal-workflow' width='720'/>"
            "<div class='caption'>Minimal workflow: Start → Prompt → LLM → Output.</div>"
            "</div>");

        html += QStringLiteral("<h2>Core Rules</h2>");
        html += QStringLiteral(
            "<ul>"
            "<li>Connections must be dragged from output ports to input ports.</li>"
            "<li>Port data types protect the graph from invalid wiring: flow, text, completion, error, and http_response.</li>"
            "<li>Nodes show warning / error states when required fields are empty or structural links are missing.</li>"
            "<li>The Inspector highlights the exact field that needs attention.</li>"
            "</ul>");
        html += QStringLiteral(
            "<div class='help-figure'>"
            "<img src='help://validation-card' width='720'/>"
            "<div class='caption'>Example: Prompt, User template: empty, warning. "
            "Node cards, Inspector messages, and the status bar all reflect the same validation state.</div>"
            "</div>");

        html += QStringLiteral("<h2>Node Types</h2>");
        html += QStringLiteral(
            "<table><tr><th>Group</th><th>Nodes</th><th>Use</th></tr>"
            "<tr><td>Flow</td><td>Start, Condition, Output</td><td>Entry, branching, and final result collection.</td></tr>"
            "<tr><td>AI</td><td>Prompt, LLM, Agent, Memory, Retriever</td><td>Prompting, model calls, agent behavior, and context retrieval.</td></tr>"
            "<tr><td>Data / Integration</td><td>Template Variables, JSON Transform, HTTP Request, Tool, Chat Output</td><td>Prepare data, call APIs, expose tools, and format responses.</td></tr>"
            "</table>");

        html += QStringLiteral("<h2>Saving, Loading, and Exporting</h2>");
        html += QStringLiteral(
            "<p>Workflows are saved as JSON. The title bar shows <code>*</code> when unsaved changes exist. "
            "Export is available from <b>File &gt; Export</b>: Python (LangChain), Python (LangGraph), "
            "Python (CrewAI), and Python Script.</p>"
            "<div class='tip'>Exports are code skeletons. Use them as starting points and customize them in your own environment.</div>");

        html += QStringLiteral("<h2>Navigation</h2>");
        html += QStringLiteral(
            "<ul>"
            "<li><b>Center</b> focuses the current selection, or fits the graph if nothing is selected.</li>"
            "<li><b>Fit Workflow</b> always fits the whole graph.</li>"
            "<li>The mini-map appears when nodes exist. Click or drag it to navigate large canvases.</li>"
            "<li>Help opens in a reusable central tab and does not consume the node library or Inspector area.</li>"
            "</ul>");

        html += QStringLiteral("<h2>Problems Panel</h2>");
        html += QStringLiteral(
            "<p>The bottom <b>Problems</b> panel lists every current workflow warning or error.</p>"
            "<ul>"
            "<li>The title shows the total count, and the filter can switch between All, Errors, and Warnings.</li>"
            "<li>Rows show severity, node name, node type, and the issue message.</li>"
            "<li>Click a row to select that node, center the canvas, and highlight the relevant Inspector field.</li>"
            "<li>Fixing a node immediately refreshes the list, so resolved problems disappear.</li>"
            "<li>Export preflight uses this same list. Fix Problems before exporting code.</li>"
            "</ul>");

        html += QStringLiteral("<h2>Arranging Larger Workflows</h2>");
        html += QStringLiteral(
            "<p>When the canvas gets crowded, select multiple nodes and use <b>View &gt; Arrange</b>.</p>"
            "<ul>"
            "<li><b>Align Left / Right / Top / Bottom</b> straightens two or more selected nodes.</li>"
            "<li><b>Distribute Horizontally / Vertically</b> spaces three or more selected nodes evenly.</li>"
            "<li>Manual node moves and arrange operations participate in undo / redo and are saved with the workflow.</li>"
            "</ul>");

        html += QStringLiteral("<h2>Keyboard Shortcuts</h2>");
        html += QStringLiteral(
            "<table><tr><th>Shortcut</th><th>Action</th></tr>"
            "<tr><td><kbd>Ctrl/⌘+N</kbd></td><td>New workflow</td></tr>"
            "<tr><td><kbd>Ctrl/⌘+O</kbd></td><td>Open workflow</td></tr>"
            "<tr><td><kbd>Ctrl/⌘+S</kbd></td><td>Save workflow</td></tr>"
            "<tr><td><kbd>Ctrl/⌘+Z</kbd></td><td>Undo</td></tr>"
            "<tr><td><kbd>Ctrl/⌘+D</kbd></td><td>Duplicate selected nodes</td></tr>"
            "<tr><td><kbd>Space</kbd></td><td>Center selection</td></tr>"
            "<tr><td><kbd>Ctrl/⌘+0</kbd></td><td>Fit workflow</td></tr>"
            "<tr><td><kbd>F1</kbd></td><td>Open User Guide</td></tr>"
            "</table>");

        html += QStringLiteral("<h2>Documentation Maintenance Rule</h2>");
        html += QStringLiteral(
            "<div class='warning'>When a new user-facing feature is developed, the in-app User Guide and "
            "<code>docs/user-guide.md</code> must be updated in the same change. Tests should cover key menus, "
            "shortcuts, export formats, and node types so missing help updates are caught early.</div>");

        return wrap(html);
    }

    QString html;
    html += QStringLiteral("<h1>AI 工作流编辑器 — 用户指南</h1>");
    html += QStringLiteral(
        "<div class='hero'><b>它是什么：</b>一个用于设计 AI 工作流结构的可视化编排工具。"
        "它帮助你搭建、校验、保存、重新打开和导出工作流。它不是聊天壳子，不是大模型运行器，也不是通用 BPM / 低代码平台。</div>");

    html += QStringLiteral("<h2>图文导览</h2>");
    html += QStringLiteral(
        "<div class='help-figure'>"
        "<img src='help://layout-overview' width='720'/>"
        "<div class='caption'>顶部工具栏和菜单、左侧节点库、中间工作流画布、右侧 Inspector、底部状态栏。"
        "画布是中央工作区的第一个页签；用户指南会作为另一个页签打开。</div>"
        "</div>");

    html += QStringLiteral("<h2>5 分钟路线</h2>");
    html += QStringLiteral(
        "<ol>"
        "<li>添加 <b>开始</b>、<b>提示词</b>、<b>大模型</b> 和 <b>输出</b>。</li>"
        "<li>把它们连成 <code>开始 → 提示词 → 大模型 → 输出</code>。</li>"
        "<li>选中提示词节点，填写用户提示模板。</li>"
        "<li>选中大模型节点，填写模型名称。</li>"
        "<li>保存工作流，重新打开，再尝试导出 Python 代码。</li>"
        "</ol>");
    html += QStringLiteral(
        "<div class='help-figure'>"
        "<img src='help://minimal-workflow' width='720'/>"
        "<div class='caption'>最小工作流：开始 → 提示词 → 大模型 → 输出。</div>"
        "</div>");

    html += QStringLiteral("<h2>核心规则</h2>");
    html += QStringLiteral(
        "<ul>"
        "<li>连线只允许从输出端口拖到输入端口。</li>"
        "<li>端口类型会保护图结构：flow、text、completion、error、http_response。</li>"
        "<li>节点出现 warning / error，通常表示必填字段为空、端口缺少连接或 JSON 配置无效。</li>"
        "<li>Inspector 会高亮真正需要处理的字段，状态栏会给出当前选中节点的摘要。</li>"
        "</ul>");
    html += QStringLiteral(
        "<div class='help-figure'>"
        "<img src='help://validation-card' width='720'/>"
        "<div class='caption'>示例：提示词，用户提示模板：空，warning。"
        "节点卡片、Inspector 和状态栏会显示同一套校验状态。</div>"
        "</div>");

    html += QStringLiteral("<h2>节点类型</h2>");
    html += QStringLiteral(
        "<table><tr><th>分组</th><th>节点</th><th>用途</th></tr>"
        "<tr><td>流程</td><td>开始、条件、输出</td><td>定义入口、分支和最终结果收集。</td></tr>"
        "<tr><td>AI</td><td>提示词、大模型、Agent、记忆、检索器</td><td>组织提示词、模型调用、智能体行为和上下文检索。</td></tr>"
        "<tr><td>数据 / 集成</td><td>模板变量、JSON 转换、HTTP 请求、工具、聊天输出</td><td>准备数据、调用接口、暴露工具并格式化回复。</td></tr>"
        "</table>");

    html += QStringLiteral("<h2>保存、加载与导出</h2>");
    html += QStringLiteral(
        "<p>工作流保存为 JSON 文件。窗口标题出现 <code>*</code> 表示有未保存更改。"
        "导出入口在 <b>文件 &gt; 导出</b>，当前支持 Python (LangChain)、Python (LangGraph)、"
        "Python (CrewAI) 和 Python 脚本。</p>"
        "<div class='tip'>导出前会先做预检。如果底部问题面板里还有 warning / error，编辑器会先提示需要修复的节点问题，"
        "不会直接打开导出文件对话框。导出结果是代码骨架，适合作为开发起点，再到自己的工程里补充真实运行逻辑。</div>");

    html += QStringLiteral("<h2>画布导航</h2>");
    html += QStringLiteral(
        "<ul>"
        "<li><b>居中</b> 会聚焦当前选区；没有选区时适配整张图。</li>"
        "<li><b>适配全图</b> 总是把完整工作流拉回视野。</li>"
        "<li><b>画布小地图</b> 会在有节点时出现，点击或拖拽小地图可以快速导航大画布。</li>"
        "<li>帮助文档会在中央工作区复用页签，不占用节点库和 Inspector 的空间。</li>"
        "</ul>");

    html += QStringLiteral("<h2>问题面板</h2>");
    html += QStringLiteral(
        "<p>底部 <b>问题</b> 面板会汇总当前工作流中的所有 warning / error。</p>"
        "<ul>"
        "<li>标题会显示总数，例如 <b>问题 (2)</b>；右上角可以按全部、错误、警告过滤。</li>"
        "<li>每一行会显示级别、节点名称、节点类型和问题描述。</li>"
        "<li>点击某一行会自动选中对应节点、居中画布，并高亮相关 Inspector 字段。</li>"
        "<li>修复节点后列表会立即刷新，已经解决的问题会自动消失。</li>"
        "<li>导出预检也使用同一份问题列表；导出前建议先清到 <b>问题 (0)</b>。</li>"
        "</ul>");

    html += QStringLiteral("<h2>整理大型工作流</h2>");
    html += QStringLiteral(
        "<p>当画布节点变多时，先多选节点，再使用 <b>视图 &gt; 整理</b>。</p>"
        "<ul>"
        "<li><b>左对齐 / 右对齐 / 顶部对齐 / 底部对齐</b> 可以排齐 2 个及以上节点。</li>"
        "<li><b>水平分布 / 垂直分布</b> 可以把 3 个及以上节点均匀排开。</li>"
        "<li>手动拖动节点和整理操作都会进入撤销 / 重做，并随工作流一起保存。</li>"
        "</ul>");

    html += QStringLiteral("<h2>快捷键</h2>");
    html += QStringLiteral(
        "<table><tr><th>快捷键</th><th>操作</th></tr>"
        "<tr><td><kbd>Ctrl/⌘+N</kbd></td><td>新建工作流</td></tr>"
        "<tr><td><kbd>Ctrl/⌘+O</kbd></td><td>打开工作流</td></tr>"
        "<tr><td><kbd>Ctrl/⌘+S</kbd></td><td>保存工作流</td></tr>"
        "<tr><td><kbd>Ctrl/⌘+Z</kbd></td><td>撤销</td></tr>"
        "<tr><td><kbd>Ctrl/⌘+D</kbd></td><td>复制副本</td></tr>"
        "<tr><td><kbd>Space</kbd></td><td>居中选区</td></tr>"
        "<tr><td><kbd>Ctrl/⌘+0</kbd></td><td>适配全图</td></tr>"
        "<tr><td><kbd>F1</kbd></td><td>打开用户指南</td></tr>"
        "</table>");

    html += QStringLiteral("<h2>帮助文档维护规则</h2>");
    html += QStringLiteral(
        "<div class='warning'>以后开发任何面向用户的新功能必须同步更新帮助文档，包括界面内置帮助文档和 "
        "<code>docs/user-guide.md</code>。新增节点、菜单、快捷键、导出格式或重要交互时，也要补测试，"
        "让缺失的帮助文档更新尽早暴露。</div>");

    return wrap(html);
}
