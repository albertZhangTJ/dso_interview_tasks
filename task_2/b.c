// direct_crash_reproducer.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>

// KTest structure definitions
typedef struct KTestObject {
  char *name;
  unsigned numBytes;
  unsigned char *bytes;
} KTestObject;

typedef struct KTest {
  unsigned version;
  unsigned numArgs;
  char **args;
  unsigned symArgvs;
  unsigned symArgvLen;
  unsigned numObjects;
  KTestObject *objects;
} KTest;

// The target function declaration
extern unsigned kTest_bug(KTest *bo);

// Function to read entire file into a buffer
uint8_t* read_file(const char* filename, size_t* size_out) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        perror("Failed to open file");
        *size_out = 0;
        return NULL;
    }
    
    // Get file size
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    // Allocate buffer
    uint8_t* buffer = (uint8_t*)malloc(file_size);
    if (!buffer) {
        perror("Failed to allocate memory");
        fclose(file);
        *size_out = 0;
        return NULL;
    }
    
    // Read file data
    size_t bytes_read = fread(buffer, 1, file_size, file);
    fclose(file);
    
    if (bytes_read != (size_t)file_size) {
        fprintf(stderr, "Failed to read entire file\n");
        free(buffer);
        *size_out = 0;
        return NULL;
    }
    
    *size_out = file_size;
    return buffer;
}

// Free KTest resources
void free_ktest(KTest* test) {
    if (!test) return;
    
    for (unsigned i = 0; i < test->numArgs; i++) {
        free(test->args[i]);
    }
    free(test->args);
    
    for (unsigned i = 0; i < test->numObjects; i++) {
        free(test->objects[i].name);
        free(test->objects[i].bytes);
    }
    free(test->objects);
    free(test);
}

// Print KTest information
void print_ktest_info(KTest* test) {
    printf("KTest information:\n");
    printf("- Version: %u\n", test->version);
    printf("- Number of objects: %u\n", test->numObjects);
    
    // Calculate the sum that would be used in kTest_bug
    unsigned res = 0;
    for (unsigned i = 0; i < test->numObjects; i++) {
        res += test->objects[i].numBytes;
    }
    
    printf("- Total numBytes: %u\n", res);
    
    // Print object details
    printf("\nObjects:\n");
    for (unsigned i = 0; i < test->numObjects; i++) {
        printf("Object %d:\n", i);
        printf("  - Name: %s\n", test->objects[i].name);
        printf("  - Size: %u bytes\n", test->objects[i].numBytes);
        printf("  - Data: ");
        
        // Print up to 32 bytes
        unsigned bytes_to_print = test->objects[i].numBytes < 32 ? test->objects[i].numBytes : 32;
        for (unsigned j = 0; j < bytes_to_print; j++) {
            printf("%02x ", test->objects[i].bytes[j]);
        }
        if (test->objects[i].numBytes > 32) {
            printf("...");
        }
        printf("\n");
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Usage: %s <crash_file>\n", argv[0]);
        printf("Reproduces a crash by directly processing the crash file content\n");
        return 1;
    }
    
    const char* crash_file = argv[1];
    printf("Attempting to reproduce crash with file: %s\n\n", crash_file);
    
    // Read the crash file into memory
    size_t Size = 0;
    uint8_t* Data = read_file(crash_file, &Size);
    
    // Create a KTest structure directly from the data
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
    
    // Print information about the KTest structure
    print_ktest_info(&test);
    
    // Try to reproduce the crash
    printf("\nAttempting to reproduce the crash...\n");
    
    unsigned result = kTest_bug(&test);
    printf("\nFunction call completed without crashing.\n");
    printf("Return value: %u\n", result);
    
    free(Data);
    
    return 0;
}