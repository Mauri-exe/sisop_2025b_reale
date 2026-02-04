#ifndef NARGS
#define NARGS 4
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

void
ejecutar_comando(char *comando, char *args[], int nargs)
{
	pid_t pid = fork();
	if (pid == 0) {
		// Proceso Hijo: armo un array para execvp
		char **argv = malloc((nargs + 2) *
		                     sizeof(char *));  // +2 para comando y NULL
		if (argv == NULL) {
			perror("malloc");
			_exit(1);
		}
		argv[0] = comando;
		for (int i = 0; i < nargs; i++) {
			argv[i + 1] = args[i];
		}
		argv[nargs + 1] = NULL;  // NULL al final

		execvp(comando, argv);
		perror("execvp");
		free(argv);
		_exit(1);  // Uso de _exit en lugar de exit para sí poder evitar problemas de limpieza
	} else if (pid > 0) {
		// Proceso Padre: espera al hijo
		wait(NULL);
	} else {
		// Error al crear el proceso
		perror("fork");
		exit(1);
	}
}

int
main(int argc, char *argv[])
{
	if (argc < 2) {
		// Mensaje de error
		fprintf(stderr, "Uso: %s <comando>\n", argv[0]);
	} else {
		char *comando = argv[1];
		char *args[NARGS];  // Array para los argumentos
		int contador = 0;

		char *leer_lineas = NULL;
		size_t longitud = 0;

		// Leo línea por línea
		ssize_t linea_leida = getline(&leer_lineas, &longitud, stdin);
		while (linea_leida != -1) {
			leer_lineas[strcspn(leer_lineas, "\n")] =
			        '\0';  // quito '\n'
			args[contador++] =
			        strdup(leer_lineas);  // guardo una copia
			// En caso de que falle el strdup
			if (args[contador - 1] == NULL) {
				perror("strdup");
				exit(1);
			}

			if (contador == NARGS) {
				ejecutar_comando(comando, args, contador);
				// libero memoria
				for (int i = 0; i < contador; i++)
					free(args[i]);

				contador = 0;
			}
			linea_leida = getline(&leer_lineas, &longitud, stdin);
		}
		// Ejecuto lo que quedó pendiente
		if (contador > 0) {
			ejecutar_comando(comando, args, contador);
			for (int i = 0; i < contador; i++)
				free(args[i]);
		}
		free(leer_lineas);
	}

	return 0;
}
