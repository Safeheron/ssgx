#!/bin/bash
# $1: message
# $2: color name (e.g., "GREEN", "YELLOW")

MESSAGE="$1"
COLOR_NAME="$2"

# Map color name to ANSI code
case "$COLOR_NAME" in
  "GREEN")   COLOR_CODE="1;32" ;;
  "YELLOW")  COLOR_CODE="1;33" ;;
  "RED")     COLOR_CODE="1;31" ;;
  "CYAN")    COLOR_CODE="1;36" ;;
  "MAGENTA") COLOR_CODE="1;35" ;;
  "BLUE")    COLOR_CODE="1;34" ;;
  *)         COLOR_CODE="" ;; # Default: no color
esac

if [ -z "$COLOR_CODE" ]; then
    printf -- "[INFO]    -- %s\n" "$MESSAGE"
else
    COLOR_START="\033[${COLOR_CODE}m"
    COLOR_END="\033[0m"
    printf -- "${COLOR_START}[INFO]    -- %s${COLOR_END}\n" "$MESSAGE"
fi