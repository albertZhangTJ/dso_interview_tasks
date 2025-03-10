// fuzz_ktest.cpp
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <setjmp.h>
#include "KTest.h"



static jmp_buf jump_buffer;

void handle_abort(int sig) {
    longjmp(jump_buffer, 1);
}

// The target function declaration - we'll import this from KTest.h in a real scenario
// but keep it here for completeness
extern unsigned kTest_bug(KTest *bo);

// Fuzzer entry point
int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
  if (Size == 0) return 0;
  
  // Create a temporary file to write the fuzzer data
  char tmpfilename[] = "/tmp/libfuzzer-ktest-XXXXXX";
  int fd = mkstemp(tmpfilename);
  if (fd < 0) return 0;
  
  // Write fuzzer data to the temporary file
  if (write(fd, Data, Size) != (ssize_t)Size) {
    close(fd);
    unlink(tmpfilename);
    return 0;
  }
  
  close(fd);
  
   // Try to recover from potential invalid free
  if (setjmp(jump_buffer) == 0) {
    // Use the provided function to parse the file into a KTest object
    KTest *test = kTest_fromFile(tmpfilename);

    // If we couldn't parse the file, just return
    if (!test) {
      unlink(tmpfilename);
      return 0;
    }
  
    // Call the target function with the parsed KTest object
    unsigned result = 0;
    result = kTest_bug(test);
  
    // Clean up using the provided cleanup function
    kTest_free(test);
  } else {
    printf("Recovered from invalid free in function f()\n");
  }
  
  // Remove the temporary file as soon as we've read it
  unlink(tmpfilename);
  
  
  return 0;  // Return 0 to continue fuzzing
}
