#!/bin/bash

BDIR=/home/chap/DTI-SVL/onSeedingStudy/density_study/data/normal/region_for_task1/

for n in 1 2 3 4 5
do
	for fb in cst cg cc ifo ilf
	do
		echo "seeding resolution of ${n}x${n}x${n}, bundle of $fb" 
		./singleitr -f ${BDIR}/s${n}/${fb}/region_s${n}.data \
		-j ${BDIR}/s${n}/${fb}/tumorbox_0_region_s${n}.data \
		-j ${BDIR}/s${n}/${fb}/tumorbox_1_region_s${n}.data
	done
done
