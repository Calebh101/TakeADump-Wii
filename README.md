# What is this?

This is a simple Homebrew app I made for my Wii, to try to dump some scratched Wii games. It'll retry sectors multiple times, and if the sector can't be read then (if configured to) then it'll just skip it and write 0s.

It uses a TUI instead of a GUI so I could debug easier and also not spend weeks writing this, as I don't have long with my Wii. After I'm done with this, if you'd like to contribute and make a fork, then go ahead; I can't properly test though, so I probably won't merge most PRs.

Also, the name is pretty good in my opinion, and I like the banner too.

# Requirements

- Wii
- Wii remote (no GameCube support)
- SD card or USB drive

# Building

I've included the libraries I used in `include` and `lib`, so just make sure DevkitPro with DevkitPPC is installed and set in your environmental variables, and run `make` in the root directory. You can also run `make` with `DEBUG=1` for a debug build.

`assemble.sh` just copies the previous build and the release assets to make `apps/TakeADump`.