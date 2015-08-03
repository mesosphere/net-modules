#!/bin/bash
docker-compose -p modules up -d

# Wait for the marathon container to come up
sleep 60
curl -X POST http://localhost:8080/v2/apps -d @`dirname $0`/sample-flask-app.json -H "Content-type: application/json"

# Wait for calico-node image to download
sleep 90

# ping sample application to see if calico networking worked
docker exec modules_slave1_1 ping -c 4 192.168.0.2
