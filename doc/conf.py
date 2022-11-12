# Configuration file for the Sphinx documentation builder.
#
# This file only contains a selection of the most common options. For a full
# list see the documentation:
# http://www.sphinx-doc.org/en/master/config

# -- Path setup --------------------------------------------------------------

# If extensions (or modules to document with autodoc) are in another directory,
# add these directories to sys.path here. If the directory is relative to the
# documentation root, use os.path.abspath to make it absolute, like shown here.
#
# import os
# import sys
# sys.path.insert(0, os.path.abspath('.'))


# -- Project information -----------------------------------------------------

project = 'Tui Widgets'
copyright = '2022, Martin Hostettler'
author = 'Martin Hostettler'

# The full version, including alpha/beta/rc tags
release = '0.3.0'

# -- General configuration ---------------------------------------------------

# Add any Sphinx extension module names here, as strings. They can be
# extensions coming with Sphinx (named 'sphinx.ext.*') or your custom
# ones.
extensions = [
  'sphinxcontrib.images',
  'sphinx.ext.intersphinx',
]

# Add any paths that contain templates here, relative to this directory.
templates_path = ['_templates']

# List of patterns, relative to source directory, that match files and
# directories to ignore when looking for source files.
# This pattern also affects html_static_path and html_extra_path.
exclude_patterns = ['_build', 'Thumbs.db', '.DS_Store']

cpp_index_common_prefix = ['Tui::']

highlight_language = 'c++'

# -- Options for HTML output -------------------------------------------------

# The theme to use for HTML and HTML Help pages.  See the documentation for
# a list of builtin themes.
#
html_theme = 'alabaster'
html_theme_options = {
    'show_relbar_bottom': True,
    'show_relbar_top': False,
    'page_width': '1340px',
    'sidebar_width': '190px',
    'github_user': 'tuiwidgets',
    'github_repo': 'tuiwidgets',
    'github_banner': True,
    'body_max_width': 950,
}

html_show_copyright = False


# Add any paths that contain custom static files (such as style sheets) here,
# relative to this directory. They are copied after the builtin static files,
# so a file named "default.css" will overwrite the builtin "default.css".
html_static_path = ['_static']

manpages_url = "https://man7.org/linux/man-pages/man{section}/{page}.{section}.html"

intersphinx_mapping = {'qt': ('https://doc.qt.io/qt-5', 'qt5.inv'),
                       'termpaint': ('https://termpaint.namepad.de/latest/', 'termpaint.inv')}

section_pages_titles = ['Widgets', 'Misc']

#cpp_debug_lookup = True
#cpp_debug_show_tree = True

from sphinx.util.docutils import SphinxDirective
from sphinx.transforms import SphinxTransform
import sphinx.addnodes
from docutils import nodes

import os, sys

x = os.path.abspath(os.path.dirname(__file__))
#print(f"Path {x}")
sys.path.append(x)
from custom import *

class AdhocDef(SphinxDirective):

    has_content = True
    required_arguments = 1
    final_argument_whitespace = True

    def run(self):
        node = sphinx.addnodes.desc()
        sig = sphinx.addnodes.desc_signature()
        sig += nodes.Text(self.arguments[0])
        node.append(sig)
        contentnode = sphinx.addnodes.desc_content()
        node.append(contentnode)
        self.state.nested_parse(self.content, self.content_offset, contentnode)
        return [node]

from sphinx.util.osutil import ensuredir
import subprocess
import hashlib

class TpiConverter(SphinxTransform):

    default_priority = 10

    def apply(self, **kwargs) -> None:
        for node in self.document.findall(nodes.image):
            if node['uri'].endswith('.tpi'):
                srcpath = node['uri']
                self.app.env.note_dependency(srcpath)
                with open(srcpath, 'rb') as f:
                    contenthash = hashlib.sha256(f.read()).hexdigest()[:10]
                filename = os.path.basename(srcpath) + '.' + contenthash + '.png'
                imagedir = os.path.join(self.app.doctreedir, '_image_tmp')
                ensuredir(imagedir)
                destpath = os.path.join(imagedir, filename)

                subprocess.run([x + '/tpi2png.py', srcpath, destpath])

                source = os.path.dirname(self.document["source"])
                destpath = os.path.relpath(destpath, source)
                node['uri'] = destpath

from bs4 import BeautifulSoup

def fixup_sidebar_toc(html):
    soup = BeautifulSoup(html, "html.parser")
    core_section_point = None
    core_open = False
    seen_real_section = False
    for element in soup.ul.children:
        if element.name == 'li':
            if len(element.contents) > 0 and element.contents[0].name == 'a' and element.contents[0].text in section_pages_titles:
                seen_real_section = True
                element['class'] += ['tw-subsection']
            else:
                # otherwise enforce maxdepth = 2
                for sub_ul in element.select('ul ul ul'):
                    sub_ul.decompose()

            if len(element.contents) > 0 and element.contents[0].name == 'a' and element.contents[0].text == 'ZTerminal':
                core_section_point = element

            if not seen_real_section and 'current' in element['class']:
                core_open = True

    pseudo_section = False
    for element in soup.ul.children:
        if element.name == 'li':
            if element is core_section_point:
                pseudo_section = True
            if 'tw-subsection' in element['class']:
                pseudo_section = False

            if pseudo_section:
                if core_open:
                    element['class'] += ['tw-pseudosection-item']
                else:
                    element['class'] += ['tw-pseudosection-hidden']

    if core_section_point:
        new_li = soup.new_tag('li')
        new_li['class'] = ['toctree-l1', 'tw-subsection']
        if core_open:
            new_li['class'] += ['tw-pseudosection-open']
        new_a = soup.new_tag('a')
        new_a['class'] = ['reference', 'internal']
        new_a['href'] = core_section_point.contents[0]['href']
        new_a.string = 'Core'
        new_li.append(new_a)
        core_section_point.insert_before(new_li)

    return str(soup)

def html_page_context(app, pagename, templatename, context, doctree):
    orig_toctree = context['toctree']
    context['toctree'] = lambda **kwargs: fixup_sidebar_toc(orig_toctree(**kwargs))

def setup(app):
    app.add_directive('adhoc-def', AdhocDef)
    app.add_role('colorchip', colorchip_role)
    app.add_node(colorchip, html=(visit_colorchip, depart_colorchip))

    app.add_transform(TpiConverter)

    app.connect('html-page-context', html_page_context)
