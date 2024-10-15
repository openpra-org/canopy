#!/bin/sh -e
### BEGIN INIT INFO
# Provides:          set-dri-permissions
# Required-Start:    
# Required-Stop:     
# Should-Start:      
# Should-Stop:       
# Default-Start:     6
# Default-Stop:      
# Short-Description: Set permissions for DRI devices.
# Description:       Update permissions for devices in /dev/dri/ to be readable and writable by others.
### END INIT INFO

. /lib/lsb/init-functions

case "$1" in
  start)
    log_action_begin_msg "Setting permissions for DRI devices"
    for device in /dev/dri/*; do
      if [ -e "$device" ]; then
        chmod o+rw "$device"
        log_action_msg "Updated permissions for device $device"
      else
        log_warning_msg "No DRI devices found"
      fi
    done
    log_action_end_msg 0
    ;;

  stop|restart|reload|force-reload)
    log_warning_msg "Action '$1' is meaningless for this init script"
    exit 0
    ;;

  *)
    log_success_msg "Usage: $0 start"
    exit 1
    ;;
esac

exit 0
