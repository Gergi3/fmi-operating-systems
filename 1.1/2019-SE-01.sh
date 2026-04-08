type="$(tail -n +2 $filename \
    | tr ';' ' ' \
    | sort -rn -k3 \
    | head -n 1 \
    | awk '{print $2}')"

tail -n +2 $filename \
    | tr ';' ' ' \
    | awk -v type="$type" '$2 == type' \
    | sort -n -k3 \
    | head -n 1 \
    | awk '{print $1 "\t" $4}'
