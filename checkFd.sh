#!/bin/bash

if [ -z "$1" ]; then
  echo "Usage: $0 <process-name>"
  exit 1
fi

# Grab the first PID that matches the process name
pid=$(pgrep -f "$1" | head -n 1)

if [ -z "$pid" ]; then
  echo "Process '$1' not found"
  exit 1
fi

fd_dir="/proc/$pid/fd"

if [ ! -d "$fd_dir" ]; then
  echo "FD directory for PID $pid does not exist"
  exit 1
fi

echo "Open file descriptors for '$1' (PID: $pid):"
ls -l "$fd_dir"

