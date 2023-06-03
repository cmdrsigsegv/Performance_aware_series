#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define VERBOSE 1

struct mov_s{
    unsigned char W : 1;
    unsigned char D : 1;
    unsigned char RM : 3;
    unsigned char REG : 3;
    unsigned char MOD : 2;
};

FILE *openbfile( char* filename );
void disassemble( char* filename );
char *add_extension( char *filename );
FILE *createfile( char* filename );
void print_binary( FILE *fp );
void decode_mov( unsigned char buffer, FILE *fp, FILE *fp_asm );

const char *reg_table[2][8] = {
    {"al", "cl", "dl", "bl", "ah", "ch", "dh", "bh"},   // W = 0
    {"ax", "cx", "dx", "bx", "sp", "bp", "si", "di"}    // W = 1
};

int main( int argc, char *argv[] ){
    // Any other number than 2 for the number of input parameters would not be the correct usage
    if (argc != 2){
        fprintf(stderr,"Usage /path/to/diss <bitstream>\n");
        return EXIT_FAILURE;
    }

    disassemble( argv[1] );

    return EXIT_SUCCESS;
}

void disassemble( char *filename ){
    // Check if we can open the file
    FILE *fp = openbfile( filename );

    // Create the disassembly file
    char* filename_asm = add_extension(filename);
    FILE *fp_asm = createfile( filename_asm );

#if VERBOSE
    // This isn't needed for the disassembly, just to quickly look at what is being decoded.
    print_binary(fp);

    if (fseek(fp, 0, SEEK_SET) != 0) {
        perror("fseek");
        exit(EXIT_FAILURE);
    }
#endif

    unsigned char buffer;
    unsigned char opcode;

    while (fread(&buffer, 1, 1, fp) > 0){
        opcode = buffer>>2;
        switch (opcode){
            case 0x22:
                decode_mov( buffer, fp, fp_asm );
                break;
            
            default:
                break;
        }
    }

    if (fclose(fp) == EOF){
        perror(filename);
        exit(EXIT_FAILURE);
    }
    if (fclose(fp_asm) == EOF){
        perror(filename_asm);
        exit(EXIT_FAILURE);
    }
    free(filename_asm);
}

FILE *openbfile( char *filename ){
    FILE *fp = fopen(filename,"rb");
    if ferror(fp){
        perror( filename );
        exit(EXIT_FAILURE);
    }
    return fp;
}


char *add_extension( char *filename ){
    int length = strlen(filename) + 4 + 1;
    char *newfilename = malloc(length);

    if ( newfilename == NULL ){
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    strcpy(newfilename, filename);
    strcat(newfilename, ".asm");

    return newfilename;
}

FILE *createfile( char* filename ){
    FILE *fp = fopen(filename,"w");
    if ferror(fp){
        perror( filename );
        exit(EXIT_FAILURE);
    }
    fprintf(fp, "bits 16\n\n");
    return fp;
}

void print_binary(FILE *fp){
    unsigned char buffer;
    size_t bytes_read;
    int byte_counter = 0;

    while ((bytes_read = fread(&buffer, 1, 1, fp)) > 0){
        for (int i = 7; i >= 0; i--){
            printf("%d", (buffer >> i) & 1);
        }
        printf(" ");
        byte_counter++;

        if (byte_counter == 2){
            printf("\n");
            byte_counter = 0;
        }
    }

    if (ferror(fp)){
        perror("fread");
    }
}

void decode_mov( unsigned char buffer, FILE *fp, FILE *fp_asm ){
    fprintf( fp_asm, "mov ");
    struct mov_s instruction;
    instruction.D = buffer & (1<<1);
    instruction.W = buffer & (1<<0);

    fread(&buffer, 1, 1, fp);

    instruction.MOD = (buffer & 0xc0) >> 6;
    instruction.REG = (buffer & 0x38) >> 3;
    instruction.RM = buffer & 0x7;

    switch (instruction.MOD){
        case 0b00:  // Memory mode, no displacement.
            break;
        case 0b01:  // Memory mode, 8-bit displacement follows.
            break;
        case 0b10:  // Memory mode, 16-bit displacement follows.
            break;
        case 0b11:  // Register mode, no displacement.
            const char *regname1 = reg_table[instruction.W][instruction.REG];
            const char *regname2 = reg_table[instruction.W][instruction.RM];
            fprintf( fp_asm, "%s, %s\n", instruction.D ? regname1 : regname2, instruction.D ? regname2 : regname1);
            break;
        
        default:
            break;
    }
#if VERBOSE
    printf("MOV instruction found with:\n");
    printf("\tW=%d, D=%d, MOD=%d, REG=%d, RM=%d\n",
        instruction.W,
        instruction.D,
        instruction.MOD,
        instruction.REG,
        instruction.RM
    );
#endif
}