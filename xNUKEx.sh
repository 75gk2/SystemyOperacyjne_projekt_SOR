ipcs -m | awk '/^0x/ {print $2}' | xargs -n 1 ipcrm -m
ipcs -s | awk '/^0x/ {print $2}' | xargs -n 1 ipcrm -s
ipcs -q | awk '/^0x/ {print $2}' | xargs -n 1 ipcrm -q
