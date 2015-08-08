#!/bin/bash -e

set -o errexit -o nounset -o pipefail

VM=10.141.141.10

echo "Destroying group 'stars'"

curl -X DELETE http://$VM:8080/v2/groups/star-isolated?force=true
