# Task 2
This task is about implementing an LLVM pass to get statistics about numbers of instructions in each basic block.

**Disclaimer**: I used Claude 3.7 Sonnet to teach myself about LLVM passes. 
The prompt used was `Tell me about how to write an LLVM pass. I have no background knowledge nor the environment. Please start from a bare Ubuntu 22.04`.
The LLM agent generated an example pass, which can be found under `example_claude/`.
This example is incidentally very close to what is required in the task.
I believe it does not make too much sense to disregard the available example on hand and start from scratch, so I simply used it with minimal changes.
The same agent was also used to generate the gnuplot script `gp.gp`.


## Quick Start
Assuming all the dependencies are ready, the code can be run by
```
bash count_inst.sh <path_to_target>
```
Two test target programs `a.c` and `b.c` are included in the repo.

The output PNG file containing the histogram can be found at `inst_per_block.png`.

