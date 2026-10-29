#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <string.h>

#define VELOCIDAD B9600

int main(void){
  int fda=open("/dev/ttyACM1", O_RDWR | O_NOCTTY);

  if (fda == -1){
    printf ("Error al abrir archivo \n");
    return -1;
  }

  struct termios config_inicial;
  struct termios config_nueva;
  char op;

  tcgetattr(fda, &config_inicial);

  config_nueva = config_inicial;

  cfsetispeed(&config_nueva, VELOCIDAD);
  cfsetospeed(&config_nueva, VELOCIDAD);

  /* Formato 8N1 */
  config_nueva.c_cflag &= ~CSIZE;
  config_nueva.c_cflag |= CS8;
  config_nueva.c_cflag &= ~(PARENB | PARODD);
  config_nueva.c_cflag &= ~CSTOPB;
  config_nueva.c_cflag |= (CREAD | CLOCAL);

  /* Modo no canónico y sin procesar caracteres */
  config_nueva.c_lflag = 0;
  config_nueva.c_oflag = 0;
  config_nueva.c_iflag &= ~(IXON | IXOFF | IXANY);

  /* Configuración de bloqueo para lectura serie (espera al menos 1 byte) */
  config_nueva.c_cc[VMIN] = 1;
  config_nueva.c_cc[VTIME] = 0;
  tcsetattr(fda, TCSANOW, &config_nueva);


  const char *codigo_valido = "6347B219"; 

  char buffer_uid[64];
  int idx = 0;
  char c, d;
  char res;

  while (1) {
    idx = 0;

    /* Lee byte a byte hasta encontrar salto de línea */
    while (read(fda, &c, 1) > 0) {
      if (c == '\n' || c == '\r') {
        if (idx > 0) {
          break; // Fin de trama
        }
      } else if (idx < (int)sizeof(buffer_uid) - 1) {
        buffer_uid[idx++] = c;
      }
    }


    buffer_uid[idx] = '\0';

    if (idx == 0) {
      continue;
    }

    printf("UID recibido: [%s]\n", buffer_uid);

    /* Comparación */
    if (strcmp(buffer_uid, codigo_valido) == 0) {
      res = '1';
      printf("-> Valida: enviando 1\n");
    } else {
      res = '0';
      printf("-> Invalida: enviando 0\n");
    }

    /* Envío de respuesta al Arduino */
    write(fda, &res, 1);
    write(fda, "\n", 1);
  }

  sleep(2);

  tcsetattr(fda, TCSANOW, &config_inicial);
  close(fda);
  
  return 0;
}
