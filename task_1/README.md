# Task 1
This task is about generating a `LibFuzzer` fuzz target with the help of LLMs. 
The target function is `kTest_bug`, which contains a division-by-zero bug.

## Interaction with LLM
Claude 3.7 Sonnet is used in this task.
Only the most relevant interactions are shown here for the sake of readability.

Prompt
```
As a security engineer, your task today is to implement a fuzz target to be used with LibFuzzer like this example
// fuzz_target.cc
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
  DoSomethingInterestingWithMyAPI(Data, Size);
  return 0;  // Values other than 0 and -1 are reserved for future use.
}


Your target function is 
unsigned kTest_bug(KTest *bo) {
  unsigned i, res = 0;
  for (i=0; i<bo->numObjects; i++)
    res += bo->objects[i].numBytes;
  return (bo->numObjects != 3) || (100 / (res - 7) != 3);
}


And the struct KTest is defined as follows
typedef struct KTestObject KTestObject;
  struct KTestObject {
    char *name;
    unsigned numBytes;
    unsigned char *bytes;
  };
  
  typedef struct KTest KTest;
  struct KTest {
    /* file format version */
    unsigned version; 
    
    unsigned numArgs;
    char **args;

    unsigned symArgvs;
    unsigned symArgvLen;

    unsigned numObjects;
    KTestObject *objects;
  };
```

Response
```
I'll implement a fuzz target for the `kTest_bug` function that can be used with LibFuzzer. The goal is to parse the fuzzer-provided data into a valid `KTest` structure that will be fed to the target function.

<See ktest-fuzz-target.txt>


This implementation creates a fuzzer target for the `kTest_bug` function. Here's how it works:

1. **Parsing Input Data**: The fuzzer takes the random input data provided by LibFuzzer and uses it to build a `KTest` structure:
   - The first 4 bytes become the `version` field
   - The next 4 bytes determine how many objects to create (limited to 0-9 for safety)
   - For each object, it reads the `numBytes` value and creates corresponding byte arrays

2. **Safety Considerations**:
   - I included bounds checking to ensure we don't read past the available data
   - Object counts and sizes are limited to reasonable ranges to prevent excessive memory usage
   - All memory allocations are checked for failures
   - A thorough cleanup function ensures no memory leaks

3. **Crash Detection**:
   - The target function has a potential division by zero if `res - 7 == 0` 
   - The function will also crash if the result of `(res - 7)` is such that `100/(res-7)` isn't exactly 3
   - I wrapped the call to `kTest_bug` in a try/catch block to prevent exceptions from stopping the fuzzer

4. **Structure Setup**:
   - Each object gets a unique name generated from its index
   - The fuzzer doesn't populate the `args` fields since they don't appear to be used in the target function

When LibFuzzer runs with this target, it will generate random inputs that get converted into `KTest` structures with varying object counts and byte arrays, allowing it to explore the behavior of the `kTest_bug` function and potentially find inputs that cause crashes.
```

The LLM was also prompted to generate a corpus generator `ktest-corpus-generator.c`.

## Modification to the Target
The fuzz target provided by LLM was then manually modified to 