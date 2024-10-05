#!/bin/sh
set -e

# Get version slug
version_hash=$(git describe --match "v*" | sed 's/^v//' | sed 's/-/./g')
echo "** repository hash: ${version_hash} ..."

# Create dir tree for rpmbuild in user dir
rpmdev-setuptree

# Archive repository
(cd ../.. && git archive --format=tar.gz --prefix=corrade-${version_hash}/ -o ~/rpmbuild/SOURCES/corrade-${version_hash}.tar.gz HEAD)
echo "** created archive: ~/rpmbuild/SOURCES/corrade-${version_hash}.tar.gz"
sleep 2

# Replace spec version
sed -i "s/Version:.\+/Version: ${version_hash}/g" corrade.spec
echo "** building package version: ${version_hash}"

# Check dependencies
sudo dnf builddep -y corrade.spec

# Build package
rpmbuild --define "debug_package %{nil}" --clean -bb corrade.spec

echo "** packages for corrade-${version_hash} complete:"
ls -l ~/rpmbuild/RPMS/corrade-${version_hash}*.rpm
