// ktest_corpus_generator.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

// Function to write a KTest to a file
void write_ktest(KTest *test, const char *filename) {
  FILE *f = fopen(filename, "wb");
  if (!f) {
    perror("Failed to open file");
    return;
  }
  
  // Write version
  fwrite(&test->version, sizeof(unsigned), 1, f);
  
  // Write numArgs
  fwrite(&test->numArgs, sizeof(unsigned), 1, f);
  
  // Write args
  for (unsigned i = 0; i < test->numArgs; i++) {
    unsigned len = strlen(test->args[i]) + 1;
    fwrite(&len, sizeof(unsigned), 1, f);
    fwrite(test->args[i], 1, len, f);
  }
  
  // Write symArgvs and symArgvLen
  fwrite(&test->symArgvs, sizeof(unsigned), 1, f);
  fwrite(&test->symArgvLen, sizeof(unsigned), 1, f);
  
  // Write numObjects
  fwrite(&test->numObjects, sizeof(unsigned), 1, f);
  
  // Write objects
  for (unsigned i = 0; i < test->numObjects; i++) {
    // Write name
    unsigned name_len = strlen(test->objects[i].name) + 1;
    fwrite(&name_len, sizeof(unsigned), 1, f);
    fwrite(test->objects[i].name, 1, name_len, f);
    
    // Write numBytes and bytes
    fwrite(&test->objects[i].numBytes, sizeof(unsigned), 1, f);
    fwrite(test->objects[i].bytes, 1, test->objects[i].numBytes, f);
  }
  
  fclose(f);
  printf("Created %s\n", filename);
}

// Helper to create a simple object
KTestObject create_object(const char *name, unsigned numBytes, unsigned char *data) {
  KTestObject obj;
  obj.name = strdup(name);
  obj.numBytes = numBytes;
  obj.bytes = (unsigned char *)malloc(numBytes);
  memcpy(obj.bytes, data, numBytes);
  return obj;
}

// Helper to free KTest resources
void free_ktest(KTest *test) {
  for (unsigned i = 0; i < test->numArgs; i++) {
    free(test->args[i]);
  }
  free(test->args);
  
  for (unsigned i = 0; i < test->numObjects; i++) {
    free(test->objects[i].name);
    free(test->objects[i].bytes);
  }
  free(test->objects);
}

// Create corpus file 1: Basic valid file with 3 objects (should trigger the bug)
void create_corpus_file1() {
  KTest test;
  test.version = 1;
  test.numArgs = 0;
  test.args = NULL;
  test.symArgvs = 0;
  test.symArgvLen = 0;
  test.numObjects = 3;
  
  test.objects = (KTestObject *)malloc(3 * sizeof(KTestObject));
  
  // Create objects that sum to numBytes = 10, so res-7 = 3 and 100/(res-7) = 33.3
  unsigned char data1[] = {1, 2};
  unsigned char data2[] = {3, 4, 5};
  unsigned char data3[] = {6, 7, 8, 9, 10};
  
  test.objects[0] = create_object("object1", 2, data1);
  test.objects[1] = create_object("object2", 3, data2);
  test.objects[2] = create_object("object3", 5, data3);
  
  write_ktest(&test, "corpus_basic.ktest");
  free_ktest(&test);
}

// Create corpus file 2: Empty file (no objects)
void create_corpus_file2() {
  KTest test;
  test.version = 1;
  test.numArgs = 0;
  test.args = NULL;
  test.symArgvs = 0;
  test.symArgvLen = 0;
  test.numObjects = 0;
  test.objects = NULL;
  
  write_ktest(&test, "corpus_empty.ktest");
  free_ktest(&test);
}

// Create corpus file 3: File with exactly 3 objects that trigger division by 0
void create_corpus_file3() {
  KTest test;
  test.version = 1;
  test.numArgs = 0;
  test.args = NULL;
  test.symArgvs = 0;
  test.symArgvLen = 0;
  test.numObjects = 3;
  
  test.objects = (KTestObject *)malloc(3 * sizeof(KTestObject));
  
  // Create 3 objects with total numBytes = 7, so res-7 = 0 (division by zero)
  unsigned char data1[] = {1, 2};
  unsigned char data2[] = {3, 4};
  unsigned char data3[] = {5, 6, 7};
  
  test.objects[0] = create_object("object1", 2, data1);
  test.objects[1] = create_object("object2", 2, data2);
  test.objects[2] = create_object("object3", 3, data3);
  
  write_ktest(&test, "corpus_div_zero.ktest");
  free_ktest(&test);
}

// Create corpus file 4: File with 3 objects that result in 100/(res-7) == 3
void create_corpus_file4() {
  KTest test;
  test.version = 1;
  test.numArgs = 0;
  test.args = NULL;
  test.symArgvs = 0;
  test.symArgvLen = 0;
  test.numObjects = 3;
  
  test.objects = (KTestObject *)malloc(3 * sizeof(KTestObject));
  
  // We need res = 40, so 100/(res-7) = 100/33 = 3.03...
  // But since it's integer division, 100/33 = 3
  unsigned char data1[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
  unsigned char data2[] = {11, 12, 13, 14, 15, 16, 17, 18, 19, 20};
  unsigned char data3[] = {21, 22, 23, 24, 25, 26, 27, 28, 29, 30};
  
  test.objects[0] = create_object("object1", 10, data1);
  test.objects[1] = create_object("object2", 10, data2);
  test.objects[2] = create_object("object3", 20, data3);
  
  write_ktest(&test, "corpus_exact_match.ktest");
  free_ktest(&test);
}

// Create corpus file 5: File with command-line arguments
void create_corpus_file5() {
  KTest test;
  test.version = 1;
  test.numArgs = 2;
  test.args = (char **)malloc(2 * sizeof(char *));
  test.args[0] = strdup("./program");
  test.args[1] = strdup("--test");
  test.symArgvs = 0;
  test.symArgvLen = 0;
  test.numObjects = 1;
  
  test.objects = (KTestObject *)malloc(1 * sizeof(KTestObject));
  
  unsigned char data[] = {1, 2, 3, 4, 5};
  test.objects[0] = create_object("object1", 5, data);
  
  write_ktest(&test, "corpus_with_args.ktest");
  free_ktest(&test);
}

int main() {
  // Create directory for corpus files
  system("mkdir -p corpus");
  
  // Change to corpus directory
  chdir("corpus");
  
  // Generate corpus files
  create_corpus_file1();  // Basic valid file
  create_corpus_file2();  // Empty file (no objects)
  create_corpus_file3();  // Division by zero file
  create_corpus_file4();  // Exact match for condition
  create_corpus_file5();  // File with command-line args
  
  printf("All corpus files created in the 'corpus' directory.\n");
  return 0;
}
