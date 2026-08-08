# extra_hook_A

- VA: `0x1d82a0`
- insns: 39

## Field offsets (ldr/str immediates)

```
{
  "16": 2,
  "32": 2,
  "121": 2,
  "1272": 1,
  "8": 1,
  "168": 1,
  "1536": 1,
  "48": 1,
  "112": 1,
  "1544": 1,
  "1548": 1
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
    "from": "0x1d8328",
    "to": "0x1d833c"
  },
  {
    "from": "0x1d8334",
    "to": "0x1eb09c"
  }
]
```

## Rough pseudocode

```c
// recovered pseudocode: extra_hook_A
void extra_hook_A(...) {
    // branch cbz x0, #0x1d82e0
    // branch cbz x19, #0x1d82e0
    // branch cbz x8, #0x1d82e0
    // branch b.eq #0x1d8304
    // branch cbz x9, #0x1d82e0
    // branch b.eq #0x1d82e0
    // branch cbz w8, #0x1d82e0
    call(0x0x1d833c);  // @0x1d8328
    call(0x0x1eb09c);  // @0x1d8334
    // branch b #0x1d82e0
}
```
