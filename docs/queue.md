# Queue

the queue function works like the pipe on bash ,this is the flow of execution with queue command:

file | directory -> command1 -> out_command1
out_command1 -> command2 -> out_command2
            and so on...

Usage:
```
queue <file | directory> <command 1> <command 2> [...]
```

In this example 'myfile' will be decompress (dcmp), moved (mv), and read; 
```
  queue myfile dcmp mv read
```

## Internal logic

### commands.c

All last outputs are stored on the char* array 'last_outputs', the queue command uses it to operate.
```C
char *last_outputs[MAX_OUTPUTS];
```

Outputs are committed to last outputs with this command:
```C
void _handle_last_single_output(const char *filename);
```

the queue manages the outputs, and use them as input to the other functions on the queue
```C
void handle_queue(char **args, int argc);
```
### tokenizer.c

all the commands on FileFlow are executed on 'executeCommand', the queue calls this function to execute many commands
```C
void executeCommand(char** tokens);
```