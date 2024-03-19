#!/bin/bash

set -e

if [ ! -d bin ]; then
  echo "Please run this script from the root folder"
  exit
fi

if [ -z "${1}" ]; then
  echo "usage: ${0} win32|win64"
  exit
fi

if [ "${1}" = "win32" ]; then
  bit=32
elif [ "${1}" = "win64" ]; then
  bit=64
else
  echo "usage: ${0} win32|win64"
  exit
fi

# setup innosetup
dlfile="${PWD}/bin/innosetup-6.0.5.exe"
innodir="${PWD}/build/innosetup-6.0.5"
iscc="${innodir}/drive_c/InnoSetup/ISCC.exe"

# download it
if [ ! -f "${dlfile}" ]; then
    # FIXME proper dl version
    curl -L https://jrsoftware.org/download.php/is.exe -o "${dlfile}"
fi

# initialize wine
if [ ! -d "${innodir}"/drive_c ]; then
    env WINEPREFIX="${innodir}" wineboot -u
fi

# install innosetup in custom wineprefix
if [ ! -f "${innodir}"/drive_c/InnoSetup/ISCC.exe ]; then
    env WINEPREFIX="${innodir}" wine "${dlfile}" /allusers /dir=C:\\InnoSetup /nocancel /norestart /verysilent
fi

# generate resources
echo -n "" > utils/inno/resources.iss
IFS='
'
for f in $(find -L bin/Cardinal.lv2/resources/ -type f); do
    d=$(dirname $(echo ${f} | sed "s|bin/Cardinal.lv2/resources/||"))
    echo "Source: \"..\\..\\$(echo ${f} | tr '/' '\\')\"; DestDir: \"{commoncf${bit}}\\Cardinal\\$(echo ${d} | tr '/' '\\')\"; Components: resources; Flags: ignoreversion;" >> utils/inno/resources.iss
done

# generate version
echo "#define VERSION \"$(make version)\"" > utils/inno/version.iss

# create the installer file
pushd "utils/inno"
env WINEPREFIX="${innodir}" wine "${iscc}" "win${bit}.iss"
popd

# move installer file where CI expects it to be
mv utils/inno/*.exe .
