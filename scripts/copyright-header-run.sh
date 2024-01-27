#!/bin/bash

# Requires: https://github.com/cloudposse/copyright-header

copyright-header --license MIT --copyright-software emlabcpp --copyright-holder "Jan Veverak Koniarik" --copyright-year "2023" --copyright-software-description "Embedded library" --word-wrap 100 --guess-extension --syntax ./scripts/copyright-header-syntax.yml --add-path src:include:examples:tests:cmake --output-dir ./
