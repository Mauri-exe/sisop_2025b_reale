#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

void
filtro(int file_descriptor_izquierdo)
{
	int p;

	// Leo el primer valor del pipe izquierdo
	ssize_t bytes_leidos = read(file_descriptor_izquierdo, &p, sizeof(int));
	if (bytes_leidos <= 0) {
		// No hay más datos
		close(file_descriptor_izquierdo);
	} else {
		printf("primo %d\n", p);

		int file_descriptor_derecha[2];  // Pipe hacia el proceso derecho

		int resultado_pipe = pipe(
		        file_descriptor_derecha);  // Creo el pipe hacia el proceso derecho

		if (resultado_pipe < 0) {
			perror("pipe");
			exit(1);  // cierro el proceso hijo porque no se pudo crear
		}

		pid_t pid = fork();  // Creo el proceso hijo

		if (pid < 0) {
			perror("fork");
			exit(1);  // cierro el proceso hijo porque no se pudo crear
		} else if (pid == 0) {
			// Proceso hijo: se convierte en el siguiente filtro
			close(file_descriptor_derecha[1]);  // Cierro la escritura porque no me sirve en este caso
			close(file_descriptor_izquierdo);  // Cierro la lectura
			                                   // del pipe izquierdo porque no lo necesito (el hijo no la usa)
			filtro(file_descriptor_derecha[0]);  // Recursiva porque
			                                     // el hijo se convierte en el siguiente filtro
			exit(0);  // cierro el proceso hijo porque ya no lo necesito
		} else {
			// Proceso padre
			int n;
			close(file_descriptor_derecha[0]);  // cierro la lectura, no la necesito
			while (read(file_descriptor_izquierdo, &n, sizeof(int)) >
			       0) {
				if (n % p != 0) {
					int bytes_escritos =
					        write(file_descriptor_derecha[1],
					              &n,
					              sizeof(int));
					if (bytes_escritos < 0) {
						perror("write");
						exit(1);  // cierro el proceso hijo porque no se pudo crear
					}
				}
			}
			close(file_descriptor_izquierdo);
			close(file_descriptor_derecha[1]);
			wait(NULL);  // espero al hijo
		}
	}
}

int
main(int argc, char *argv[])
{
	if (argc != 2) {
		// Mensaje de error
		fprintf(stderr, "Usó: %s <n>\n", argv[0]);
	} else {
		int n = atoi(argv[1]);  // Convierte el argumento "n" a entero

		if (n < 2) {
			// Mensaje de error
			fprintf(stderr, "n debe ser mayor o igual a 2\n");
		} else {
			int file_descriptors[2];  // Pipe para la comunicación entre procesos
			int resultado_pipe =
			        pipe(file_descriptors);  // Creo el pipe
			if (resultado_pipe < 0) {
				perror("pipe");
				exit(1);
			}
			pid_t pid = fork();  // Creo el primer filtro
			if (pid < 0) {
				perror("fork");
				exit(1);  // Error al crear el proceso y termina el programa/proceso
			} else if (pid == 0) {
				// Proceso hijo
				close(file_descriptors[1]);  // cierro escritura (no lo necesito)
				filtro(file_descriptors[0]);
				exit(0);  // Salgo del proceso hijo indicando que todo está bien
			} else {
				// Proceso Padre
				close(file_descriptors[0]);  // cierro lectura (no lo necesito)

				for (int i = 2; i <= n; i++) {
					int bytes_escritos =
					        write(file_descriptors[1],
					              &i,
					              sizeof(int));
					if (bytes_escritos < 0) {
						perror("write");
						exit(1);  // cierro el proceso padre porque no se pudo crear
					}
				}

				close(file_descriptors[1]);  // cierro escritura (Ya no lo necesito)
				wait(NULL);  // espero a que termine el primer filtro
			}
		}
	}
	return 0;
}