  /* test1.c */
  #include <stdio.h>
  #include <keystone/keystone.h>
  #include <sys/mman.h>
  
  // separate assembly instructions by ; or \n
const char * CODE=
	"pushq $0x000A303532\n"
	"movq $0x7363206F6C6C6548,%rsi\n"
	"push %rsi\n"
        "before:\n"
	"movq $1, %rax\n"
        "movq $1, %rdi\n"
        "movq %rsp, %rsi\n"
        "movq $12, %rdx\n"
        "syscall\n"
        "jmp before\n"
        "movq $60, %rax\n"
        "movq $0, %rdi\n"
        "syscall\n"
        "ret\n"
	;

  int main(int argc, char **argv)
  {
      ks_engine *ks;
      ks_err err;
      size_t count;
      unsigned char *encode=(unsigned char*)malloc(200);
      size_t size;
  
      err = ks_open(KS_ARCH_X86, KS_MODE_64, &ks);
      if (err != KS_ERR_OK) {
          printf("ERROR: failed on ks_open(), quit\n");
          return -1;
      }

      ks_option(ks, KS_OPT_SYNTAX, KS_OPT_SYNTAX_ATT);

      if (ks_asm(ks, CODE, (unsigned long int) encode, &encode, &size, &count) != KS_ERR_OK) {
          printf("ERROR: ks_asm() failed & count = %lu, error = %u\n",
		         count, ks_errno(ks));
      } else {
          size_t i;
  
          printf("%s = ", CODE);
          for (i = 0; i < size; i++) {
              printf("%02x ", encode[i]);
          }
          printf("\n");
          printf("Compiled: %lu bytes, statements: %lu\n", size, count);
      }
  
      //call
      //mprotect( (void*) ((unsigned long)encode&~4095), 4096*2, PROT_READ|PROT_WRITE|PROT_EXEC);
      void * (*fptr)() = (void * (*)())&encode[0]; 
      (*fptr)();
  
      // NOTE: free encode after usage to avoid leaking memory
      ks_free(encode);
  
      // close Keystone instance when done
      ks_close(ks);

      return 0;
  }

