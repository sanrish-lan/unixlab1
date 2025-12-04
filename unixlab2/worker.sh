#!/bin/sh

CID=$(tr -dc 'a-zA-Z0-9' < /dev/urandom | head -c 8)
echo "Container started with ID: $CID"

DATA_DIR="/data"

LOCK_FILE="$DATA_DIR/.lockfile"

SEQ=0

mkdir -p "$DATA_DIR"

while true; do
    SEQ=$((SEQ + 1)) 
    (
        flock -x 73
        i=1
        while true; do
            FNAME=$(printf "%03d" $i)
            FPATH="$DATA_DIR/$FNAME"
            if [ ! -e "$FPATH" ]; then
                echo "$CID $SEQ" > "$FPATH"
                echo "$FPATH" > /tmp/current_file
                break
            fi
            i=$((i + 1))
        done
    ) 73>"$LOCK_FILE"

    CURRENT_FILE=$(cat /tmp/current_file)
    echo "[$CID] Created $CURRENT_FILE (Seq: $SEQ)"

    sleep 1

    if [ -f "$CURRENT_FILE" ]; then
        rm "$CURRENT_FILE"
        echo "[$CID] Deleted $CURRENT_FILE"
    else
        echo "[$CID] Error: File $CURRENT_FILE was stolen or missing!"
    fi

    sleep 1
done
