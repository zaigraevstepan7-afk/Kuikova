# DobbyHook

- VA: `0x2389d0`
- insns: 63

## Field offsets (ldr/str immediates)

```
{
  "16": 4,
  "32": 4,
  "48": 3,
  "1264": 1,
  "8": 1,
  "24": 1
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
    "from": "0x2389ec",
    "to": "0x23bc80"
  },
  {
    "from": "0x2389f8",
    "to": "0x23bc80"
  },
  {
    "from": "0x238a08",
    "to": "0x23bccc"
  },
  {
    "from": "0x238a10",
    "to": "0x23b6cc"
  },
  {
    "from": "0x238a18",
    "to": "0x23b700"
  },
  {
    "from": "0x238a40",
    "to": "0x26c330"
  },
  {
    "from": "0x238a50",
    "to": "0x23bc5c"
  },
  {
    "from": "0x238a58",
    "to": "0x26c330"
  },
  {
    "from": "0x238aa8",
    "to": "0x238db8"
  },
  {
    "from": "0x238aac",
    "to": "0x23b6cc"
  },
  {
    "from": "0x238ab4",
    "to": "0x23b730"
  }
]
```

## Rough pseudocode

```c
// recovered pseudocode: DobbyHook
void DobbyHook(...) {
    // branch cbz x0, #0x238a34
    call(0x0x23bc80);  // @0x2389ec
    call(0x0x23bc80);  // @0x2389f8
    call(0x0x23bccc);  // @0x238a08
    // branch tbz w0, #0, #0x238a20
    call(0x0x23b6cc);  // @0x238a10
    call(0x0x23b700);  // @0x238a18
    // branch cbz x0, #0x238a3c
    return;
    return;
    call(0x0x26c330);  // @0x238a40
    call(0x0x23bc5c);  // @0x238a50
    call(0x0x26c330);  // @0x238a58
    // branch cbz x21, #0x238aa4
    call(0x0x238db8);  // @0x238aa8
    call(0x0x23b6cc);  // @0x238aac
    call(0x0x23b730);  // @0x238ab4
    return;
}
```
