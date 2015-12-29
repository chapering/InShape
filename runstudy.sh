#!/bin/bash 
: '
this script, presuming that all executables and data have been readyed in the
configurable directories as follows, boost a run of the whole study of seeding
methods in DTI tractography.
--- this version is proprietarily written for pilot study.
'
BASEDIR=`pwd`

BINDIR=${BASEDIR}/bin
TASKDIRS="pretask task0 task1 task2 task3 task4 task5 task6 task7"
EXECUTABLES="singleitr multiviewitr multiwindowitr"

DATADIR=${BASEDIR}/data
NORMALCASEDATADIR=${DATADIR}/normal
ABNORMALCASEDATADIR=${DATADIR}/agenesis
TUMORDATADIR=${DATADIR}/tumor

LOG=${BASEDIR}/studylog

function checkbins()
{
	if [ ! -d ${BINDIR} ];then
		echo "FATAL: directory ${BINDIR} NOT found, please set up firstly."
		return 1
	fi

	for ts in ${TASKDIRS}
	do
		for bin in ${EXECUTABLES}
		do
			if [ ! -s ${BINDIR}/${ts}/${bin} ];then
				echo "ERROR: executable ${bin} for ${ts} NOT found,"
			    echo "please set up firstly."
				return 1
			fi
		done
	done
	return 0
}

function checkdata()
{
	for bdir in ${NORMALCASEDATADIR} ${ABNORMALCASEDATADIR}
	do
		if [ ! -d ${bdir} ];then
			echo "ERROR: data directory ${bdir} NOT found."
			return 1
		fi

		for ((n=1;n<=5;n++));do
			if [ `ls ${bdir}/*_s${n}_?.data | wc -l` -le 0 ];then
				echo "ERROR: data of resolution ${n}x${n}x${n} NOT found in the"
				echo "directory ${bdir}"
				return 1
			fi
		done
	done

	if [ ! -s ${TUMORDATADIR}/potato.sm ];then
		echo "ERROR: tumor data NOT found in ${TUMORDATADIR}."
		return 1
	fi

	return 0
}

function taskflag()
{
	echo -e "\n##############################################################" >> $LOG
	echo "                           TASK $1                            " >> $LOG
	echo -e "##############################################################\n" >> $LOG
}

#-----------------------------------------#
#
# pretask
#
#-----------------------------------------#
function pretask()
{
	: '
	this is a trivial, actually virutal, task. It just show the introductory
	message of the tasks to follow
	'
	${BINDIR}/pretask/singleitr \
		-f ${NORMALCASEDATADIR}/*_s5_?.data \
		-t ${BINDIR}/pretask/tasktext \
		-V 1>> $LOG 2>&1
}

#-----------------------------------------#
#
# task 0 
#
#-----------------------------------------#
function task0()
{
	taskflag 0
	setid=1
	if [ $# -ge 1 ];then
		setid=$1
	fi

	for ((n=1;n<=5;n++))
	do
		echo -e "\n##### with Normal case #####" >> $LOG
		echo "##### under seeding resolution of ${n}x${n}x${n} #####" >> $LOG
		${BINDIR}/task0/singleitr \
		-f \
		${NORMALCASEDATADIR}/region_for_task0/region*normalcase_s${n}_?.data \
		-i \
		${NORMALCASEDATADIR}/region_for_task0/set${setid}/FApt*normalcase_s${n}_?.data \
		-p ${BINDIR}/task0/helptext \
		-t ${BINDIR}/task0/tasktext \
		-V 1>> $LOG 2>&1
	done
}

#-----------------------------------------#
#
# task 1 
#
#-----------------------------------------#
function task1()
{
	taskflag 1
	for ((n=1;n<=5;n++))
	do
		echo -e "\n##### with Normal case #####" >> $LOG
		echo "##### under seeding resolution of ${n}x${n}x${n} #####" >> $LOG
		${BINDIR}/task1/singleitr \
		-f \
		${NORMALCASEDATADIR}/region_for_task1/region*normalcase_s${n}_?.data \
		-p ${BINDIR}/task1/helptext \
		-t ${BINDIR}/task1/tasktext \
		-a 4 \
		-V 1>> $LOG 2>&1
	done

	for ((n=1;n<=5;n++))
	do
		echo -e "\n##### with Abnormal case #####" >> $LOG
		echo "##### under seeding resolution of ${n}x${n}x${n} #####" >> $LOG
		${BINDIR}/task1/singleitr \
		-f \
		${ABNORMALCASEDATADIR}/region_for_task1/region*Agcase1_s${n}_?.data \
		-p ${BINDIR}/task1/helptext \
		-t ${BINDIR}/task1/tasktext \
		-a 3 \
		-V 1>> $LOG 2>&1
	done
}

#-----------------------------------------#
#
# task 2 
#
#-----------------------------------------#
function task2()
{
	taskflag 2
	for ((n=1;n<=5;n++))
	do
		echo -e "\n##### with Normal case #####" >> $LOG
		echo "##### under seeding resolution of ${n}x${n}x${n} #####" >> $LOG
		${BINDIR}/task2/singleitr \
		-f \
		${NORMALCASEDATADIR}/region_for_task2/region*normalcase_s${n}_?.data \
		-i \
		${NORMALCASEDATADIR}/region_for_task2/fiberidx*normalcase_s${n}_?.data \
		-p ${BINDIR}/task2/helptext \
		-t ${BINDIR}/task2/tasktext \
		-V 1>> $LOG 2>&1
	done
}

#-----------------------------------------#
#
# task 3 
#
#-----------------------------------------#
function task3()
{
	taskflag 3
	for ((n=1;n<=5;n++))
	do
		echo -e "\n##### with Normal case #####" >> $LOG
		echo "##### under seeding resolution of ${n}x${n}x${n} #####" >> $LOG
		${BINDIR}/task3/singleitr \
		-f \
		${NORMALCASEDATADIR}/region_for_task3/region*normalcase_s${n}_?.data \
		-i \
		${NORMALCASEDATADIR}/region_for_task3/fiberidx*normalcase_s${n}_?.data \
		-p ${BINDIR}/task3/helptext \
		-t ${BINDIR}/task3/tasktext \
		-V 1>> $LOG 2>&1
	done

	for ((n=1;n<=5;n++))
	do
		echo -e "\n##### with Abnormal case #####" >> $LOG
		echo "##### under seeding resolution of ${n}x${n}x${n} #####" >> $LOG
		${BINDIR}/task3/singleitr \
		-f \
		${ABNORMALCASEDATADIR}/region_for_task3/region*Agcase1_s${n}_?.data \
		-i \
		${ABNORMALCASEDATADIR}/region_for_task3/fiberidx*Agcase1_s${n}_?.data \
		-p ${BINDIR}/task3/helptext \
		-t ${BINDIR}/task3/tasktext \
		-V 1>> $LOG 2>&1
	done
}

#-----------------------------------------#
#
# task 4 
#
#-----------------------------------------#
function task4()
{
	taskflag 4
	for ((n=1;n<=4;n++))
	do
		echo -e "\n##### with Normal case #####" >> $LOG
		((m=n+1))
		echo "##### seeding resolution ${n}x${n}x${n} v.s. ${m}x${m}x${m} #####" >> $LOG
		${BINDIR}/task4/multiviewitr \
		-f \
		${NORMALCASEDATADIR}/normalcase_s${n}_?.data \
		-f \
		${NORMALCASEDATADIR}/normalcase_s${m}_?.data \
		-p ${BINDIR}/task4/helptext \
		-t ${BINDIR}/task4/tasktext \
		-V 1>> $LOG 2>&1
	done

	for ((n=1;n<=4;n++))
	do
		echo -e "\n##### with Abnormal case #####" >> $LOG
		((m=n+1))
		echo "##### seeding resolution ${n}x${n}x${n} v.s. ${m}x${m}x${m} #####" >> $LOG
		${BINDIR}/task4/multiviewitr \
		-f \
		${ABNORMALCASEDATADIR}/Agcase1_s${n}_?.data \
		-f \
		${ABNORMALCASEDATADIR}/Agcase1_s${m}_?.data \
		-p ${BINDIR}/task4/helptext \
		-t ${BINDIR}/task4/tasktext \
		-V 1>> $LOG 2>&1
	done
}

#-----------------------------------------#
#
# task 5 
#
#-----------------------------------------#
function task5()
{
	taskflag 5
	for ((n=1;n<=5;n++))
	do
		echo -e "\n##### with Normal case #####" >> $LOG
		echo "##### under seeding resolution of ${n}x${n}x${n} #####" >> $LOG
		echo "#####    when the tumor touches fibers    #### ">> $LOG
		${BINDIR}/task5/singleitr \
		-f \
		${NORMALCASEDATADIR}/region_for_task5/region*normalcase_s${n}_?.data \
		-i \
		${NORMALCASEDATADIR}/region_for_task5/touch/tumorbox_region*normalcase_s${n}_?.data \
		-u \
		${TUMORDATADIR}/potato.sm \
		-p ${BINDIR}/task5/helptext \
		-t ${BINDIR}/task5/tasktext \
		-V 1>> $LOG 2>&1

		echo -e "\n#####    when the tumor is tangential to fibers    ####" >> $LOG
		${BINDIR}/task5/singleitr \
		-f \
		${NORMALCASEDATADIR}/region_for_task5/region*normalcase_s${n}_?.data \
		-i \
		${NORMALCASEDATADIR}/region_for_task5/non-touch/tumorbox_region*normalcase_s${n}_?.data \
		-u \
		${TUMORDATADIR}/potato.sm \
		-p ${BINDIR}/task5/helptext \
		-t ${BINDIR}/task5/tasktext \
		-V 1>> $LOG 2>&1
	done
}

#-----------------------------------------#
#
# task 6 
#
#-----------------------------------------#
function task6()
{
	taskflag 6
	for ((n=1;n<=5;n++))
	do
		echo -e "\n##### under seeding resolution of ${n}x${n}x${n} #####" >> $LOG
		${BINDIR}/task6/multiviewitr \
		-f \
		${NORMALCASEDATADIR}/normalcase_s${n}_?.data \
		-d \
		${NORMALCASEDATADIR}/b0 \
		-f \
		${ABNORMALCASEDATADIR}/Agcase1_s${n}_?.data \
		-d \
		${ABNORMALCASEDATADIR}/b0 \
		-p ${BINDIR}/task6/helptext \
		-t ${BINDIR}/task6/tasktext \
		-V 1>> $LOG 2>&1
	done
}

#-----------------------------------------#
#
# task 7 
#
#-----------------------------------------#
function task7()
{
	taskflag 7
	for ((n=1;n<=5;n++))
	do
		echo -e "\n##### with Normal case #####" >> $LOG
		echo "##### under seeding resolution of ${n}x${n}x${n} #####" >> $LOG
		${BINDIR}/task7/singleitr \
		-f \
		${NORMALCASEDATADIR}/normalcase_s${n}_?.data \
		-p ${BINDIR}/task7/helptext \
		-t ${BINDIR}/task7/tasktext \
		-a Y \
		-V 1>> $LOG 2>&1

		echo -e "\n##### with Abnormal case #####" >> $LOG
		echo "##### under seeding resolution of ${n}x${n}x${n} #####" >> $LOG
		${BINDIR}/task7/singleitr \
		-f \
		${ABNORMALCASEDATADIR}/Agcase1_s${n}_?.data \
		-p ${BINDIR}/task7/helptext \
		-t ${BINDIR}/task7/tasktext \
		-a N \
		-V 1>> $LOG 2>&1
	done
}

#####################################################################################
###    Task ordering and main flow control
###
#####################################################################################

checkbins
if [ $? -ne 0 ];then
	exit 1
fi

checkdata
if [ $? -ne 0 ];then
	exit 1
fi

> $LOG
start=`date +%s`

pretask

task0

task1

task2

task3

task4

task5

task6

task7

end=`date +%s`
echo -e "\n################## ALL FINISHED #######################" >> $LOG
((d=end-start))
((h=d/3600))
((m=d%3600/60))
((s=d%3600%60))
echo " Time cost: $h hours $m minutes $s seconds." >> $LOG

echo "All finished, thank you!"
echo

exit 0

