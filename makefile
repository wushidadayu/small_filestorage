
curdir = .

flincr = $(curdir)/incr
fllib = $(curdir)/lib
flsrc = $(curdir)/src
flutil = $(curdir)/src/util

CC = gcc

util_upload = $(flutil)/make_log.c  $(flutil)/redis_op.c
util_data = $(flutil)/cJSON.c  $(flutil)/make_log.c  $(flutil)/redis_op.c


echo:$(flsrc)/echo.c
	$(CC) $(flsrc)/echo.c -o echo -lfcgi
	spawn-fcgi -a 127.0.0.1 -p 8083 -f ./echo

demo:$(flsrc)/demo.c
	$(CC) $(flsrc)/demo.c -o demo -lfcgi
	spawn-fcgi -a 127.0.0.1 -p 8082 -f ./demo

cloud-upload:$(flsrc)/cloud_upload.c $(util_upload)
	$(CC) $(flsrc)/cloud_upload.c $(util_upload) -o cloud_upload -lfcgi -lhiredis -lmyfastdfs -I$(flincr)/
	spawn-fcgi -a 127.0.0.1 -p 8085 -f ./cloud_upload

cloud-data:$(flsrc)/cloud_data.c $(util_data)
	$(CC) $(flsrc)/cloud_data.c $(util_data) -o cloud_data -lfcgi -lhiredis -lm -I$(flincr)/
	spawn-fcgi -a 127.0.0.1 -p 8086 -f ./cloud_data
