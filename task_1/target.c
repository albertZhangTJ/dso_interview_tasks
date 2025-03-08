// fuzz_target.cc
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "KTest.h"

// Helper function to clean up allocated memory
void cleanup_ktest(KTest *test) {
    if (!test) return;
    
    if (test->args) {
      for (unsigned i = 0; i < test->numArgs; i++) {
        free(test->args[i]);
      }
      free(test->args);
    }
    
    if (test->objects) {
      for (unsigned i = 0; i < test->numObjects; i++) {
        free(test->objects[i].name);
        free(test->objects[i].bytes);
      }
      free(test->objects);
    }
  }

// Fuzzer entry point
int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
  // Need minimum amount of data to create a valid KTest
  if (Size < 8) return 0;
  
  KTest test;
  memset(&test, 0, sizeof(test));
  
  // Use the first 4 bytes for version
  test.version = *(unsigned *)Data;
  
  // Use next 4 bytes to determine numObjects (limit to reasonable range)
  unsigned numObjectsRaw = *(unsigned *)(Data + 4);
  test.numObjects = numObjectsRaw % 10;  // Limit to 0-9 objects for safety
  
  if (Size < 8 + test.numObjects * 4) return 0;  // Not enough data
  
  // Allocate memory for objects
  test.objects = (KTestObject *)malloc(test.numObjects * sizeof(KTestObject));
  if (!test.objects) return 0;
  memset(test.objects, 0, test.numObjects * sizeof(KTestObject));
  
  size_t offset = 8;  // Start after version and numObjects
  
  // Parse each object
  for (unsigned i = 0; i < test.numObjects; i++) {
    // Need at least 4 bytes for numBytes
    if (offset + 4 > Size) {
      cleanup_ktest(&test);
      return 0;
    }
    
    // Get numBytes for this object (limit to reasonable range)
    unsigned numBytesRaw = *(unsigned *)(Data + offset);
    test.objects[i].numBytes = numBytesRaw % 100;  // Limit to 0-99 bytes
    offset += 4;
    
    // Create a name for the object
    char nameBuffer[32];
    snprintf(nameBuffer, sizeof(nameBuffer), "object_%u", i);
    test.objects[i].name = strdup(nameBuffer);
    
    // Allocate and fill bytes if there's enough data
    if (test.objects[i].numBytes > 0) {
      if (offset + test.objects[i].numBytes > Size) {
        // Not enough data left, adjust numBytes
        test.objects[i].numBytes = Size - offset;
      }
      
      if (test.objects[i].numBytes > 0) {
        test.objects[i].bytes = (unsigned char *)malloc(test.objects[i].numBytes);
        if (test.objects[i].bytes) {
          memcpy(test.objects[i].bytes, Data + offset, test.objects[i].numBytes);
          offset += test.objects[i].numBytes;
        } else {
          test.objects[i].numBytes = 0;  // Couldn't allocate, so adjust
        }
      }
    }
  }
  
  
  kTest_bug(&test);
  
  
  // Clean up
  cleanup_ktest(&test);
  
  return 0;  // Return 0 to continue fuzzing
}