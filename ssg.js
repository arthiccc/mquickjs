// µStatic - The Tiny SSG
print("\x1b[1;36m--- µStatic Build Process Starting ---\x1b[0m");

var Markdown = {
    parse: function(md) {
        // Simple regex-based parser
        var html = md;
        // Headers
        html = html.replace(/^### (.*$)/gm, '<h3>$1</h3>');
        html = html.replace(/^## (.*$)/gm, '<h2>$1</h2>');
        html = html.replace(/^# (.*$)/gm, '<h1>$1</h1>');
        // Bold
        html = html.replace(/\*\*(.*?)\*\*/g, '<strong>$1</strong>');
        // Links
        html = html.replace(/\[(.*?)\]\((.*?)\)/g, '<a href="$2">$1</a>');
        // Code blocks
        html = html.replace(/```([\s\S]*?)```/g, '<pre><code>$1</code></pre>');
        // Paragraphs (simplified)
        html = html.split('\n\n').map(function(p) {
            if (p.indexOf('<h') === 0 || p.indexOf('<pre') === 0) return p;
            return '<p>' + p + '</p>';
        }).join('\n');
        return html;
    }
};

function build() {
    var layout = readFile("layout.html");
    if (!layout) {
        print("Error: layout.html not found");
        return;
    }

    var files = listFiles("content");
    if (!files) {
        print("Error: content directory not found");
        return;
    }

    for (var i = 0; i < files.length; i++) {
        var filename = files[i];
        if (filename.indexOf(".md") === -1) continue;

        print("Processing: " + filename);
        var content = readFile("content/" + filename);
        var htmlBody = Markdown.parse(content);
        
        var finalHtml = layout.replace("{{title}}", filename.replace(".md", ""));
        finalHtml = finalHtml.replace("{{body}}", htmlBody);
        
        var outName = "public/" + filename.replace(".md", ".html");
        writeFile(outName, finalHtml);
        print("Generated: " + outName);
    }
}

// Ensure public directory exists (we assume user created it or we'd need mkdir binding)
// For now, let's just run it.
build();
print("\x1b[1;32m--- µStatic Build Complete ---\x1b[0m");
