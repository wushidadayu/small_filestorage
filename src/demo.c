#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "fcgi_stdio.h"

int main(int argc, char **argv)
{
	int count = 0;
	while(FCGI_Accept() >= 0)
	{
		printf("Contect-type: text/html\r\n");
		printf("\r\n");
		printf("<title> Fast CGI hello!</title>");
		printf("<h1>Fast CGI hello!</h1>");
		printf("number %d running on host <i>%s</i>\n", ++count, getenv("SERVER_NAME"));
	}
	return 0;
}
