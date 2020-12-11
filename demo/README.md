## Demo

This is a small Linux application using [eli-cli](../README.md) and an example
that demonstrates the functionality of the user interface.

```
(linux shell)$ make
(linux shell)$ ./demo

(ile-cli) > echo hello world

hello world

(ile-cli) > ?
line                         Console configuration
debug                        Debug configuration
echo                         Echo
exit                         Enter to exit the demo program

        *  *  *  *  *  *  *  *  *  *  *  *  *  *
```

Banner configuration:
```
(ile-cli) > line banner
colour                       Banner colour
name                         Banner name
(ile-cli) > line banner name ?
<name>                       Enter banner name
(ile-cli) > line banner name (demo)

(demo) > line banner colour
<colour>                     Enter <red | green | yellow | blue | magenta | cyan | white>
reset                        Reset banner colour
(demo) > line banner colour cyan

(demo) >
```

Do not forget that you can move in the command history entries with
the "up" and "down" keys, or see all entries:

```
(demo) > line history
0 : line history
1 : line banner colour cyan
2 : line banner name (demo)

(demo) >
```

Execute the command from the Linux shell:
```
(linux shell)$ ./demo -C "echo hello world"

hello world
```

Use this example to create a lightweight and complex interface for
managing your hardware. Good luck!
