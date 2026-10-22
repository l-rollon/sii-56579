#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>

int main(int argc, char **argv) {
    int fd;
    char buffer[1024];

    // borrar FIFO por si existiera previamente
    unlink("/tmp/tuberia");

    /* crear el FIFO */
    if (mkfifo("/tmp/tuberia",0666)<0) {
        perror("No puede crearse el FIFO");
        return(1);
    }
    /* Abrir el FIFO */
    fd=open("/tmp/tuberia",O_RDONLY);
    if (fd<0) {
        perror("No puede abrirse el FIFO");
        return(1);
    }
    /* Leer de la FIFO */
    ssize_t bytesRead; 
    while ((bytesRead=read(fd,buffer,sizeof(buffer)-1))>0) {
	// X5: Imprimir el texto en la pantalla
	buffer[bytesRead]='\0';
	printf("%s",buffer);
	fflush(stdout);
    }
    close(fd);
    unlink("/tmp/tuberia");
    return(0);
}
