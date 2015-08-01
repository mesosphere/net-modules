#!/bin/bash -e

set -o errexit -o nounset -o pipefail

echo "Destroying group 'stars'"

curl -X DELETE http://$(boot2docker ip):8080/v2/groups/star?force=true
