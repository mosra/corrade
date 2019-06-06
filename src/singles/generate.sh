set -e

../acme/acme.py CorradeArrayView.h --output ../../../magnum-singles
../acme/acme.py CorradeStridedArrayView.h --output ../../../magnum-singles
../acme/acme.py CorradeArray.h --output ../../../magnum-singles
../acme/acme.py CorradeOptional.h --output ../../../magnum-singles
../acme/acme.py CorradePointer.h --output ../../../magnum-singles
../acme/acme.py CorradeReference.h --output ../../../magnum-singles
../acme/acme.py CorradeScopeGuard.h --output ../../../magnum-singles

../acme/acme.py CorradeStlForwardArray.h --output ../../../magnum-singles
../acme/acme.py CorradeStlForwardString.h --output ../../../magnum-singles
../acme/acme.py CorradeStlForwardTuple.h --output ../../../magnum-singles
../acme/acme.py CorradeStlForwardVector.h --output ../../../magnum-singles
../acme/acme.py CorradeStlMath.h --output ../../../magnum-singles
