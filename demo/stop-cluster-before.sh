#!/bin/bash -e

set -o errexit -o nounset -o pipefail

DEMO_DIR=`dirname $0`

echo "Shutting down cluster..."

pushd $DEMO_DIR/before
docker-compose -p netmodules kill
docker-compose -p netmodules  rm --force
popd

echo ""
echo "Done."

