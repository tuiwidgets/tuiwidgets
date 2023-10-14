#include <Tui/ZDocument.h>
#include <Tui/ZDocumentCursor.h>
#include <Tui/ZTextMetrics.h>

void exampleCursor(Tui::ZTextMetrics textMetrics, Tui::ZDocument &doc) {
// snippet-start
    Tui::ZDocumentCursor cursor{&doc, [textMetrics, &doc](int line, bool wrappingAllowed) {
            Tui::ZTextLayout lay(textMetrics, doc.line(line));
            // <your custom configuration here>
            lay.doLayout(65000);
            return lay;
        }
    };
// snippet-end
}
