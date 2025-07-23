# Graphark

2d function grapher written in OpenGL.

![Graphark running](./docs/images/graphark_screenshot.png)

## Usage

Place the binary `Graphark` in the same directory as `shaders/` and execute it, you may use the `-h` flag to get help on
its syntax.

```
running_dir/
    Graphark.exe
    shaders/
        vertex.glsl
        fragment.glsl
```

```shell
Graphark [OPTIONS] functions


POSITIONALS:
  functions TEXT REQUIRED     Functions to graph separated by comma

OPTIONS:
  -h,     --help              Print this help message and exit
```

Some example valid values for `functions`:

- `x`
- `sin(x)`
- `x^x`
- `x,x+1`
- `x,x^x,sin(x)`

![Three functions displayed in graphark](./docs/images/graphark_three_functions_screenshot.png)

> [!NOTE]
> You can use the arrow keys, `-` and `=` to zoom and pan the graph.
