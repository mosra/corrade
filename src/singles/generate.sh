set -e

../acme/acme.py CorradeArrayView.h --output ../../../magnum-singles
../acme/acme.py CorradeOptional.h --output ../../../magnum-singles
../acme/acme.py CorradePointer.h --output ../../../magnum-singles
../acme/acme.py CorradeReference.h --output ../../../magnum-singles
../acme/acme.py CorradeScopeGuard.h --output ../../../magnum-singles
