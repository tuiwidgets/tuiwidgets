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

highlight_language = 'c++'

# -- Options for HTML output -------------------------------------------------

# The theme to use for HTML and HTML Help pages.  See the documentation for
# a list of builtin themes.
#
html_theme = 'alabaster'
html_theme_options = {
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


#cpp_debug_lookup = True
#cpp_debug_show_tree = True

from sphinx.util.docutils import SphinxDirective
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

def setup(app):
    app.add_directive('adhoc-def', AdhocDef)
    app.add_role('colorchip', colorchip_role)
    app.add_node(colorchip, html=(visit_colorchip, depart_colorchip))
