#include "app/HelpDocumentWidget.hpp"

#include <QTextBrowser>
#include <QVBoxLayout>

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
    _browser->setHtml(buildHelpContent());
}

QString HelpDocumentWidget::buildHelpContent() const
{
    const QString css = QStringLiteral(
        "body { color: #3d3529; background: #faf6ef; padding: 24px 40px; line-height: 1.7; }"
        "h1 { font-size: 22px; color: #5b4f3e; border-bottom: 2px solid #d8cfbf; padding-bottom: 8px; margin-top: 0; }"
        "h2 { font-size: 17px; color: #6b5d4d; margin-top: 28px; border-bottom: 1px solid #e8e0d4; padding-bottom: 4px; }"
        "h3 { font-size: 14px; color: #7a6c5a; margin-top: 20px; }"
        "p, li { font-size: 13px; }"
        "code { background: #f0e8dc; padding: 2px 5px; border-radius: 3px; font-size: 12px; }"
        "table { border-collapse: collapse; width: 100%; margin: 12px 0; }"
        "th { background: #f0e8dc; color: #5b4f3e; text-align: left; padding: 8px 12px; font-size: 13px; }"
        "td { border-bottom: 1px solid #e8e0d4; padding: 7px 12px; font-size: 13px; }"
        "tr:hover td { background: #f5efe5; }"
        "kbd { background: #e8e0d4; border: 1px solid #d0c8b8; border-radius: 3px; "
        "      padding: 1px 6px; font-size: 12px; }"
        ".tip { background: #eef6e8; border-left: 3px solid #8cb878; padding: 10px 14px; "
        "       margin: 12px 0; border-radius: 0 4px 4px 0; font-size: 13px; }"
        ".section-icon { font-size: 16px; margin-right: 6px; }");

    QString html = QStringLiteral("<html><head><style>%1</style></head><body>").arg(css);

    // Title
    html += QStringLiteral("<h1>%1</h1>").arg(tr("AI Workflow Editor — User Guide"));

    // Overview
    html += QStringLiteral("<h2>%1</h2>").arg(tr("Overview"));
    html += QStringLiteral("<p>%1</p>").arg(
        tr("AI Workflow Editor is a visual workflow authoring tool. You can compose "
           "AI pipelines by dragging nodes onto the canvas, connecting them, and "
           "configuring properties in the inspector panel. Workflows can be saved as "
           "JSON files and exported to Python code for multiple frameworks."));

    // Interface Layout
    html += QStringLiteral("<h2>%1</h2>").arg(tr("Interface Layout"));
    html += QStringLiteral(
        "<table>"
        "<tr><th>%1</th><th>%2</th></tr>"
        "<tr><td>%3</td><td>%4</td></tr>"
        "<tr><td>%5</td><td>%6</td></tr>"
        "<tr><td>%7</td><td>%8</td></tr>"
        "<tr><td>%9</td><td>%10</td></tr>"
        "<tr><td>%11</td><td>%12</td></tr>"
        "</table>")
               .arg(tr("Area"), tr("Description"),
                    tr("Node Library (left panel)"),
                    tr("Lists all available node types grouped by category. Drag a node onto the canvas or double-click to add it."),
                    tr("Canvas (center)"),
                    tr("The main editing area where you build workflows by placing and connecting nodes."),
                    tr("Inspector (right panel)"),
                    tr("Shows and edits properties for the currently selected node."),
                    tr("Toolbar (top)"),
                    tr("Quick access to file operations, editing actions, and view controls."),
                    tr("Status Bar (bottom)"),
                    tr("Displays validation messages and the current zoom level."));

    html += QStringLiteral("<h2>%1</h2>").arg(tr("Workbench Tabs"));
    html += QStringLiteral("<p>%1</p>").arg(
        tr("The center workspace uses tabs. The Workflow tab contains the canvas. "
           "The User Guide opens as a separate tab, and future text or preview pages can "
           "use the same workspace without taking space away from the node library or inspector."));

    // Node Types
    html += QStringLiteral("<h2>%1</h2>").arg(tr("Node Types"));
    html += QStringLiteral("<table><tr><th>%1</th><th>%2</th><th>%3</th></tr>").arg(
        tr("Node"), tr("Category"), tr("Description"));

    struct NodeEntry { const char *name; const char *category; const char *desc; };
    NodeEntry entries[] = {
        {QT_TR_NOOP("Start"), QT_TR_NOOP("Flow"), QT_TR_NOOP("Entry point of the workflow. Every workflow should begin with a Start node.")},
        {QT_TR_NOOP("Prompt"), QT_TR_NOOP("AI"), QT_TR_NOOP("Defines a prompt template with system and user messages.")},
        {QT_TR_NOOP("LLM"), QT_TR_NOOP("AI"), QT_TR_NOOP("Calls a large language model with configurable model name, temperature, and max tokens.")},
        {QT_TR_NOOP("Agent"), QT_TR_NOOP("AI"), QT_TR_NOOP("An autonomous agent with instructions, model, and tool access.")},
        {QT_TR_NOOP("Memory"), QT_TR_NOOP("AI"), QT_TR_NOOP("Conversation memory for maintaining chat history across turns.")},
        {QT_TR_NOOP("Retriever"), QT_TR_NOOP("AI"), QT_TR_NOOP("Retrieves relevant documents from a knowledge base.")},
        {QT_TR_NOOP("Template Variables"), QT_TR_NOOP("Data"), QT_TR_NOOP("Provides key-value variables as JSON for downstream nodes.")},
        {QT_TR_NOOP("HTTP Request"), QT_TR_NOOP("Integration"), QT_TR_NOOP("Makes HTTP requests to external APIs.")},
        {QT_TR_NOOP("JSON Transform"), QT_TR_NOOP("Data"), QT_TR_NOOP("Transforms data using a JSON mapping definition.")},
        {QT_TR_NOOP("Tool"), QT_TR_NOOP("Integration"), QT_TR_NOOP("Defines a callable tool that agents can use.")},
        {QT_TR_NOOP("Condition"), QT_TR_NOOP("Flow"), QT_TR_NOOP("Branches the workflow into True/False paths based on a condition.")},
        {QT_TR_NOOP("Chat Output"), QT_TR_NOOP("Output"), QT_TR_NOOP("Formats and outputs a chat message with a role and template.")},
        {QT_TR_NOOP("Output"), QT_TR_NOOP("Output"), QT_TR_NOOP("Terminal node that collects the final workflow result.")},
    };

    for (auto const &e : entries) {
        html += QStringLiteral("<tr><td><b>%1</b></td><td>%2</td><td>%3</td></tr>")
                    .arg(tr(e.name), tr(e.category), tr(e.desc));
    }
    html += QStringLiteral("</table>");

    // Basic Operations
    html += QStringLiteral("<h2>%1</h2>").arg(tr("Basic Operations"));

    html += QStringLiteral("<h3>%1</h3>").arg(tr("Adding Nodes"));
    html += QStringLiteral("<p>%1</p>").arg(
        tr("Drag a node from the Node Library on the left and drop it onto the canvas. "
           "You can also double-click a node in the library to add it at the center of the canvas."));

    html += QStringLiteral("<h3>%1</h3>").arg(tr("Connecting Nodes"));
    html += QStringLiteral("<p>%1</p>").arg(
        tr("Click and drag from an output port (right side of a node) to an input port "
           "(left side of another node). Connections are only allowed between compatible "
           "port types. Incompatible ports will show a rejection indicator."));

    html += QStringLiteral("<h3>%1</h3>").arg(tr("Editing Properties"));
    html += QStringLiteral("<p>%1</p>").arg(
        tr("Select a node on the canvas to see its properties in the Inspector panel on the right. "
           "Edit the fields to configure the node. Changes are saved automatically and can be undone."));

    html += QStringLiteral("<h3>%1</h3>").arg(tr("Deleting Nodes and Connections"));
    html += QStringLiteral("<p>%1</p>").arg(
        tr("Select a node or connection and press <kbd>Delete</kbd> or <kbd>Backspace</kbd>. "
           "You can also right-click for a context menu with delete options."));

    // Keyboard Shortcuts
    html += QStringLiteral("<h2>%1</h2>").arg(tr("Keyboard Shortcuts"));
    html += QStringLiteral("<table><tr><th>%1</th><th>%2</th></tr>").arg(tr("Shortcut"), tr("Action"));

    struct ShortcutEntry { const char *key; const char *action; };
    ShortcutEntry shortcuts[] = {
        {"Ctrl+N / ⌘N", QT_TR_NOOP("New workflow")},
        {"Ctrl+O / ⌘O", QT_TR_NOOP("Open workflow")},
        {"Ctrl+S / ⌘S", QT_TR_NOOP("Save workflow")},
        {"Ctrl+Shift+S / ⌘⇧S", QT_TR_NOOP("Save As...")},
        {"Ctrl+Z / ⌘Z", QT_TR_NOOP("Undo")},
        {"Ctrl+Shift+Z / ⌘⇧Z", QT_TR_NOOP("Redo")},
        {"Ctrl+C / ⌘C", QT_TR_NOOP("Copy selected nodes")},
        {"Ctrl+V / ⌘V", QT_TR_NOOP("Paste nodes")},
        {"Ctrl+D / ⌘D", QT_TR_NOOP("Duplicate selected nodes")},
        {"Ctrl+A / ⌘A", QT_TR_NOOP("Select all nodes")},
        {"Delete / Backspace", QT_TR_NOOP("Delete selected items")},
        {"Space", QT_TR_NOOP("Center / fit selection in view")},
    };

    for (auto const &s : shortcuts) {
        html += QStringLiteral("<tr><td><kbd>%1</kbd></td><td>%2</td></tr>")
                    .arg(QString::fromUtf8(s.key), tr(s.action));
    }
    html += QStringLiteral("</table>");

    // Port Data Types
    html += QStringLiteral("<h2>%1</h2>").arg(tr("Port Data Types"));
    html += QStringLiteral("<p>%1</p>").arg(
        tr("Each port has a data type that determines which connections are allowed. "
           "The following types are used:"));
    html += QStringLiteral("<table><tr><th>%1</th><th>%2</th></tr>").arg(tr("Type"), tr("Description"));

    struct PortTypeEntry { const char *type; const char *desc; };
    PortTypeEntry portTypes[] = {
        {QT_TR_NOOP("Flow"), QT_TR_NOOP("General execution flow. Compatible with all other types.")},
        {QT_TR_NOOP("Text"), QT_TR_NOOP("Text data such as prompts or responses.")},
        {QT_TR_NOOP("Completion"), QT_TR_NOOP("LLM completion results.")},
        {QT_TR_NOOP("Error"), QT_TR_NOOP("Error information for exception handling.")},
        {QT_TR_NOOP("HTTP Response"), QT_TR_NOOP("Response data from HTTP requests.")},
    };

    for (auto const &pt : portTypes) {
        html += QStringLiteral("<tr><td><b>%1</b></td><td>%2</td></tr>")
                    .arg(tr(pt.type), tr(pt.desc));
    }
    html += QStringLiteral("</table>");

    // Export
    html += QStringLiteral("<h2>%1</h2>").arg(tr("Exporting Workflows"));
    html += QStringLiteral("<p>%1</p>").arg(
        tr("Use <b>File &gt; Export</b> to generate runnable Python code from your workflow. "
           "Four export formats are available:"));
    html += QStringLiteral("<table><tr><th>%1</th><th>%2</th></tr>").arg(tr("Format"), tr("Description"));

    struct ExportEntry { const char *format; const char *desc; };
    ExportEntry exports[] = {
        {QT_TR_NOOP("Python (LangChain)"), QT_TR_NOOP("Generates code using LangChain's LCEL pipe syntax with ChatPromptTemplate and ChatOpenAI.")},
        {QT_TR_NOOP("Python (LangGraph)"), QT_TR_NOOP("Generates a LangGraph StateGraph with typed state, node functions, and edge routing.")},
        {QT_TR_NOOP("Python (CrewAI)"), QT_TR_NOOP("Maps workflow nodes to CrewAI Agents, Tasks, and Tools with Crew assembly.")},
        {QT_TR_NOOP("Python Script"), QT_TR_NOOP("Generates a standalone Python script with functions chained in a pipeline.")},
    };

    for (auto const &ex : exports) {
        html += QStringLiteral("<tr><td><b>%1</b></td><td>%2</td></tr>")
                    .arg(tr(ex.format), tr(ex.desc));
    }
    html += QStringLiteral("</table>");

    html += QStringLiteral("<div class='tip'>%1</div>").arg(
        tr("Tip: Export generates a code skeleton. You can use it as a starting point "
           "and customize the generated code in your own development environment."));

    // Saving and Loading
    html += QStringLiteral("<h2>%1</h2>").arg(tr("Saving and Loading"));
    html += QStringLiteral("<p>%1</p>").arg(
        tr("Workflows are saved as JSON files using <b>File &gt; Save</b>. The window title shows "
           "an asterisk (*) when there are unsaved changes. The editor will prompt you before "
           "discarding unsaved work when creating a new workflow, opening another file, or closing "
           "the application. Recent files are accessible via <b>File &gt; Recent Files</b>."));

    // Validation
    html += QStringLiteral("<h2>%1</h2>").arg(tr("Validation"));
    html += QStringLiteral("<p>%1</p>").arg(
        tr("Nodes are validated in real time. Warning badges appear on nodes that have missing "
           "required fields or unconnected ports. The Inspector panel highlights the specific field "
           "that needs attention. The status bar shows a summary of the selected node's validation state."));

    // Mini-map
    html += QStringLiteral("<h2>%1</h2>").arg(tr("Canvas Mini-Map"));
    html += QStringLiteral("<p>%1</p>").arg(
        tr("A mini-map overlay appears in the bottom-right corner of the canvas when nodes are present. "
           "It shows a bird's-eye view of all nodes and the current viewport. Click or drag on the "
           "mini-map to quickly navigate the canvas."));

    html += QStringLiteral("</body></html>");
    return html;
}
