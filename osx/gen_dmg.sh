#!/bin/bash

APP=${1}
VERSION=${2}

rm -f "${APP}"-"${VERSION}".dmg

mkdir -p dmg_template
cp -r "${APP}".app dmg_template/
ln -s /Applications dmg_template

hdiutil create -fs HFS+ -srcfolder dmg_template/ -volname "${APP}" -format UDRW "${APP}"-temp.dmg
open .
read
hdiutil convert "${APP}"-temp.dmg -format UDZO -imagekey zlib-level=9 -o "${APP}"-"${VERSION}".dmg

rm "${APP}"-temp.dmg
rm -rf dmg_template

