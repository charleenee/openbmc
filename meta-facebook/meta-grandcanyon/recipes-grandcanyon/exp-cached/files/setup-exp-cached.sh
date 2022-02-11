#!/bin/sh
#
# Copyright 2020-present Facebook. All Rights Reserved.
#
### BEGIN INIT INFO
# Provides:          setup-exp-cached
# Required-Start:
# Required-Stop:
# Default-Start:     S
# Default-Stop:
# Short-Description: Set Cachcing for SCC expander info
### END INIT INFO

echo "Setup Caching for SCC Expander info.."

<<<<<<< HEAD
/usr/bin/exp-cached > /dev/null 2>&1 &
=======
/usr/bin/exp-cached --booting > /dev/null 2>&1 &
>>>>>>> facebook/helium

echo "done."
