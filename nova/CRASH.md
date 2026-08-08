# nova crash-safe inject notes

## If game still crashes

1. Test empty library first (proves injector/hide, not our logic):

```bash
./AndKittyInjector --package com.axlebolt.standoff2 --libs libnova_empty.so --memfd --hide --delay 3000000
```

- Empty crashes → problem is injector/`--hide`/device, not ESP code  
- Empty OK → use `libnova.so` below

2. Full build (no ctor, hook after 8s, ELF PLT only):

```bash
./AndKittyInjector --package com.axlebolt.standoff2 --libs libnova.so --memfd --hide --delay 3000000
```

Do **not** use `--free` with this build.

Wait ~10s in-match for `nova#` strip (8s delay + warmup frames).
