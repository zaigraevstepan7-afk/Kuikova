# egl_callback

- VA: `0x1d76f0`
- insns: 188

## Field offsets (ldr/str immediates)

```
{
  "16": 6,
  "32": 4,
  "8": 3,
  "1432": 3,
  "1436": 3,
  "48": 2,
  "64": 2,
  "80": 2,
  "96": 2,
  "112": 2,
  "40": 2,
  "3928": 1,
  "3744": 1,
  "24": 1,
  "1616": 1,
  "1620": 1,
  "1624": 1,
  "1296": 1,
  "1440": 1,
  "1441": 1,
  "1442": 1,
  "1444": 1,
  "1352": 1,
  "1400": 1
}
```

## Strings referenced

```
[]
```

## BL targets

```
[
  {
    "from": "0x1d7730",
    "to": "0x26c4f0"
  },
  {
    "from": "0x1d773c",
    "to": "0x1ef77c"
  },
  {
    "from": "0x1d7748",
    "to": "0x22e93c"
  },
  {
    "from": "0x1d774c",
    "to": "0x1f2744"
  },
  {
    "from": "0x1d7754",
    "to": "0x1daaec"
  },
  {
    "from": "0x1d7768",
    "to": "0x1f2744"
  },
  {
    "from": "0x1d7780",
    "to": "0x26c500"
  },
  {
    "from": "0x1d7794",
    "to": "0x26c500"
  },
  {
    "from": "0x1d77b8",
    "to": "0x26c510"
  },
  {
    "from": "0x1d781c",
    "to": "0x26c4b0"
  },
  {
    "from": "0x1d7834",
    "to": "0x1ebd9c"
  },
  {
    "from": "0x1d7848",
    "to": "0x1ebf60"
  },
  {
    "from": "0x1d7854",
    "to": "0x26c4e0"
  },
  {
    "from": "0x1d7858",
    "to": "0x22eaf8"
  },
  {
    "from": "0x1d785c",
    "to": "0x1f3570"
  },
  {
    "from": "0x1d7880",
    "to": "0x1da768"
  },
  {
    "from": "0x1d7890",
    "to": "0x1e955c"
  },
  {
    "from": "0x1d78d4",
    "to": "0x1ed030"
  },
  {
    "from": "0x1d78dc",
    "to": "0x1f288c"
  },
  {
    "from": "0x1d78f8",
    "to": "0x208f88"
  },
  {
    "from": "0x1d7908",
    "to": "0x1f288c"
  },
  {
    "from": "0x1d794c",
    "to": "0x208758"
  },
  {
    "from": "0x1d7974",
    "to": "0x208758"
  },
  {
    "from": "0x1d7978",
    "to": "0x1dac28"
  },
  {
    "from": "0x1d797c",
    "to": "0x1db864"
  },
  {
    "from": "0x1d7980",
    "to": "0x1fba6c"
  },
  {
    "from": "0x1d7994",
    "to": "0x26c520"
  },
  {
    "from": "0x1d7998",
    "to": "0x1f2764"
  },
  {
    "from": "0x1d799c",
    "to": "0x22ef18"
  }
]
```

## Rough pseudocode

```c
// recovered pseudocode: egl_callback
void egl_callback(...) {
    // branch tbnz w8, #0, #0x1d7760
    call(0x0x26c4f0);  // @0x1d7730
    // branch cbz x0, #0x1d7760
    call(0x0x1ef77c);  // @0x1d773c
    call(0x0x22e93c);  // @0x1d7748
    call(0x0x1f2744);  // @0x1d774c
    call(0x0x1daaec);  // @0x1d7754
    // branch tbz w8, #0, #0x1d79a0
    call(0x0x1f2744);  // @0x1d7768
    call(0x0x26c500);  // @0x1d7780
    call(0x0x26c500);  // @0x1d7794
    call(0x0x26c510);  // @0x1d77b8
    // branch cbz x11, #0x1d780c
    call(0x0x26c4b0);  // @0x1d781c
    call(0x0x1ebd9c);  // @0x1d7834
    call(0x0x1ebf60);  // @0x1d7848
    call(0x0x26c4e0);  // @0x1d7854
    call(0x0x22eaf8);  // @0x1d7858
    call(0x0x1f3570);  // @0x1d785c
    call(0x0x1da768);  // @0x1d7880
    // branch cbz w8, #0x1d7894
    call(0x0x1e955c);  // @0x1d7890
    // branch cbz w8, #0x1d78fc
    // branch cbz w8, #0x1d78fc
    call(0x0x1ed030);  // @0x1d78d4
    call(0x0x1f288c);  // @0x1d78dc
    call(0x0x208f88);  // @0x1d78f8
    // branch cbz w8, #0x1d7978
    call(0x0x1f288c);  // @0x1d7908
    call(0x0x208758);  // @0x1d794c
    call(0x0x208758);  // @0x1d7974
    call(0x0x1dac28);  // @0x1d7978
    call(0x0x1db864);  // @0x1d797c
    call(0x0x1fba6c);  // @0x1d7980
    call(0x0x26c520);  // @0x1d7994
    call(0x0x1f2764);  // @0x1d7998
    call(0x0x22ef18);  // @0x1d799c
    // branch b.ne #0x1d7a08
    return;
}
```
