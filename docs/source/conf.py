# Configuration file for the Sphinx documentation builder.
#
# For the full list of built-in configuration values, see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

# -- Project information -----------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#project-information

import subprocess

project = 'Buzzay'
copyright = '2026, Byson94'
author = 'Byson94'
release = '1.0'

# -- General configuration ---------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#general-configuration

subprocess.run(["doxygen", "Doxyfile"], cwd="..")

extensions = [
    "myst_parser",
    "breathe",
    "sphinx_copybutton"
]

breathe_projects = {
    "libcula": "../xml"
}
breathe_default_project = "libcula"
exclude_patterns = []

# -- Options for HTML output -------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#options-for-html-output

html_theme = 'furo'
html_theme_options = {
    "footer_icons": [
        {
            "name": "GitHub",
            "url": "https://github.com/byson94/libcula",
            "html": "",
            "class": "fa-brands fa-solid fa-github fa-2x",
        },
    ],
    "source_repository": "https://github.com/byson94/lubcula/",
    "source_branch": "main",
    "source_directory": "docs/source",
}

html_title = "libcula docs"

