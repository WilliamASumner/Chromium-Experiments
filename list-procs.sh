for i in `pgrep -x chrome`; do
    STR="/proc/$i/cmdline"
    echo -ne "$i:\t"

    cat $STR

    echo -e "\n"
done
