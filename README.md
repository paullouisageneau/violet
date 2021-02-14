# Violet - Lightweight STUN/TURN server

Violet is a lightweight STUN/TURN server ([RFC8489](https://tools.ietf.org/html/rfc8489) and [RFC8656](https://tools.ietf.org/html/rfc8656)) based on [libjuice](https://github.com/paullouisageneau/libjuice).

![Oompa-Loompas rolling Violet, from Charlie and the Chocolate Factory](https://github.com/paullouisageneau/violet/blob/master/image.png?raw=true)

> "Mercy! Save us!" yelled Mrs Beauregarde. "[...] Violet, you’re **turn**ing violet, Violet!" [...]
>
> "Squeeze her," said Mr Wonka. "We've got to squeeze the **juice** out of her immediately."
>
> -- Charlie and the Chocolate Factory, Roald Dahl

## Building

### Clone repository and submodules

```bash
$ git clone https://github.com/paullouisageneau/violet.git
$ cd violet
$ git submodule update --init --recursive
```

### Building with CMake

```bash
$ cmake -B build
$ cd build
$ make -j2
```

### Running

```bash
$ ./violet -c USER:PASSWORD
```

