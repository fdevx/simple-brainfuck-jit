# Brainfuck JIT
A very simple JIT for brainfuck using llvm, which I wrote for fun.

Can be run in single file execution or interactive mode.
In single file execution mode the specified file will be executed.
In interactive mode each line read from standard input will be interpreted individually. Each line is required 
to be a complete brainfuck expression. Loops cannot be split over multiple lines in interactive mode!

### Examples
Interprets the file "hello_world.b"
```bash
BrainfuckJIT ./hello_world.b
```

Runs the JIT in interactive mode
```bash
BrainfuckJIT
```

Prints the required execution time for various stages of the JIT process.
```bash
BrainfuckJIT -p
```

Prints information about the brainfuck state after execution.
Information includes current cell position and contents.
As well as number of allocated cells and the content of the first few cells.
```bash
BrainfuckJIT -s
```
