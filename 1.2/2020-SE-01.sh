#!/bin/bash

function get_inf() {
    grep -E "^[ ]*$1[ :]*(.*)$" "$2" \
        | sed -E "s/^[ ]*$1[ :]*(.*)$/\1/" \
        | tr -d '\n'
    
    if [[ $3 == 'newline' ]]; then
        echo ""
    else
        echo -n ","
    fi 
}

[[ $# -eq 2 ]] || { echo "Two args required"; exit 1; }

[[ -d $2 ]] || { echo "$2 must be a directory"; exit 3; }

dirname="$(realpath $2)"

echo "hostname,phy,vlans,hosts,failover,VPN-3DES-AES,peers,VLAN Trunk Ports,license,SN,key" > "$1"


while read filename; do
    echo -n "$(basename "$filename")," >> $1
    get_inf "Maximum Physical Interfaces" "$filename" >> $1
    get_inf "VLANs" "$filename" >> $1
    get_inf "Inside Hosts" "$filename" >> $1
    get_inf "Failover" "$filename" >> $1
    get_inf "VPN-3DES-AES" "$filename" >> $1
    get_inf "\*Total VPN Peers" "$filename" >> $1
    get_inf "VLAN Trunk Ports" "$filename" >> $1
    get_inf "This platform has a" "$filename" | sed -E 's/^(.*) license.$/\1/' >> $1
    get_inf "Serial Number" "$filename" >> $1
    get_inf "Running Activation Key" "$filename" 'newline' >> $1
done < <(find $dirname -regextype posix-extended -regex '.*\.log')

