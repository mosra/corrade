# Corrade RPM Package

You will need to install the following dependencies for build rpms:
```
sudo dnf install fedora-packager rpmdevtools gcc
```

Create user directory tree building RPMs:
```
mkdir -p ~/rpmbuild/{BUILD,BUILDROOT,RPMS,SOURCES,SPECS,SRPMS}
```
