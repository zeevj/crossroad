#!/bin/sh
 
start() {
    /usr/bin/python run_arduino.py &
    echo "Ramzor started."
}
 
stop() {
    pid=`ps -ef | grep '[p]ython run_arduino.py' | awk '{ print $2 }'`
    echo $pid
    kill $pid
    sleep 2
    echo "Ramzor killed."
}
 
case "$1" in
  start)
    start
    ;;
  stop)
    stop   
    ;;
  restart)
    stop
    start
    ;;
  *)
    echo "Usage: ramzor {start|stop|restart}"
    exit 1
esac
exit 0