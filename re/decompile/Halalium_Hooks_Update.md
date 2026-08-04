# Halalium_Hooks_Update

- VA: `0x1d7a0c`
- insns: 258

## Field offsets (ldr/str immediates)

```
{
  "16": 6,
  "48": 6,
  "80": 5,
  "1272": 5,
  "8": 5,
  "32": 4,
  "64": 4,
  "96": 4,
  "352": 3,
  "1264": 3,
  "1464": 2,
  "121": 2,
  "1336": 2,
  "1472": 2,
  "136": 2,
  "160": 2,
  "360": 2,
  "420": 2,
  "1480": 2,
  "1448": 1,
  "1456": 1,
  "216": 1,
  "12": 1,
  "1640": 1,
  "1632": 1,
  "1441": 1,
  "1484": 1,
  "1485": 1,
  "1486": 1,
  "3916": 1,
  "1487": 1,
  "3920": 1,
  "1488": 1,
  "152": 1,
  "176": 1,
  "168": 1,
  "104": 1,
  "112": 1,
  "1490": 1,
  "232": 1,
  "40": 1,
  "3728": 1,
  "1492": 1
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
    "from": "0x1d7a4c",
    "to": "0x1d8968"
  },
  {
    "from": "0x1d7a54",
    "to": "0x1d8a80"
  },
  {
    "from": "0x1d7a6c",
    "to": "0x26c530"
  },
  {
    "from": "0x1d7ab0",
    "to": "0x268190"
  },
  {
    "from": "0x1d7b68",
    "to": "0x1d8928"
  },
  {
    "from": "0x1d7bb0",
    "to": "0x26c530"
  },
  {
    "from": "0x1d7c10",
    "to": "0x26c540"
  },
  {
    "from": "0x1d7c18",
    "to": "0x1d9b94"
  },
  {
    "from": "0x1d7c64",
    "to": "0x1d93dc"
  },
  {
    "from": "0x1d7c78",
    "to": "0x1d9b94"
  },
  {
    "from": "0x1d7ca4",
    "to": "0x1d9c30"
  },
  {
    "from": "0x1d7cb0",
    "to": "0x1d9c30"
  },
  {
    "from": "0x1d7cd0",
    "to": "0x1d9c54"
  },
  {
    "from": "0x1d7d04",
    "to": "0x1d9c00"
  },
  {
    "from": "0x1d7d14",
    "to": "0x1d9bd4"
  },
  {
    "from": "0x1d7d20",
    "to": "0x1d9c00"
  },
  {
    "from": "0x1d7dc0",
    "to": "0x1d9e00"
  }
]
```

## Rough pseudocode

```c
// recovered pseudocode: Halalium_Hooks_Update
void Halalium_Hooks_Update(...) {
    // branch cbz x8, #0x1d7df8
    call(0x0x1d8968);  // @0x1d7a4c
    call(0x0x1d8a80);  // @0x1d7a54
    // branch cbz w8, #0x1d7af0
    call(0x0x26c530);  // @0x1d7a6c
    call(0x0x268190);  // @0x1d7ab0
    // branch cbz x8, #0x1d7b60
    // branch b #0x1d7b64
    // branch cbz x8, #0x1d7df8
    // branch b.eq #0x1d7df8
    // branch b #0x1ea560
    call(0x0x1d8928);  // @0x1d7b68
    // branch b.le #0x1d7b8c
    // branch b #0x1d7ba4
    // branch b.lt #0x1d7bcc
    call(0x0x26c530);  // @0x1d7bb0
    call(0x0x26c540);  // @0x1d7c10
    call(0x0x1d9b94);  // @0x1d7c18
    // branch tbz w0, #0, #0x1d7c44
    // branch cbz x8, #0x1d7c4c
    // branch cbz x8, #0x1d7c4c
    // branch cbz x8, #0x1d7c4c
    // branch cbz w8, #0x1d7c6c
    // branch cbz x0, #0x1d7c6c
    call(0x0x1d93dc);  // @0x1d7c64
    // branch b #0x1d7c74
    call(0x0x1d9b94);  // @0x1d7c78
    // branch tbz w0, #0, #0x1d7d24
    // branch cbz x8, #0x1d7d24
    // branch cbz x21, #0x1d7d24
    // branch cbz w8, #0x1d7cb4
    call(0x0x1d9c30);  // @0x1d7ca4
    call(0x0x1d9c30);  // @0x1d7cb0
    // branch cbz w8, #0x1d7cd4
    call(0x0x1d9c54);  // @0x1d7cd0
    // branch cbz x20, #0x1d7d24
    // branch cbz w8, #0x1d7d24
    call(0x0x1d9c00);  // @0x1d7d04
    call(0x0x1d9bd4);  // @0x1d7d14
    call(0x0x1d9c00);  // @0x1d7d20
    // branch cbz w8, #0x1d7d74
    // branch cbz x9, #0x1d7d74
    // branch cbz x8, #0x1d7d74
    // branch cbz x10, #0x1d7d74
    // branch cbz x9, #0x1d7d74
    // branch cbz w8, #0x1d7dbc
    // branch cbz x8, #0x1d7dbc
    // branch cbz x8, #0x1d7dbc
    // branch cbz x0, #0x1d7dbc
    call(0x0x1d9e00);  // @0x1d7dc0
    // branch cbz w8, #0x1d7df8
    // branch b #0x1ea940
    return;
}
```
