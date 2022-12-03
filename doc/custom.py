from sphinx.util.docutils import SphinxDirective
import sphinx.addnodes
from docutils import nodes


class colorchip(nodes.Element):
    pass

def visit_colorchip(writer, node):
    writer.body.append(f'<span class="tw-colorchip" style="background-color: {node.color};"></span>')

def depart_colorchip(writer, node):
    pass

def colorchip_role(name, rawtext, text, lineno, inliner, options={}, content=[]):
    n = colorchip()
    n.color = text
    return [n], []


class collapsable(nodes.Element):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.summary = None

def visit_collapsable(writer, node):
    text = writer.encode(node.summary or 'Expand')
    writer.body.append(f'<details class="tw-collapsable"><summary><span>{text}</span></summary>')

def depart_collapsable(writer, node):
    writer.body.append(f'</details>')

