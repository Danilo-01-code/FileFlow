# Queue
// TODO
the queue function works like the pipe on bash this is the flow that occurs:

file|directory -> command1 -> out_command1
out_command1 -> command2 -> out_command2
            and so on...

```
queue <file | directory> <command 1> <command 2> [...]
```

## Internal logic

On commands.c all last outputs are stored on the array 'last_outputs', the queue command uses it to operate many commands
```C
char *last_outputs[MAX_OUTPUTS];
```

```C
void _handle_last_single_output(const char *filename);
```

```C
void _handle_last_single_output(const char *filename);
```

```C
void handle_queue(char **args, int argc)
```