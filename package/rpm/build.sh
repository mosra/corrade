#!/bin/sh

# get version slug
version_hash=$(git describe --match "v*" | sed 's/^v//' | sed 's/-/./g')
echo "** repository hash: ${version_hash} ..."

# archive reository
(cd ../.. && git archive --format=tar.gz --prefix=corrade-${version_hash}/ -o ~/rpmbuild/SOURCES/corrade-${version_hash}.tar.gz HEAD)
echo "** created archive: ~/rpmbuild/SOURCES/corrade-${version_hash}.tar.gz"
sleep 2

# replace spec version
spec_file="corrade.spec"
cp ${spec_file} ${spec_file}.build
sed -i "s/Version:.\+/Version: ${version_hash}/g" ${spec_file}.build
echo "** building package version: ${version_hash}"

# check dependencies
sudo dnf builddep -y ${spec_file}

# build package
rpmbuild --define "debug_package %{nil}" --clean -ba ${spec_file}.build
rm ${spec_file}.build
