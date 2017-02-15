for CONFIG_NUMBER_OF_TERMINALS in `seq 10 10 500`
do
	for CONFIG_SENDER_INTERVAL_MEAN in 0.0033 0.0025 0.002 0.0016
	do
		for CONFIG_SERVER_LINK_DATA_RATE in 100000000 1000000000 
		do
			for CONFIG_OUTPUT_BUFFER_SIZE_BYTES in 131072 262144 524288 1310720 2621440 5242880
			do
				filename="log_${CONFIG_NUMBER_OF_TERMINALS}_${CONFIG_SENDER_INTERVAL_MEAN}_${CONFIG_OUTPUT_BUFFER_SIZE_BYTES}_${CONFIG_SERVER_LINK_DATA_RATE}"
				cat filename | grep PacketArrival | cut -f1 > ${filename}_PacketArrival
				cat filename | grep PacketsInBuffer | cut -f1,2 > ${filename}_BytesInBuffer
				cat filename | grep PacketDrop | cut -f1 > ${filename}_PacketDrop
			done

		done
	done
done
