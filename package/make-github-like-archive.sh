#!/bin/bash

rev=v0.2
revas=v0.2

cd ../
hash=$(git rev-parse ${revas} | head -c 7)
package=corrade-$(echo $revas | tail -c +2).tar
echo $hash $package
git archive --prefix=mosra-corrade-${hash}/ -o package/$package ${rev}
gzip -nf package/$package
