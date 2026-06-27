So, appimage basically just jams a squashfs image into an executable, and the image contains your app and whatever dependencies you shove in.
It _does not_ help you track down your dependencies, so that could be a pain if you're expecting to distribute a portable app, but it's _technically_ out of scope.
At some point though, I have to ask what it can really do?

Put assets alongside your exe?
    But the squashfs is reaad-only, so no modding or self-updating.
    Also, you can compile-in assets to the `.data` section already, at the cost of a little compile script but no extra size cost.
Bundle your library dependencies?
    But you could just statically-link them, with again no extra size cost.
    The exception is audio/visual APIs.
It bundles a `.desktop` file and an icon?
    I don't think it translates these to non-freedesktop standards.
    I guess that skips an unzip+install step?
There's a bunch of metadata, like signing?
    Ok, but `.sig` files are already well-supported, and ordinary users want an app store which will handle signing anyway.

- [ ] [linuxdeploy](https://docs.appimage.org/packaging-guide/from-source/linuxdeploy-user-guide.html) looks like it might help find+bundle dependencies?
  I mean, I already have nix, but that's something, right? Yeah, if I'm loking at code that isn't already nix-packaged, I wonder if I could use it or its techniques to track down their dependencies.
