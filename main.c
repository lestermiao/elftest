#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include <sys/mman.h>


#include <sys/types.h>
#include <unistd.h>

// Format of an ELF executable file

#define ELF_MAGIC 0x464C457FU  // "\x7FELF" in little endian

// File header
struct elfhdr {
  unsigned int magic;  // must equal ELF_MAGIC
  unsigned char elf[12];
  unsigned short type;
  unsigned short machine;
  unsigned int version;
  unsigned int entry;
  unsigned int phoff;
  unsigned int shoff;
  unsigned int flags;
  unsigned short ehsize;
  unsigned short phentsize;
  unsigned short phnum;
  unsigned short shentsize;
  unsigned short shnum;
  unsigned short shstrndx;
};

// Program section header
struct proghdr {
  unsigned int type;
  unsigned int off;
  unsigned int vaddr;
  unsigned int paddr;
  unsigned int filesz;
  unsigned int memsz;
  unsigned int flags;
  unsigned int align;
};


struct sectionhdr {
unsigned int sh_name ;
unsigned int sh_type ;
unsigned int sh_flags;
unsigned int  sh_addr;
unsigned int  sh_offset;
unsigned int sh_size;
unsigned int sh_link;
unsigned int sh_info;
unsigned int sh_addralign;
unsigned int  sh_entsize;
};


struct symEntry {
unsigned int  st_name;
unsigned int  st_value;
unsigned int  st_size;
unsigned char st_info;
unsigned char  st_other;
unsigned short  st_shndx;
};

// Values for Proghdr type
#define ELF_PROG_LOAD           1

// Flag bits for Proghdr flags
#define ELF_PROG_FLAG_EXEC      1
#define ELF_PROG_FLAG_WRITE     2
#define ELF_PROG_FLAG_READ      4

char* openElf(){

    long sz = 0;
    char* buf = NULL;
   FILE *fptr;

   if ((fptr = fopen("./elf","rb")) == NULL){
       printf("Error! opening file");

       // Program exits if the file pointer returns NULL.
       return;
   }

   fseek(fptr, 0L, SEEK_END);    
    sz = ftell(fptr);
    buf = (char*)malloc(sz);
    if(buf){
        printf("allocated %d\n", sz);
        fseek(fptr, 0L, SEEK_SET);
        fread(buf, sizeof(char) * sz, 1, fptr); 
    }

   fclose(fptr);   
   return buf;
}

char* createMMap(){
    char * region = NULL;
    size_t pagesize = getpagesize();

  printf("System page size: %zu bytes\n", pagesize);

  region = mmap(
    (void*) (pagesize * (1 << 20)),   // Map from the start of the 2^20th page
    pagesize,                         // for one page length
    PROT_READ|PROT_WRITE|PROT_EXEC,
    MAP_ANON|MAP_PRIVATE,             // to a private block of hardware memory
    0,
    0
  );

  if (region == MAP_FAILED) {
    perror("Could not mmap");
    return NULL;
  }

  return region;
}

void unMMap(char* region){
    int unmap_result = munmap(region, 1 << 12);
  if (unmap_result != 0) {
    perror("Could not munmap");
    return ;
  }
}

int getMe(int x){
    return x + 10;
}

int findFunc(char* fname, char* elfBuf){    
    struct elfhdr *pelf = (struct elfhdr*)elfBuf;
    struct sectionhdr *pph = (struct sectionhdr *) (elfBuf + pelf->shoff);
    //0x2 sym
    struct sectionhdr *pphsym = NULL;
    //0x3 str
    struct sectionhdr *pphstr = NULL;
    struct symEntry* ppsymentry = NULL;
    char * strtabval = NULL;
    char * ret = NULL;

    for(int i = 0; i< pelf->shnum; i++){
        if(pph->sh_type == 0x2)
            pphsym = pph;
        else if(pph->sh_type == 0x3 && pphstr == NULL)
            pphstr = pph;

        pph += 1;
    }

    if(pphsym != NULL && pphstr != NULL){
        printf("found section header\n");
        int x = pphsym->sh_size / (sizeof(struct symEntry ));
    
        strtabval = (char*)(elfBuf + pphstr->sh_offset);
        ppsymentry = (struct symEntry*) (elfBuf + pphsym->sh_offset);
        
        for(int i = 0; i<x; i++){
            if( ppsymentry->st_name != 0 ){
                if(strcmp(fname, strtabval + ppsymentry->st_name) == 0){
                    printf("found function %s\n", fname);

                    return ppsymentry->st_value;

                }
            }

            ppsymentry++;
        }
    }

    return -1;
}



int main(int argc, char* argv[]) {
    struct elfhdr *pelf;
    struct proghdr *pph;
    int (*sum)(int a, int b);
    void *entry = NULL;
    int ret; 
    char* mmRegion = NULL;
    int funOff = NULL;
    void* pgetMe =NULL;
    unsigned int xval = 0;

    int (*foo) (int);

    char* buf = openElf();
    pelf = (struct elfhdr*)buf;

    printf("%s\n", pelf->elf);
    printf("version %d\n", pelf->version);
    printf("entry %x\n", pelf->entry);
    printf("pheader %d\n", pelf->phoff);
    printf("section header %d\n", pelf->shoff);

    mmRegion = createMMap();

    pph = (struct proghdr *) (buf + pelf->phoff);
    printf("prog header off %d\n", pph->off);
    printf("prog header filesz %d\n", pph->filesz);
    printf("prog header memsz %d\n", pph->memsz);
    printf("prog header align %d\n", pph->align);

    memcpy(mmRegion, (buf + pph->off), pph->memsz);

    funOff = findFunc("test", buf);
    entry = (void*) (mmRegion + funOff);

    printf("test at %x\n", funOff);
    funOff = findFunc("getMe", buf);
    printf("getMe at %x\n", getMe);
    printf("main at %x\n", main);

    foo = getMe;
    foo(3);
    xval = (int)getMe;
    printf("main at %x\n", main);


    /* Add your ELF loading code here */

    if (entry != NULL) {
        sum = entry; 
        ret = sum(13, foo);
        printf("sum:%d\n", ret); 
    };

    unMMap(mmRegion);

}


