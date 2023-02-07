#!/bin/bash

#This shell script is ran by the startup.service everytime a vm is created.
#Its role is to run our test application postgresql on the vm, run the make check command
#and return the logs of the make check command back to the dom 0 
#permitting us know if the test was successfull


cd /home/postgres/postgresql-14.0

su - postgres

cd /home/postgres/postgresql-14.0


/usr/local/pgsql/bin/pg_ctl -D /usr/local/pgsql/data -l logfile start


make check > logfile.txt


scp /home/postgres/postgresql-14.0/logfile.txt joe@192.168.106.193:/home/joe/code