#!/bin/bash

for fb in cc cg cst slf ilf ifo
do
	echo "------------------------------------"
	echo "Loading dataset for the ${fb} bundle, please wait for a few seconds..."

	timestamp=`date +%s`
	./src/singleitr -f \
	./data/${fb}/normal_s1.data \
	-i \
	./data/${fb}/fiberidx_${fb}_normal_s1.data \
	-j \
	./data/${fb}/tumorbox_${fb}_normal_s1_s.data \
	-j \
	./data/${fb}/tumorbox_${fb}_normal_s1_e.data \
	-s \
	./data/skeletons/normalSkeleton_s1.data \
	-V 1>./log/log_${fb}_${timestamp} 2>&1

done

echo "------------------------------------"
echo "This session finished, Thank you!."
exit 0

