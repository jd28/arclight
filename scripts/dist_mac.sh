#!/bin/sh

set -e

if [[ -z "${Qt6_DIR}" ]]; then
  echo "Environment variable Qt6_DIR not set, do you have Qt installed?"
  exit 1
fi

echo "Configuring..."
cmake --preset macos -DCMAKE_PREFIX_PATH="${Qt6_DIR}/lib/cmake/"

echo "Building..."
cmake --build --preset default

echo "Installing..."
cmake --install build/ --prefix .

echo "Running macdeployqt..."
pushd bin/ > /dev/null
${Qt6_DIR}/bin/macdeployqt erfherder.app -dmg -always-overwrite
${Qt6_DIR}/bin/macdeployqt texview.app -dmg -always-overwrite
${Qt6_DIR}/bin/macdeployqt dlg.app -dmg -always-overwrite
${Qt6_DIR}/bin/macdeployqt arclight.app -dmg -always-overwrite
popd > /dev/null
