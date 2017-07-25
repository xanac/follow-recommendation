#!/bin/bash
filename='/var/lib/distsn/instance-first-toot/instance-first-toot.json'
if [ -f $filename ]
then
	echo 'Content-type: application/json'
	echo ''
	cat $filename
else
	echo 'Status: 404 Not found'
fi

