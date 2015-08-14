#!/bin/bash -e

set -o errexit -o nounset -o pipefail

DEMO_DIR=`dirname $0`
PROJECT_DIR=$DEMO_DIR/..

echo "Launching cluster with network isolation modules disabled..."

pushd $DEMO_DIR/before
docker-compose -p netmodules up -d
popd

$PROJECT_DIR/add-container-route.sh

echo ""
echo "Done."

