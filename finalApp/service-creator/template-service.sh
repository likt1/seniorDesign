#!/bin/sh
# based on gist.github.com/naholyr/4275302

### BEGIN INIT INFO
# Provides:		<NAME>
# Required-Start:	$all
# Required-Stop:	$all
# Default-Start:	2 3 4 5
# Default-Stop:		0 1 6
# Description:		<DESCRIPTION>
### END INIT INFO

SCRIPT=<COMMAND>
RUNAS=<USERNAME>

PIDFILE=/var/run/<NAME>.pid
LOGFILE=/var/log/<NAME>.log

start() {
	if [ -f $PIDFILE ]; then
		echo 'Service already running' >&2
		return 1
	fi
	echo 'Starting service...' >&2
	local CMD="$SCRIPT &> \"$LOGFILE\" & echo \$!"
	su -c "$CMD" $RUNAS > "$PIDFILE"
	echo 'Service started' >&2
}

stop() {
	if [ ! -f $PIDFILE ]; then
		echo 'Service not running' >&2
		return 1
	fi
	echo 'Stopping service...' >&2
	kill -15 $(cat "$PIDFILE") 
	rm -f "$PIDFILE"
	echo 'Service stopped' >&2
}

uninstall() {
	echo -n "Are you really sure you want to uninstall this service? It cannot be undone. [y/N]"
	local SURE
	read SURE
	if [ "$SURE" = "y" ]; then
		stop
		rm -f "$PIDFILE"
		echo "Notice: log file is not removed: '$LOGFILE'" >&2
		update-rc.d -f <NAME> remove
		rm -fv "$0"
	fi
}

case "$1" in
	start)
		start
		;;
	stop)
		stop
		;;
	uninstall)
		uninstall
		;;
	restart)
		stop
		start
		;;
	*)
		echo "Usage: $0 {start|stop|restart|uninstall}"
esac
