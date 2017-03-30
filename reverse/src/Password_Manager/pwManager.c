#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <time.h>
#include <string.h>

#include <sys/ptrace.h>

#include <openssl/md5.h>

const unsigned int TIME_WAIT = 20000;

void usage( char argv[] )
{
    printf("Usage:\n%s <KEY>\n",argv);
}

// Obtenir la clé pour décrypter le password
char* getKey()
{
    // Initialisation de la clé
    char* key = NULL;

    // Obtention du hostname
    char hostname[1024];
    hostname[1023] = '\0';
    gethostname(hostname, 1023);

    char authorizedPC[] = "Edouard-PC";
    unsigned char c[MD5_DIGEST_LENGTH];
    MD5((const unsigned char *)authorizedPC,sizeof(authorizedPC)-1,c);

    // Patch de solution à retirer pour la version finale
    //memcpy(hostname,authorizedPC,11);


    if( !strncmp(authorizedPC,hostname,sizeof(hostname)) )
    {
        // Mettre la clé (hostname) sur le heap
        key = malloc(MD5_DIGEST_LENGTH);
        memcpy(key,c,MD5_DIGEST_LENGTH);
    }

    // Retourne un pointeur sur la clé
    return key;
}

// Affiche les mots de passe dans stdout
void printPasswords( char* key, unsigned int startTime )
{
    // Tableau temporaire de mots de passe
    int encryptedPass[] = {4294967245,116,4294967220,4294967243,4294967228,4294967286,102,4294967196,4294967176,4294967268,19};
    char cheater[11] = "-\nCheater";
    char tmp = '\00';

    unsigned int timeNow = time(0)-startTime;
    unsigned int timeHint = timeNow - (timeNow%TIME_WAIT);

    printf("[+] Mot de passe instagram: ");
    // XOR byte par byte
    for(int i = 0; i < sizeof(encryptedPass)/sizeof(encryptedPass[0]);i++)
    {
        if( time(0) - startTime >= TIME_WAIT )
            tmp = key[i] ^ (TIME_WAIT-timeHint) ^encryptedPass[i];
        else
            tmp = key[i] ^ cheater[i] ^ key[i];
        printf("%c",tmp);
    }
    printf("\n");
}

int main(int argc, char *argv[])
{
    if( argc != 2 )
    {
        usage(argv[0]);
        return 1;
    }

    // Compare le mot de passe donné avec le mot de passe static
    char easyStart[] = "cs{Great_First_Step}";
    if(strncmp(easyStart,argv[1],sizeof(easyStart)))
    {
        printf("[-] Wrong Password\n");
        return 1;
    }
    
    unsigned int* traceRes = (void *)ptrace(PTRACE_TRACEME,0,0);
    printf("[+] Correct Password\n");

    printf("[+] Verifying environment ...\n");
    char* key = getKey();
    traceRes = (void *)((uintptr_t)traceRes ^ (uintptr_t)key);
    traceRes = (void *)((uintptr_t)traceRes & (uintptr_t)key);
    
    unsigned int startTime = time(0);

    if( traceRes )
    {
        printf("[+] This is an authorized environment\n");
        printf("[+] Decrypting password vault\n");
        unsigned int retTime = time(0) + TIME_WAIT;
        unsigned int timeLeft = retTime-time(0);

        while (time(0) <= retTime)
        {
            printf("[+] %i seconds left\r",timeLeft);
            timeLeft = retTime-time(0);
        }
        printPasswords( (char*)traceRes, startTime );
    }
    else
    {
        printf("[-] This is an UNAUTHORIZED environment\n");
        return 1;
    }
    return 0;
}
