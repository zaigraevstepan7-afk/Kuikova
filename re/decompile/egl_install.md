# egl_install

- VA: `0x1d84cc`
- insns: 74

## Field offsets (ldr/str immediates)

```
{
  "1264": 3,
  "16": 2,
  "32": 2,
  "48": 2,
  "64": 1
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
    "from": "0x1d84f0",
    "to": "0x26c550"
  },
  {
    "from": "0x1d8508",
    "to": "0x26c560"
  },
  {
    "from": "0x1d851c",
    "to": "0x26c550"
  },
  {
    "from": "0x1d8534",
    "to": "0x26c560"
  },
  {
    "from": "0x1d8568",
    "to": "0x26c560"
  },
  {
    "from": "0x1d8578",
    "to": "0x1d917c"
  },
  {
    "from": "0x1d85ac",
    "to": "0x26c560"
  },
  {
    "from": "0x1d85bc",
    "to": "0x1d917c"
  }
]
```

## Rough pseudocode

```c
// recovered pseudocode: egl_install
void egl_install(...) {
    call(0x0x26c550);  // @0x1d84f0
    // branch cbz x0, #0x1d850c
    call(0x0x26c560);  // @0x1d8508
    call(0x0x26c550);  // @0x1d851c
    // branch cbz x0, #0x1d8538
    call(0x0x26c560);  // @0x1d8534
    // branch cbz x21, #0x1d85e0
    call(0x0x26c560);  // @0x1d8568
    call(0x0x1d917c);  // @0x1d8578
    // branch b.eq #0x1d85f4
    call(0x0x26c560);  // @0x1d85ac
    call(0x0x1d917c);  // @0x1d85bc
    // branch b.ne #0x1d85fc
    // branch b #0x1d863c
    return;
}
```
