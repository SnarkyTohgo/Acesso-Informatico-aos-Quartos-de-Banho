#!/bin/sh
# $? = 0 se compilou bem
# $? = 2 otherwise
make
if [ $? -eq 0 ] ; then
  sh test.sh -q 10 -u 10 -l 5 -n 100 -f fifo.server | tee -a result.log
  echo "----------------------------"
  make clean
else
  echo "MAKE ERROR";
fi