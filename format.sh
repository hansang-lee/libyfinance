#!/bin/bash
set -e

if command -v git &>/dev/null && git rev-parse --show-toplevel &>/dev/null; then
    BASE_DIR="$(git rev-parse --show-toplevel)"
elif BASE_DIR="$(find "$(realpath "${PWD}")" \
    -maxdepth 10 \
    -type f \
    -name .root \
    -exec dirname {} \; | head -n1)" && [ -n "${BASE_DIR}" ]; then :
else
    BASE_DIR="$(dirname "$(realpath "$0")")"
fi

exists() {
    if [ -e "$1" ]; then
        return 0
    fi
    return 1
}

format() {
    if exists "$1"; then
        find "$1" \
            -not -path "*/build/*" \
            -not -path "*/.git/*" \
            \( \
                -iname *.h \
                -o -iname *.hpp \
                -o -iname *.cpp \
                -o -iname *.c \
            \) \
            | xargs clang-format -i
    fi
}

if [ $# -gt 0 ]; then
    for path in "$@"; do
        format "$path"
    done
else
    format "${BASE_DIR}"
fi
