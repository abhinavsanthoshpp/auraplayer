#!/usr/bin/env bash
set -e

echo "=== AuraPlayer Installation Script ==="

INSTALL_PREFIX="${HOME}/.local"
BIN_DIR="${INSTALL_PREFIX}/bin"
APPS_DIR="${INSTALL_PREFIX}/share/applications"
ICON_DIR="${INSTALL_PREFIX}/share/icons/hicolor/scalable/apps"

mkdir -p "${BIN_DIR}" "${APPS_DIR}" "${ICON_DIR}"

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${SCRIPT_DIR}/build"

if [ ! -f "${BUILD_DIR}/auraplayer" ]; then
    echo "Building AuraPlayer with CMake..."
    cmake -B "${BUILD_DIR}" -S "${SCRIPT_DIR}" -DCMAKE_BUILD_TYPE=Release
    cmake --build "${BUILD_DIR}" -j"$(nproc)"
fi

echo "Installing binary to ${BIN_DIR}/auraplayer..."
cp -f "${BUILD_DIR}/auraplayer" "${BIN_DIR}/auraplayer"
chmod +x "${BIN_DIR}/auraplayer"

echo "Installing application icon to ${ICON_DIR}/auraplayer.svg..."
cp -f "${SCRIPT_DIR}/resources/icons/auraplayer.svg" "${ICON_DIR}/auraplayer.svg"

echo "Installing desktop launcher to ${APPS_DIR}/auraplayer.desktop..."
sed "s|Exec=auraplayer|Exec=${BIN_DIR}/auraplayer|g" "${SCRIPT_DIR}/resources/auraplayer.desktop" > "${APPS_DIR}/auraplayer.desktop"
chmod +x "${APPS_DIR}/auraplayer.desktop"

if command -v update-desktop-database >/dev/null 2>&1; then
    update-desktop-database "${APPS_DIR}" 2>/dev/null || true
fi

if command -v gtk-update-icon-cache >/dev/null 2>&1; then
    gtk-update-icon-cache -f -t "${INSTALL_PREFIX}/share/icons/hicolor" 2>/dev/null || true
fi

echo "============================================="
echo "✅ AuraPlayer successfully installed!"
echo "You can launch it by running: auraplayer"
echo "Or find it in your system Applications menu!"
echo "============================================="
