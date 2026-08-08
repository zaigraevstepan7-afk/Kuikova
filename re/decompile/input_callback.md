# input_callback

- VA: `0x1d760c`
- insns: 51

## Field offsets (ldr/str immediates)

```
{
  "16": 2,
  "32": 2,
  "1408": 1,
  "1616": 1,
  "1620": 1,
  "1624": 1,
  "48": 1
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
    "from": "0x1d7644",
    "to": "0x26c490"
  },
  {
    "from": "0x1d7654",
    "to": "0x26c4a0"
  },
  {
    "from": "0x1d7664",
    "to": "0x26c4b0"
  },
  {
    "from": "0x1d7670",
    "to": "0x26c4c0"
  },
  {
    "from": "0x1d7684",
    "to": "0x26c4d0"
  },
  {
    "from": "0x1d76c0",
    "to": "0x26c4e0"
  }
]
```

## Rough pseudocode

```c
// recovered pseudocode: input_callback
void input_callback(...) {
    // branch cbz x20, #0x1d76c4
    // branch cbnz x19, #0x1d76c4
    // branch cbz x20, #0x1d76c4
    call(0x0x26c490);  // @0x1d7644
    // branch b.ne #0x1d76c4
    call(0x0x26c4a0);  // @0x1d7654
    call(0x0x26c4b0);  // @0x1d7664
    call(0x0x26c4c0);  // @0x1d7670
    call(0x0x26c4d0);  // @0x1d7684
    // branch b.eq #0x1d76ac
    // branch b.ne #0x1d76b8
    // branch b #0x1d76b0
    call(0x0x26c4e0);  // @0x1d76c0
    return;
}
```
