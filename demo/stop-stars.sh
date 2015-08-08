#!/bin/bash -e

set -o errexit -o nounset -o pipefail

echo "Destroying group 'stars'"

VM=10.141.141.10

curl -X DELETE http://$VM:8080/v2/groups/star?force=true
