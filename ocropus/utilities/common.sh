#
# common functions for test scripts in ocropus
# include this file with the following line into test scripts:
# . `dirname $0`/common.sh
# this allows for calling the scripts also from the top-level
# ocropus directory. the scripts must all reside in the same
# subfolder!
#
# Responsible: kofler

#
# makes sure that the commands are executed in the correct directory
# i.e. the top-level folder of ocropus
#
verifyDir() {
if [[ `pwd` == */utilities ]]; then
       	cd ..
fi
}

#
# verfies that a given command works properly, i.e. results in a
# specified exit code. the command mus be quoted!
# usage:   "command" expectedExitCode
# returns: 0 on success
#
verifyCommand() {
if [ $# -ne 2 ]; then
	echo >&2
	echo "WRONG USAGE OF verfiyCommand!!!" >&2
        echo usage:   "command" expectedExitCode >&2
	echo returns: 0 on success >&2
	echo >&2
	return -1
fi
echo "  command: $1" >&2
$1 > /dev/null 2>err.out
cmdRet=$?
if [ $cmdRet = $2 ]; then
        ok
	retval=0
else
        failed err.out
        retval=$cmdRet
fi

rm err.out
return $retval
}

#
# prints the name of a section
#
section() {
echo >&2
echo "  === $@ ===" >&2
echo >&2
}

#
# prints an OK message
#
ok() {
echo "  === OK ========" >&2
}

#
# prints a message that sth FAILED
# optionally pass the name of a file which contains error details
#
failed() {
echo "  === FAILED! ===" >&2
if [ ! -z "$1" ]; then
  if [ -s $1 ]; then
	echo "  (details below)" >&2
	cat $1 >&2
  fi
fi
}

#
# terminates a script with a message to stderr
#
die() {
echo "$*" >&2;
exit 1;
}
