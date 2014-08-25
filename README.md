# Status
Functional, but documentation is nonexistent at this point.



# Overview
GCiaB provides a simple GC algorithm designed to make life easier during the implementation of new programming languages. It isn't highly performant, employing mark & sweep collection atop malloc calls, but the API is super simple. If you're prototyping a language, or building a toy language, where GC hangs aren't detrimental, something like this may fit your needs. 

However, if X-language has high ambitions, this library should be replaced later on, once core features are functional and the prototyping stage is complete.



# Sample Usage
Coming soon...



# Installation
This project is managed by [Kit](https://github.com/dasmithii/Kit). If you haven't already, install kit via [this link](https://github.com/dasmithii/Kit#installation), then execute the following:
```
kit fetch GCiaB
```
Once complete, `#include <kit/GCiaB.h>` will import all API definitions when built with Kit. For those uninterested in a new project manager, just copy GCiaB sources directly.