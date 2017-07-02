#!/bin/bash
# /usr/local/bin/plane-zero-server.sh
#
 
function d_start()
{
	echo "Plane Zero Server : starting service"
	/usr/local/bin/node  /home/pi/plane-zero-server/node-plane.js > /tmp/nodelog &
        export APP_PID=$!
	echo $APP_PID > /tmp/plane-zero-server.pid
        sleep 5
	echo "PID is $(cat /tmp/plane-zero-server.pid)"
}
 
function d_stop()
{
	echo "Plane Zero Server : stopping service (PID=$(cat /tmp/plane-zero-server.pid))"
	kill $(cat /tmp/plane-zero-server.pid)
	rm /tmp/plane-zero-server.pid
}
 
function d_status()
{
	ps -ef | grep node | grep -v grep
	echo "PID file indicate $(cat /tmp/plane-zero-server.pid 2>/dev/null)"
}
 
# Some things that run always
touch /var/lock/plane-zero-server
 
# Gestion des instructions du service
case "$1" in
	start)
		d_start
		;;
	stop)
		d_stop
		;;
	reload)
		d_stop
		sleep 1
		d_start
		;;
	status)
		d_status
		;;
	*)
	echo "Usage: $0 {start|stop|reload|status}"
	exit 1
	;;
esac
 
exit 0
