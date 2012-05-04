stat=$2
pid=$3

./stats $1 | grep $stat | grep $pid | cut -d ' ' -f 4 | Tools/avg
