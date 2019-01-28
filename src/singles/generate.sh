set -e

../acme/acme.py CorradeOptional.h --output ../../../magnum-singles
../acme/acme.py CorradePointer.h --output ../../../magnum-singles
../acme/acme.py CorradeReference.h --output ../../../magnum-singles
