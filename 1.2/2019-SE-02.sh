#!/bin/bash


n=10

if [[ $1 == '-n' && $2 =~ [0-9]+ ]]; then
    n=$2
    shift 2
elif [[ $1 == '-n' ]]; then 
    echo "Invalid option -n given, expected -n <number>, got -n $2" >&2
    exit 1
fi


temp=$(mktemp)

for file in $@; do
    [[ -f "$file.log" ]] || { echo "No such file $file.log" >&2; continue; }
    
    cat "$file.log" \
        | tail -n $n \
        | sed -E "s/^([0-9]{4}-[0-9]{2}-[0-9]{2} [0-9]{2}:[0-9]{2}:[0-9]{2}) (.*)$/\1 $file \2/g" >> $temp

    # awk alternative
    #cat "$file.log" \
    #    | tail -n $n \
    #    | awk -v idf=$file '{
    #        toPrint = $1 OFS $2 OFS idf;
    #        for (i = 3; i <= NF; i++) toPrint = toPrint OFS $i;
    #        print toPrint}' >> $temp
done

cat $temp | sort -t' ' -k1,2

rm $temp

