#!/bin/bash

# Screenshot directory
DIR="screenshots"
# Ensure the screenshot directory exists
mkdir -p "$DIR"

# Initialize counter
count=1

while true; do
    # Capture screenshot with fbcat and save it with an incrementing filename
    ./fbcat > "$DIR/$count.netpbm"
    
    # Increment the counter
    ((count++))
    
    # Sleep for 10 milliseconds
    sleep 0.01
done
