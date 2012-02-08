#!/bin/bash

version=0.2

echo "INSERT INTO documents (sectionid, langid, classid, nick, caption, filesize, extension) VALUES (3, 2, 8, \"tarball/corrade-$version\", \"Corrade $version (source package)\", $(du -B1 corrade-$version.tar.gz | cut -f1), \"tar.gz\");" > source-arch-$version.sql

echo "INSERT INTO documents (sectionid, langid, classid, nick, caption, filesize, extension) VALUES (3, 2, 8, \"archlinux/corrade-$version-1\", \"Corrade $version (ArchLinux package)\", $(du -B1 archlinux/corrade/corrade-$version-1.src.tar.gz | cut -f1), \"src.tar.gz\");" >> source-arch-$version.sql

echo "INSERT INTO documents (sectionid, langid, classid, nick, caption, filesize, extension) VALUES (3, 2, 8, \"archlinux/mingw32-corrade-$version-1\", \"Corrade $version (mingw32) (ArchLinux package)\", $(du -B1 archlinux/mingw32-corrade/mingw32-corrade-$version-1.src.tar.gz | cut -f1), \"src.tar.gz\");" >> source-arch-$version.sql
