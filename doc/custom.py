from sphinx.util.docutils import SphinxDirective
import sphinx.addnodes
from docutils import nodes


class colorchip(nodes.Element):
    pass

def visit_colorchip(writer, node):
    writer.body.append(f'<span class="tw-colorchip" style="background-color: {node.color};"></span>')
    pass

def depart_colorchip(writer, node):
    pass

def colorchip_role(name, rawtext, text, lineno, inliner, options={}, content=[]):
    n = colorchip()
    n.color = text
    return [n], []

