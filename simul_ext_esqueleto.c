#include<stdio.h>
#include<string.h>
#include<ctype.h>
#include "cabeceras.h"

#define LONGITUD_COMANDO 100

void Printbytemaps(EXT_BYTE_MAPS *ext_bytemaps);
int ComprobarComando(char *strcomando, char *orden, char *argumento1, char *argumento2);
void LeeSuperBloque(EXT_SIMPLE_SUPERBLOCK *psup);
int BuscaFich(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, char *nombre);
void Directorio(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos);
int Renombrar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos,  char *nombreantiguo, char *nombrenuevo);
int Imprimir(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, EXT_DATOS *memdatos, char *nombre);
int Borrar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, EXT_BYTE_MAPS *ext_bytemaps, EXT_SIMPLE_SUPERBLOCK *ext_superblock, char *nombre,  FILE *fich);
int Copiar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, EXT_BYTE_MAPS *ext_bytemaps, EXT_SIMPLE_SUPERBLOCK *ext_superblock, EXT_DATOS *memdatos, char *nombreorigen, char *nombredestino,  FILE *fich);
void Grabarinodosydirectorio(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, FILE *fich);
void GrabarByteMaps(EXT_BYTE_MAPS *ext_bytemaps, FILE *fich);
void GrabarSuperBloque(EXT_SIMPLE_SUPERBLOCK *ext_superblock, FILE *fich);
void GrabarDatos(EXT_DATOS *memdatos, FILE *fich);

int main(int argc , char** argv) {
	char comando[LONGITUD_COMANDO];
	char orden[LONGITUD_COMANDO];
	char argumento1[LONGITUD_COMANDO];
	char argumento2[LONGITUD_COMANDO];
	 
	int i,j;
	unsigned long int m;
	EXT_SIMPLE_SUPERBLOCK ext_superblock;
	EXT_BYTE_MAPS ext_bytemaps;
	EXT_BLQ_INODOS ext_blq_inodos;
    EXT_ENTRADA_DIR directorio[MAX_FICHEROS];
    EXT_DATOS memdatos[MAX_BLOQUES_DATOS];
    EXT_DATOS datosfich[MAX_BLOQUES_PARTICION];
    int entradadir;
    int grabardatos;
    FILE *fent;
	
    // Lectura del fichero completo de una sola vez
    fent = fopen("particion.bin","r+b");
    fread(&datosfich, SIZE_BLOQUE, MAX_BLOQUES_PARTICION, fent);     
    memcpy(&ext_superblock,(EXT_SIMPLE_SUPERBLOCK *)&datosfich[0], SIZE_BLOQUE);
    memcpy(&directorio,(EXT_ENTRADA_DIR *)&datosfich[3], SIZE_BLOQUE);
    memcpy(&ext_bytemaps,(EXT_BLQ_INODOS *)&datosfich[1], SIZE_BLOQUE);
    memcpy(&ext_blq_inodos,(EXT_BLQ_INODOS *)&datosfich[2], SIZE_BLOQUE);
    memcpy(&memdatos,(EXT_DATOS *)&datosfich[4],MAX_BLOQUES_DATOS*SIZE_BLOQUE);
     
    // Bucle de tratamiento de comandos
    for (;;){
		do {
			printf (">> ");
			fflush(stdin);
			fgets(comando, LONGITUD_COMANDO, stdin);
		} while (ComprobarComando(comando,orden,argumento1,argumento2) !=0);
		if (strcmp(orden,"dir")==0) {
			Directorio(&directorio,&ext_blq_inodos);
            continue;
        }
        // Escritura de metadatos en comandos rename, remove, copy     
    	Grabarinodosydirectorio(&directorio,&ext_blq_inodos,fent);
    	GrabarByteMaps(&ext_bytemaps,fent);
    	GrabarSuperBloque(&ext_superblock,fent);
    	if (grabardatos)
			GrabarDatos(&memdatos,fent);
    	grabardatos = 0;
        //Si el comando es salir se habrán escrito todos los metadatos
        //faltan los datos y cerrar
        if (strcmp(orden,"salir")==0){
        	GrabarDatos(&memdatos,fent);
           	fclose(fent);
           	return 0;
        }
    }
}

int ComprobarComando(char* strcomando, char* orden, char* argumento1, char* argumento2){
	int esComandoValido = 0;
	int numArgs = sscanf(strcomando, "%s %s %s", orden, argumento1, argumento2);

	if (strcmp(orden, "info") == 0){
		if (numArgs == 1){
			esComandoValido = 1;
		} else {
			printf("ERROR: Demasiados argumentos\n");
 		}
	} else if (strcmp(orden, "dir") == 0){
		if (numArgs == 1){
			esComandoValido = 1;
		} else {
			printf("ERROR: Demasiados argumentos\n");
		} 
	} else if (strcmp(orden, "rename") == 0) {
		if (numArgs == 3) {
			esComandoValido = 1;
		} else if (numArgs > 3){
			printf("ERROR: Demasiados Argumentos\n");
		} else {
			printf("ERROR: Argumentos Insuficientes\n");
		}
	} else if (strcmp(orden, "imprimir") == 0) {
		if (numArgs == 1) {
			printf("ERROR:  Argumentos Insuficientes\n");
		} else if (numArgs >= 3) {
			printf("ERROR: Demasiados Argumentos\n");
		} else if (numArgs == 2){
			esComandoValido = 1;
		}
	} else if (strcmp(orden, "remove") == 0){
		if (numArgs == 1) {
			printf("ERROR:  Argumentos Insuficientes\n");
		} else if (numArgs >= 3) {
			printf("ERROR: Demasiados Argumentos\n");
		} else if (numArgs == 2){
			esComandoValido = 1;
		}
	} else if (strcmp(orden, "copy") == 0){
		if (numArgs == 3) {
			esComandoValido = 1;
		} else if (numArgs > 3){
			printf("ERROR: Demasiados Argumentos\n");
		} else {
			printf("ERROR: Argumentos Insuficientes\n");
		}
	} else {
		printf("ERROR: Comando %s no existe\n", orden);
	}

	return esComandoValido;
}

void LeeSuperBloque(EXT_SIMPLE_SUPERBLOCK *psup){
	printf("Informacion del Superbloque: ");
	printf("Inodos: %d\n", psup->s_inodes_count);
	printf("Bloques: %d\n", psup->s_blocks_count);
	printf("Bloques Libres: &d\n", psup->s_free_blocks_count);
	printf("Primer Bloque: %d\n", psup->s_first_data_block);
	printf("Tamaño del Bloque: %d\n", psup->s_block_size);
}

void Printbytemaps(EXT_BYTE_MAPS *ext_bytemaps) {
    printf("Bytemap de bloques:\n");
    for (int i = 0; i < 25; i++) {
        printf("%d ", ext_bytemaps->bmap_bloques[i]);
    }
    printf("\nBytemap de inodos:\n");
    for (int i = 0; i < MAX_INODOS; i++) {
        printf("%d ", ext_bytemaps->bmap_inodos[i]);
    }
    printf("\n");
}

void Directorio(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos) {
	for (int i = 0; i < MAX_FICHEROS; i++){
		if (directorio->dir_inodo != NULL_INODO) {
			printf("Nombre: %s, Tamaño: %d, inodo: %d, Bloques: ",
			directorio[i].dir_nfich, 
			inodos->blq_inodos[directorio[i].dir_inodo].size_fichero,
			inodos->blq_inodos[directorio[i].dir_inodo].i_nbloque);
			for (int j = 0; j < MAX_NUMS_BLOQUE_INODO; j++) {
				if (inodos->blq_inodos[directorio[i].dir_inodo].i_nbloque[j] != NULL_BLOQUE) {
					printf("%d ", inodos->blq_inodos[directorio[i].dir_inodo].i_nbloque[j]);
				}
			}
			printf("\n");
		}
	}
}

int BuscaFich(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, char *nombre){
	int ficheroEncontrado = -1;
	for (int i = 0; i < MAX_FICHEROS; i++){
		if (directorio[i].dir_inodo != NULL_INODO) {
			if (strcmp(directorio[i].dir_nfich,nombre) == 0){
				ficheroEncontrado = i;
			}
		}
	}
	return ficheroEncontrado;
}

int Renombrar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos,  char *nombreantiguo, char *nombrenuevo) {
	int exito = 0;
	int posFichero = BuscaFich(directorio,inodos,nombreantiguo);
	if (posFichero != -1){
		if (BuscaFich(directorio,inodos,nombreantiguo) == -1) {
			if (strlen(nombrenuevo) < LEN_NFICH) {
				strcpy(directorio[posFichero].dir_nfich,nombrenuevo);
				directorio[posFichero].dir_nfich[LEN_NFICH - 1] = '\0';	
				exito = 1;
			} else {
				printf("ERROR: Nuevo nombre para el fichero es demasiado largo \n");
			}
			
		} else {
			printf("ERROR: El fichero %s ya existe\n", nombrenuevo);
		}
	} else {
		printf("ERROR: No se ha encontrado el fichero %s\n", nombreantiguo);
	}

	return exito;

}

int Imprimir(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, EXT_DATOS *memdatos, char *nombre) {
	int exito = 0;
	int posFichero = BuscaFich(directorio,inodos,nombre);
	if (posFichero == -1) {
		printf("ERROR: Fichero %s no encontrado\n", nombre);
	} else {
		EXT_SIMPLE_INODE inodo = inodos->blq_inodos[directorio[posFichero].dir_inodo];
		if (inodo.size_fichero == 0){
			printf("ERROR: El fichero %s es vacio\n", nombre);
		} else {
			for (int i = 0; i < MAX_NUMS_BLOQUE_INODO; i++){
				if (inodo.i_nbloque[i]  != NULL_BLOQUE) {
					char* bloque = memdatos[inodo.i_nbloque[i]].dato;
					printf("%s",bloque);
				}
			}
			exito = 1;
		}
	}
	printf("\n");
	return exito;
}

int Borrar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, EXT_BYTE_MAPS *ext_bytemaps, EXT_SIMPLE_SUPERBLOCK *ext_superblock, char *nombre,  FILE *fich) {
	int exito = 0;
	int posFichero = BuscaFich(directorio,inodos,nombre);

	if (posFichero == -1) {
		printf("ERROR: Fichero %s no encontrado\n", nombre);
	} else {
		//Liberacion de bloques
		EXT_SIMPLE_INODE *inodo = &inodos.blq_inodos[directorio[posFichero].dir_inodo];
		for (int i = 0; i < MAX_NUMS_BLOQUE_INODO; i++) {
			if (inodo->i_nbloque[i] != NULL_BLOQUE){
				ext_superblock->s_free_blocks_count++;
				ext_bytemaps->bmap_bloques[inodo->i_nbloque[i]] = 0;
				inodo->i_nbloque[i] = NULL_BLOQUE;
			}
		}

		//Liberar inodo
		inodo->size_fichero = 0;
		ext_superblock->s_free_inodes_count++;
		ext_bytemaps->bmap_inodos[directorio[posFichero].dir_inodo] = 0;
		ext_bytemaps->bmap_bloques[inodo->i_nbloque[i]] = 0;
		//Eliminar entrada. Limpio nombre y inodo
		memset(directorio[posFichero].dir_nfich,0,LEN_NFICH); 
		directorio->dir_inodo = NULL_INODO;

	}


	return exito;
}

int Copiar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, EXT_BYTE_MAPS *ext_bytemaps, EXT_SIMPLE_SUPERBLOCK *ext_superblock, EXT_DATOS *memdatos, char *nombreorigen, char *nombredestino,  FILE *fich){
	int exito = 0;
	return exito;
}

void Grabarinodosydirectorio(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, FILE *fich){
	int escritura = 0;
	//Posiciono el puntero en el bloque 2 para los inodos
	fseek(fich,2*SIZE_BLOQUE,SEEK_SET);

	escritura = fwrite(inodos,SIZE_BLOQUE,1,fich);
	if (escritura != 1) {
		printf("ERROR: No se ha podido escribir la lista de inodos\n");
	}
	//Posiciono el puntero en el bloque 3 para el directorio
	fseek(fich,3*SIZE_BLOQUE,SEEK_SET);

	escritura = fwrite(directorio,SIZE_BLOQUE,1,fich);
	if (escritura != 1) {
		printf("ERROR: No se ha podido escribir el directorio\n");
	}

	fflush(fich);
}

void GrabarByteMaps(EXT_BYTE_MAPS *ext_bytemaps, FILE *fich){
	int escritura = 0;
	
	//Nos posicionamos en el bloque 1
	fseek(fich,SIZE_BLOQUE,SEEK_SET);

	escritura = fwrite(ext_bytemaps, SIZE_BLOQUE,1,fich);
	if (escritura != 1) {
		printf("ERROR: No se ha podido escribir los bytemaps\n")
	}
	fflush(fich);
}

void GrabarSuperBloque(EXT_SIMPLE_SUPERBLOCK *ext_superblock, FILE *fich){
	int escritura = 0;
	//Posicionamos el puntero en el bloque 0
	fseek(fich,0,SEEK_SET);

	escritura = fwrite(ext_superblock, SIZE_BLOQUE,1,fich);
	if (escritura != 1) {
		printf("ERROR: No se ha podido escribir el superbloque\n")
	}
	fflush(fich);

}

void GrabarDatos(EXT_DATOS *memdatos, FILE *fich){
	//Movemos el puntero al inicio del primer bloque de datos
	fseek(fich,PRIM_BLOQUE_DATOS * SIZE_BLOQUE, SEEK_SET);

	//Escribimos los datos
	for (int i = 0; i < MAX_BLOQUES_DATOS; i++) {
		fwrite(&memdatos[i],SIZE_BLOQUE,1,fich);
	}

	fflush(fich);
}