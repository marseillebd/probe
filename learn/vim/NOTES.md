This is the guide I wished I had as I learned vim.
You can find a tutorial, and you can find reference material, and you can find suggestions.
I wanted them all in one place.
That way, I could learn a small section, move on when ready, and refer back to it when needed.

If you're new, follow these links:
- [First Time](#first-time)
- [First Movement](#first-movement)
- Now explore at your pleasure.
  I do have some suggestions:
  - Figure out buffers and panes. Most editors let you work with multiple files easily, and vim is no different.
  - I suggest learning operators, as they are the "killer feature". Without them, why even bother using vim as your daily driver?

Vim has a very high skill ceiling, and there's a lot left that I don't know.
Enough that my ignorance gets in my way sometimes, and you shouold know that.
This guide is constantly in-progress.
The "Stuff I Just Learned" sections show where I'm currently at.
Whie most sections are close by default, these are open because, let's face it, I might be the only reader of this guide.
I'm not sure under what exactly conditions a browser keeps sections open/closed, but hopefully your selections will persist.

Fundamentals
============

Start the editor, open file, do a little something, save, save as, quit: anything you'd expect in the "File" menu.
Also, vim is a modal editor, and this is where we discuss modes.

First Time
----------

<details><summary>Details</summary>

You begin in normal mode.
Type `i` to go to insert mode and add text.
From insert mode, type `<Esc>` to go back to normal mode.
From normal mode, use `:w` to save (write) your changes.
Then, you may use `:q` to quit.
Use `:wq` to write and quit all at once.
Use `:q!` to quit without saving changes.

TODO I know there's some configs to let you use the arrows and mouse if that's your jam

</details>

Stuff I Just Learned
--------------------

<details open><summary>Details</summary>

Reload the current file: `:e`.

</details>

Movement
========

First Movement
--------------

<details><summary>Details</summary>

Move the cursor one character/line at a time with `h, j, k, l`.
They're all on the same row, so how do I keep them straight?
Well `h` and `l` are easy: they're on the left and right, so they move left and right.
Then `j` is down because you move down more often than up, and `j` is under the index finger, which is more commonly used than your middle finger.
That just leaves `k` for up.
Really, it doesn't take long to get used to it.

Why these weird letters and not arrows? Historically, not many computers had arrow keys.
Why not arrows today? `hjkl` are right there at your fingertips, so you don't need to spend time moving your hand over to the side.
That might sound like a silly small amount of time save, but while I'm not a purist about it, I do feel like it really adds up, and it's a little more comfy on my appendages.
Fair warning: good touch typing skills will come in handy here, and they'll also speed you up in any other editor too.

Realistically, the big time saves with vim come from fancier motions than just one-at-a-time.

</details>

Operators
=========

Multiple Files
==============

TODO

`:vsplit`/`:vs` to split the screen into two vertical panes.
`<ctrl-w> {h,j,k,l}` to move between panes.
I've configured `zv` for vertical split, and `z{h,j,k,l}` for inter-pane movement.
`<ctrl-w> q` to close a pane (but the buffer stays open).

use `:ls` to see open buffers, then `:bN` for some N to switch to that buffer.
I really didn't like that so I installed a fzf plugin.
