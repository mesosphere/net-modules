#!/bin/bash -e

set -o errexit -o nounset -o pipefail

VM=10.141.141.10
DEMO_DIR=`dirname $0`
MARATHON_DIR=$DEMO_DIR/marathon

echo "Launching group 'stars'"

curl -X POST -H "Content-Type: application/json" http://$VM:8080/v2/groups -d @$MARATHON_DIR/stars.json

echo ""
echo "Done."
