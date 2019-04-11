#
# Regular cron jobs for the libgatha package
#
0 4	* * *	root	[ -x /usr/bin/libgatha_maintenance ] && /usr/bin/libgatha_maintenance
