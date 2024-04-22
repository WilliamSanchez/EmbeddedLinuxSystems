#!/bin/sh

start() {
    /usr/bin/myQTApp
}

stop() {
    /usr/bin/killall myQTApp
}

case "$1" in
    start|restart)
        echo "Starting QtDemo"
        stop
        start
        ;;
    stop)
        echo "Stopping QtDemo"
        stop
        ;;
esac
