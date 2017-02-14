for NUMBER_OF_TERMINALS in `seq 10 10 500`
do
	for CONFIG_SENDER_INTERVAL_MEAN in 0.0033 0.0025 0.002 0.0016
	do
		for CONFIG_SERVER_LINK_DATA_RATE in "100Mbps" "1Gbps" 
		do
			for CONFIG_OUTPUT_BUFFER_SIZE_BYTES in 131072 262144 524288 1310720 2621440 5242880
			do
				cp config.h.template config.h
				cat "#define NUMBER_OF_TERMINALS $NUMBER_OF_TERMINALS"						>> config.h		
				cat "#define CONFIG_SENDER_INTERVAL_MEAN $CONFIG_SENDER_INTERVAL_MEAN"                          >> config.h
				cat "#define CONFIG_SERVER_LINK_DATE_RATE $CONFIG_SERVER_LINK_DATE_RATE"                        >> config.h
				cat "#define CONFIG_OUTPUT_BUFFER_SIZE_BYTES $CONFIG_OUTPUT_BUFFER_SIZE_BYTES"                  >> config.h
				echo " " >> csmabridgeOnOff.cc
				cd ..
				./waf 
				echo "Compilation finished!"
				./waf --run csmabridge 2>&1 | cat > log_${NUMBER_OF_TERMINALS}_${CONFIG_SENDER_INTERVAL_MEAN}_${CONFIG_OUTPUT_BUFFER_SIZE_BYTES}_${CONFIG_SERVER_LINK_DATE_RATE} 
				cd scratch
			done

		done
	done
done
