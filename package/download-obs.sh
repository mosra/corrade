#!/bin/bash

ver=0.2

rm -r download-obs
mkdir -p download-obs
cd download-obs

echo -n > insert.sql

# $1 - path in OBS's URL
# $2 - directory where to save
# $3 - human readable distribution name
# $4 - patch version
# $5 - architecture
# $6 - human readable architecture

dl_deb_internal() {
    package=corrade${7}_${ver}_$5
    echo "    $package.deb"

    wget -nc -q http://download.opensuse.org/repositories/home:/mosra/$1/$5/corrade${7}_${ver}-${4}_$5.deb -O $package.deb

    filesize=$(du -B1 $package.deb | cut -f1)

    if [ $filesize = 0 ] ; then
        echo "Error downloading http://download.opensuse.org/repositories/home:/mosra/$1/$5/corrade${7}_${ver}-${4}_$5.deb"
        exit 1
    fi

    echo "INSERT INTO documents (sectionid, langid, classid, nick, caption, filesize, extension) VALUES (3, 2, 8, \"$2/$package\", \"Corrade $ver ($3 $6 package)\", $(du -B1 $package.deb | cut -f1), \"deb\");" >> ../insert.sql

    if [ -z "$7" ] ; then
        dl_deb_internal "$1" "$2" "$3" "$4" "$5" "$6 development" -dev
    fi
}

dl_rpm_internal() {
    package=corrade$7-$ver.$5
    echo "    $package.rpm"

    wget -nc -q http://download.opensuse.org/repositories/home:/mosra/$1/$5/corrade$7-$ver-$4.$5.rpm -O  $package.rpm

    filesize=$(du -B1 $package.rpm | cut -f1)

    if [ $filesize = 0 ] ; then
        echo "Error downloading http://download.opensuse.org/repositories/home:/mosra/$1/$5/corrade$7-$ver-$4.$5.rpm"
        exit 1
    fi

    echo "INSERT INTO documents (sectionid, langid, classid, nick, caption, filesize, extension) VALUES (3, 2, 8, \"$2/$package\", \"Corrade $ver ($3 $6 package)\", $(du -B1 $package.rpm | cut -f1), \"rpm\");" >> ../insert.sql

    if [ -z "$7" ] ; then
        dl_rpm_internal "$1" "$2" "$3" "$4" "$5" "$6 development" -devel
    fi
}

dl_rpm() {
    mkdir -p $2
    cd $2

    echo $2 $6

    dl_rpm_internal "$1" "$2" "$3" "$4" "$5" "$6"

    cd ..
}

dl_deb() {
    mkdir -p $2
    cd $2

    echo $2 $6

    dl_deb_internal "$1" "$2" "$3" "$4" "$5" "$6"

    cd ..
}

dl_rpm openSUSE_11.4 opensuse-11.4 "openSUSE 11.4"          1.1 i586    32bit
dl_rpm openSUSE_11.4 opensuse-11.4 "openSUSE 11.4"          1.1 x86_64  64bit

dl_rpm openSUSE_12.1 opensuse-12.1 "openSUSE 12.1"          1.1 i586    32bit
dl_rpm openSUSE_12.1 opensuse-12.1 "openSUSE 12.1"          1.1 x86_64  64bit

dl_rpm openSUSE_Factory opensuse-factory "openSUSE Factory" 1.1 i586    32bit
dl_rpm openSUSE_Factory opensuse-factory "openSUSE Factory" 1.1 x86_64  64bit

dl_rpm Fedora_15 fedora-15 "Fedora 15"                      1.1 i386    32bit
dl_rpm Fedora_15 fedora-15 "Fedora 15"                      1.1 x86_64  64bit

dl_rpm Fedora_16 fedora-16 "Fedora 16"                      1.1 i386    32bit
dl_rpm Fedora_16 fedora-16 "Fedora 16"                      1.1 x86_64  64bit

dl_rpm Mandriva_2010.1 mandriva-2010.1 "Mandriva 2010.1"    1.2 i586    32bit
dl_rpm Mandriva_2010.1 mandriva-2010.1 "Mandriva 2010.1"    1.2 x86_64  64bit

dl_rpm Mandriva_2011 mandriva-2011 "Mandriva 2011"          1.1-mdv2011.0 i586    32bit
dl_rpm Mandriva_2011 mandriva-2011 "Mandriva 2011"          1.1-mdv2011.0 x86_64  64bit

dl_deb Debian_6.0 debian-6 "Debian 6"                       1   i386    32bit
dl_deb Debian_6.0 debian-6 "Debian 6"                       1   amd64   64bit

dl_deb xUbuntu_10.04 ubuntu-10.04 "Ubuntu 10.04"            1   i386    32bit
dl_deb xUbuntu_10.04 ubuntu-10.04 "Ubuntu 10.04"            1   amd64   64bit

dl_deb xUbuntu_10.10 ubuntu-10.10 "Ubuntu 10.10"            1   i386    32bit
dl_deb xUbuntu_10.10 ubuntu-10.10 "Ubuntu 10.10"            1   amd64   64bit

dl_deb xUbuntu_11.04 ubuntu-11.04 "Ubuntu 11.04"            1   i386    32bit
dl_deb xUbuntu_11.04 ubuntu-11.04 "Ubuntu 11.04"            1   amd64   64bit

dl_deb xUbuntu_11.10 ubuntu-11.10 "Ubuntu 11.10"            1   i386    32bit
dl_deb xUbuntu_11.10 ubuntu-11.10 "Ubuntu 11.10"            1   amd64   64bit
