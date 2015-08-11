#!/bin/bash -e

set -o errexit -o nounset -o pipefail

DEMO_DIR=`dirname $0`
PROJECT_DIR=$DEMO_DIR/..

echo "Launching cluster with network isolation modules enabled..."

docker-compose up -d

$PROJECT_DIR/add-container-route.sh

echo ""
echo "Done."
