#!/bin/sh

get_abs_filename() {
    echo "$(cd "$(dirname "$1")" && pwd)/$(basename "$1")"
}
absPath=$(get_abs_filename ../build/bin/AttoTCPListen)
gnome-terminal --tab --title="TCP" -- "$absPath";
sleep 1
absPath=$(get_abs_filename ../build/bin/AttoTest)
gnome-terminal --tab --title="Test" -- "$absPath" -t 22;
sleep 1
absPath=$(get_abs_filename ../build/bin/AttoUDPSend)
gnome-terminal --tab --title="UDP" -- "$absPath" -t 22;